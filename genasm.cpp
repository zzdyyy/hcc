#include "stdafx.h"

#define PRINTF_NEWLINE
#undef PRINTF_NEWLINE


static ostringstream out;
static funcitem *fitem = nullptr;

void genheader()
{
    out << ".386" << endl
        << ".model flat, stdcall" << endl
        << "option casemap:none" << endl
        << "include c:/masm32/include/msvcrt.inc" << endl
        << "includelib c:/masm32/lib/msvcrt.lib" << endl
        << "" << endl
        ;
}

void gendataseg()
{
    string type_name[] = {"", "dd", "dd"}; //define double-word/byte
    out << ".data" << endl;

    //all global const and vars
    for(tblitem &item: glbtbl)
    {
        if(item.objtype == tblitem::FUNCTION)
            continue;
        bool isconst = (item.objtype == tblitem::CONST);
        //out << "    " << "align " << size_of[item.datatype] << endl;
        out << "    _" << item.name << " " << type_name[item.datatype] << " ";
        if(item.isarray)
            out << item.value << " dup(0)" <<endl;
        else
        {
            if(isconst)
                out << item.value << endl;
            else
                out << "0" << endl;
        }
    }

    //all string value
    for(unsigned int i=0; i<strtbl.size(); ++i)
    {
        if(strtbl[i] == "")
            out << "    s" << i << " db 0" << endl;
        else
            out << "    s" << i << " db \"" << strtbl[i] << "\",0" << endl;
    }
    //format control string
    out << "    fmtd db \"%d\",0" << endl;
    out << "    fmtc db \"%c\",0" << endl;
    out << "    fmts db \"%s\",0" << endl;
    out << "    fmtnl db 10,0" << endl;//newline
    out << endl;
}

//return the addressing string of immediate number / temp number / simple var/const
//or base address of array
//immediate number      10 -> 10
//temp number           $5 -> DWORD PTR [ebp-20]
//simple var/const      glb -> BYTE PTR _glb
//                      var -> DWORD PTR [ebp-8]
//                      para -> DWORD PTR [ebp+12]
string addrstr(qoperand qo)
{
    if(qo.type == qoperand::IMDCHAR || qo.type == qoperand::IMDINT)
        return tostr(qo.value);

    if(qo.type == qoperand::TMP)//temp local var
        return "DWORD PTR [ebp-" +
                tostr(fitem->varsize + 4 + qo.value*4)
                + "]";//the first local var is [ebp-4]

    tblitem &ti = getitem(qo);

    if(qo.type == qoperand::GLB_OBJ)//global object
    {
        return "DWORD PTR _"+ti.name;
    }

    //local object
    if(qo.value < fitem->argnum)//parameter
        return "DWORD PTR [ebp+" +
                tostr(8 + qo.value*4)//the first arg is [ebp+8]
                + "]";
    else
        return "DWORD PTR [ebp-"
                +tostr(4+ti.addr)//the first local var is [ebp-4] TODO: this is not compatible with size[char_t]=1(compact store)
                +"]";
}

string cmpopstr(int op)
{
    switch(op)
    {
        case qi::JEQ: return("jz ");
        case qi::JNE: return("jne");
        case qi::JGE: return("jge");
        case qi::JGT: return("jg ");
        case qi::JLE: return("jle");
        case qi::JLT: return("jl ");
    }
    return ("ERR");
}

