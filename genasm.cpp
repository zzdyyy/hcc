#include "stdafx.h"

static ostringstream out;
static funcitem *fitem = nullptr;

//some different name for different type
string ptr_arr[] = {"DWORD PTR", "BYTE PTR"};
string mov_arr[] = {"mov  ", "movsx"};
string eax_arr[] = {"eax", "al"};
inline int t2i(int itype_t) { return itype_t-1; }//type_t to index
inline string &ptr(qoperand q) { return ptr_arr[t2i(getitem(q).datatype)]; }
inline string &mov(qoperand q) { return mov_arr[t2i(getitem(q).datatype)]; }
inline string &eax(qoperand q) { return eax_arr[t2i(getitem(q).datatype)]; }

void genheader()
{
    //TODO: out << ".386";
}

void gendataseg()
{
    string type_name[] = {"", "dd", "db"}; //define double-word/byte
    out << ".data" << endl;

    //all global const and vars
    for(tblitem &item: glbtbl)
    {
        if(item.objtype == tblitem::FUNCTION)
            continue;
        bool isconst = (item.objtype == tblitem::CONST);
        out << "    " << "align " << size_of[item.datatype] << endl;
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
        out << "    s" << i << " db \"" << strtbl[i] << "\",0" << endl;
    }
    //format control string
    out << "    fmtd db \"%d\",0" << endl;
    out << "    fmtc db \"%c\",0" << endl;
    out << "    fmts db \"%s\",0" << endl;
    out << "    fmtnl db 10,0" << endl;//newline
}

//return the addressing string of immediate number / temp number / simple var/const
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
    assert(!ti.isarray);

    if(qo.type == qoperand::GLB_OBJ)//global object
    {
        return ptr(qo)+"_"+ti.name;
    }

    //local object
    if(qo.value < fitem->argnum)//parameter
        return "DWORD PTR [ebp+" +
                tostr(8 + qo.value*4)//the first arg is [ebp+8]
                + "]";
    else
        return ptr(qo)
                +string(" [ebp-")
                +tostr(4+ti.addr)//the first local var is [ebp-4]
                +"]";
}

void gencodeseg()
{
    out << ".code" << endl;
    out << "init:" << endl;
    out << "..." << endl << endl;//TODO

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
        for(qi &q: fi.qilist)
        {
            switch(q.op)
            {
            case qi::LABEL:
                out << "L" << q.A.value << ":" <<endl;
                break;

            case qi::RET:
                if(q.A.type != qoperand::BLANK)
                    out << "    " << mov(q.A) << " " << eax(q.A) << ", " << addrstr(q.A) << endl;
                out << "    jmp   " << lblexit << endl;
                break;

            case qi::WR:
                if(qi.A.type == qoperand::STRING)
                    out<<"    invoke crt_printf,addr fmts,addr s"<<qi.A.value<<endl;

            case
            }
        }
        out << lblexit << ":" << endl
            << "    mov  esp, ebp" << endl
            << "    pop  ebp" << endl
            << "    ret" << endl;
        ++context;
    }
}

void genasm()
{
    genheader();
    gendataseg();
    gencodeseg();
    *asm_output << out.str();
}