void gencodeseg()
{
    out << ".code" << endl;
    out << "init:" << endl;
    out << "    call  _main" << endl;
    out << "    invoke crt_exit" << endl;
    out << endl ;

    context = 0;
    for(funcitem &fi: functbl)
    {
        fitem = &fi;
        tblitem &ti = glbtbl[fi.index];
        string lblexit = "exit_"+ti.name;
        out << "_" << ti.name << ":" << endl
            << "    push  ebp" << endl
            << "    mov   ebp, esp" << endl
            << "    sub   esp, " << fi.varsize+fi.tmpamt*4 << endl;
        //set value for constant first
        int i = 0;
        for(tblitem &cti: functbl[context].lcltbl)
        {
            if(cti.objtype == tblitem::CONST)
                out << "    mov   "<<addrstr(qoperand{qoperand::LCL_OBJ, i})
                    << ", " << cti.value <<endl;
            ++i;
        }
        //generate asm for statements
        for(qi &q: fi.qilist)
        {
            switch(q.op)
            {
            case qi::LABEL:
                out << "L" << q.A.value << ":" <<endl;
                break;

            case qi::RET:
                if(q.A.type != qoperand::BLANK)
                    out << "    mov   eax, " << addrstr(q.A) << endl;
                out << "    jmp   " << lblexit << endl;
                break;

            case qi::WR:
                if(q.A.type == qoperand::STRING)
                    out<<"    invoke crt_printf,addr fmts,addr s"<<q.A.value<<endl;
                else
                {
                    out << "    mov   eax, " << addrstr(q.A) << endl;
                    if(q.A.type == qoperand::IMDCHAR ||
                       ((q.A.type == qoperand::LCL_OBJ || q.A.type == qoperand::GLB_OBJ) && getitem(q.A).datatype == char_t ))
                        out<<"    invoke crt_printf,addr fmtc,eax"<<endl;
                    else
                        out<<"    invoke crt_printf,addr fmtd,eax"<<endl;
                }
                #ifdef PRINTF_NEWLINE
                out<<"    invoke crt_printf,addr fmts,addr fmtnl"<<endl;
                #endif // PRINTF_NEWLINE
                break;

            case qi::RD:
                out << "    lea   eax, " << addrstr(q.A) << endl;
                if( getitem(q.A).datatype == char_t )
                    out<<"    invoke crt_scanf,addr fmtc,eax"<<endl;
                else
                    out<<"    invoke crt_scanf,addr fmtd,eax"<<endl;
                break;

            case qi::PUSH:
                out<<"    mov   eax, "<<addrstr(q.A)<<endl;
                out<<"    push  eax"<<endl;
                break;

            case qi::CALL:
                out<<"    call  _"<<glbtbl[q.A.value].name<<endl;
                out<<"    add   esp, "<<functbl[glbtbl[q.A.value].value].argnum*4<<endl;
                out<<"    mov   "<<addrstr(q.D)<<", eax"<<endl;
                break;

            case qi::ARRAYLOAD:
                out<<"    lea   edx, "<<addrstr(q.A)<<endl;//base addr
                out<<"    mov   eax, "<<addrstr(q.B)<<endl;//index
                out<<"    mov   eax, DWORD PTR [edx+eax*"<<
                    size_of[getitem(q.A).datatype]<<"]"<<endl;//load
                out<<"    mov   "<<addrstr(q.D)<<", eax"<<endl;//store
                break;

            case qi::ARRAYASS:
                out<<"    lea   edx, "<<addrstr(q.A)<<endl;//base addr
                out<<"    mov   eax, "<<addrstr(q.B)<<endl;//index
                out<<"    mov   ecx, "<<addrstr(q.D)<<endl;//value
                out<<"    mov   DWORD PTR [edx+eax*"<<
                    size_of[getitem(q.A).datatype]<<"], ecx"<<endl;//assign
                break;

            case qi::ASSIGN:
                out<<"    mov   eax, "<<addrstr(q.A)<<endl;//value
                out<<"    mov   "<<addrstr(q.D)<<", eax"<<endl;//assign
                break;

            case qi::JMP:
                out<<"    jmp   L"<<q.D.value<<endl;
                break;

            case qi::JZ:
                out<<"    mov   eax, "<<addrstr(q.A)<<endl;
                out<<"    test  eax, eax"<<endl;
                out<<"    jz    L"<<q.D.value<<endl;
                break;

            case qi::JNE:
            case qi::JEQ:
            case qi::JGT:
            case qi::JGE:
            case qi::JLT:
            case qi::JLE:
                out<<"    mov   eax, "<<addrstr(q.A)<<endl;
                out<<"    mov   ecx, "<<addrstr(q.B)<<endl;
                out<<"    cmp   eax, ecx"<<endl;
                out<<"    "<<cmpopstr(q.op)<<"   L"<<q.D.value<<endl;
                break;

            case qi::MUL:
                out<<"    mov   eax, "<<addrstr(q.A)<<endl;
                out<<"    imul  eax, "<<addrstr(q.B)<<endl;
                out<<"    mov   "<<addrstr(q.D)<<", eax"<<endl;
                break;

            case qi::DIV:
                out<<"    mov   eax, "<<addrstr(q.A)<<endl;
                out<<"    cdq"<<endl;
                out<<"    mov   ecx, "<<addrstr(q.B)<<endl;
                out<<"    idiv  ecx"<<endl;
                out<<"    mov   "<<addrstr(q.D)<<", eax"<<endl;
                break;

            case qi::ADD:
                out<<"    mov   eax, "<<addrstr(q.A)<<endl;
                out<<"    add   eax, "<<addrstr(q.B)<<endl;
                out<<"    mov   "<<addrstr(q.D)<<", eax"<<endl;
                break;

            case qi::SUB:
                out<<"    mov   eax, "<<addrstr(q.A)<<endl;
                out<<"    sub   eax, "<<addrstr(q.B)<<endl;
                out<<"    mov   "<<addrstr(q.D)<<", eax"<<endl;
                break;

            case qi::NEG:
                out<<"    mov   eax, "<<addrstr(q.A)<<endl;
                out<<"    neg   eax"<<endl;
                out<<"    mov   "<<addrstr(q.D)<<", eax"<<endl;
                break;

            default:
                assert(0);
            }
        }
        out << lblexit << ":" << endl
            << "    mov  esp, ebp" << endl
            << "    pop  ebp" << endl
            << "    ret" << endl
            << endl;
        ++context;
    }
}

void genasm()
{
    genheader();
    gendataseg();
    gencodeseg();

    out << endl;
    out << "end init";

    *asm_output << out.str() << endl;
}
