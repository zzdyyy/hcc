#include"stdafx.h"

vector<tblitem> glbtbl;//global symbol table
vector<string> strtbl;//string table
vector<funcitem> functbl;//function table

qoperand foundopr = qoperand{ qoperand::GLB_OBJ, -1 };//found result by findidt()
int context = -1;//the function context (index of functbl), <0 means not in a function context
int size_of[] = { 0, 4, 1, 0};//the size of void_t, int_t, char_t and string_t

//variables used only in this source file
int local_addr = 0;//address offset of object in stack frame
int llblc = 0;//local label counter
int ltmpc = 0;//local temp variable counter
const qoperand BLANKOP = qoperand{qoperand::BLANK, 0};

//found: findglb==index>=0 ; not found: findglb<0
int findglb(const string &nm)
{
    for(uint i=0; i<glbtbl.size(); ++i)
        if(glbtbl[i].name == nm)//found
            return i;
    return -1;
}

int findlcl(const string &nm)
{
    assert(context>=0);
    for(uint i=0; i < functbl[context].lcltbl.size(); ++i)
        if(functbl[context].lcltbl[i].name == nm)//found
            return i;
    return -1;
}

bool findidt(const string &nm)
{
    assert(context>=0);
    int idx = 0;

    idx = findlcl(nm);
    if(idx >= 0)//found in local
    {
        foundopr = qoperand{qoperand::LCL_OBJ, idx};
        return true;
    }
    idx = findglb(nm);
    if(idx >= 0)//found in global
    {
        foundopr = qoperand{qoperand::GLB_OBJ, idx};
        return true;
    }
    if(nm!="scanf" && nm!="printf")
        ERROR("Identifier not found: "+nm);
    return false;
}

bool isfunction(const string &idt)
{
    if(findidt(idt))
    {
        if(foundopr.type == foundopr.GLB_OBJ)
            return (glbtbl[foundopr.value].objtype == tblitem::FUNCTION);
        else
            return false;// function defined in function is not allowed
    }
    return false;
}



bool insertobj(int objtyp, const string& nm, int typ, bool isarray, int val)
{
    if(context < 0)//global object
    {
        if(findglb(nm)>=0)//found object in table
        {
            tbl_dump();
            ERROR("Multi definition: " + nm);
            return false;
        }
        glbtbl.push_back(tblitem{objtyp, nm, typ, isarray, val, tblitem::NOUSE});
        return true;
    }
    else    //local object
    {
        if(findlcl(nm)>=0)
        {
            tbl_dump();
            ERROR("Multi definition: " + nm);
            return false;
        }
        //allocate stack memory and insert
        int sz = size_of[typ];
        local_addr = (local_addr + sz - 1) / sz * sz;//set align
        functbl[context].lcltbl.push_back(tblitem{objtyp, nm, typ, isarray, val, local_addr});
        if(isarray)
            local_addr += sz*val;
        else
            local_addr += sz;
        functbl[context].varsize = local_addr;
        return true;
    }
}

bool insertpara(int typ, const string &nm)
{
    assert(context >= 0);//in a function context
    if(findlcl(nm)>=0)
    {
        tbl_dump();
        ERROR("Multi definition: " + nm);
        return false;
    }
    functbl[context].lcltbl.push_back(tblitem{tblitem::VAR, nm, typ, false, tblitem::NOUSE, tblitem::NOUSE});
    functbl[context].argnum ++;
    return true;
}

//insert the function into table, and build a funcitem as context
void buildcontext(int rettyp, const string& nm)
{
    assert(context < 0);
    int idx = functbl.size();
    if(!insertobj(tblitem::FUNCTION, nm, rettyp, false, idx))
        ;//TODO: throw a exception out to function declaration

    functbl.push_back(funcitem{int(glbtbl.size())-1, 0, 0, 0,});
    context = idx;
    local_addr = 0;
    ltmpc = 0;
}

void exitcontext()
{
    assert(context >= 0);
    int t = functbl[context].varsize;
    functbl[context].varsize = (t+3)/4*4;//align
    functbl[context].tmpamt = ltmpc;
    context = -1;
}

void emitqi(const qi &q)
{
    assert(context >= 0);
    functbl[context].qilist.push_back(q);
}

qoperand newtmp()
{
    return qoperand{qoperand::TMP, ltmpc++};
}

qoperand newlabel()
{
    return qoperand{qoperand::LABEL, llblc++};
}

qoperand arrayload(qoperand arropr, qoperand index)
{
    tblitem &item = getitem(arropr);
    if(! item.isarray)
        ERROR("Not an array: "+item.name);
    qoperand element = newtmp();
    emitqi(qi{qi::ARRAYLOAD, arropr, index, element});
    return element;
}

void arrayass(qoperand arr, qoperand index, qoperand val)
{
    tblitem &item = getitem(arr);
    if(! item.isarray)
        ERROR("Not an array: "+item.name);
    emitqi(qi{qi::ARRAYASS, arr, index, val});
}

void assign(qoperand var, qoperand val)
{
    tblitem &item = getitem(var);
    if(item.isarray)
        ERROR("Array cannot be assigned without []: "+item.name);
    if(item.objtype==tblitem::CONST)
        ERROR("Cannot assign to a constant.");
    emitqi(qi{qi::ASSIGN, val, BLANKOP, var});
}

qoperand muldiv(int op, qoperand f1, qoperand f2)
{
    /*if( (f1.type==qoperand::GLB_OBJ || f1.type==qoperand::GLB_OBJ) && getitem(f1).isarray
       || (f2.type==qoperand::GLB_OBJ || f2.type==qoperand::GLB_OBJ) && getitem(f2).isarray)
        ERROR("Factor cannot be an array.");*///already checked in factor()
    qoperand f = newtmp();
    if(op == multop)
        emitqi(qi{qi::MUL, f1, f2, f});
    else
        emitqi(qi{qi::DIV, f1, f2, f});
    return f;
}

qoperand addsub(int op, qoperand t1, qoperand t2)
{
    qoperand t = newtmp();
    if(op == addop)
        emitqi(qi{qi::ADD, t1, t2, t});
    else
        emitqi(qi{qi::SUB, t1, t2, t});
    return t;
}

qoperand neg(qoperand t)
{
    qoperand t1 = newtmp();
    emitqi(qi{qi::NEG, t, BLANKOP, t1});
    return t1;
}

void push(qoperand arg)
{
    emitqi(qi{qi::PUSH, arg, BLANKOP, BLANKOP});
}

qoperand call(qoperand func)
{
    qoperand ret = newtmp();
    emitqi(qi{qi::CALL, func, BLANKOP, ret});
    return ret;
}

void ret(bool withvalue, qoperand value)
{
    if(withvalue)
        emitqi(qi{qi::RET, value, BLANKOP, BLANKOP});
    else
        emitqi(qi{qi::RET, BLANKOP, BLANKOP, BLANKOP});
}

void jz(qoperand cond, qoperand lbl)
{
    emitqi(qi{qi::JZ, cond, BLANKOP, lbl});
}

void jifnot(int op, qoperand e1, qoperand e2, qoperand lbl)
{
    switch(op)
    {
    case leop:
        emitqi(qi{qi::JGT, e1, e2, lbl}); break;
    case ltop:
        emitqi(qi{qi::JGE, e1, e2, lbl}); break;
    case geop:
        emitqi(qi{qi::JLT, e1, e2, lbl}); break;
    case gtop:
        emitqi(qi{qi::JLE, e1, e2, lbl}); break;
    case eqlop:
        emitqi(qi{qi::JNE, e1, e2, lbl}); break;
    case neqlop:
        emitqi(qi{qi::JEQ, e1, e2, lbl}); break;
    default:
        assert(0);
    }
}

void setlabel(qoperand lbl)
{
    emitqi(qi{qi::LABEL, lbl, BLANKOP, BLANKOP});
}

void jmp(qoperand lbl)
{
    emitqi(qi{qi::JMP, BLANKOP, BLANKOP, lbl});
}

void rd(qoperand opr)
{
    tblitem &item = getitem(opr);
    if(item.isarray)
        ERROR("Cannot read into an array: "+item.name);
    if(item.objtype == tblitem::CONST)
        ERROR("Cannot read into a constant: "+item.name);
    emitqi(qi{qi::RD, opr, BLANKOP, BLANKOP});
}

qoperand insertstr(const string &str)
{
    int idx = strtbl.size();
    strtbl.push_back(str);
    return qoperand{qoperand::STRING, idx};
}

void wr(qoperand opr)
{
    emitqi(qi{qi::WR, opr, BLANKOP, BLANKOP});
}






string qotostr(qoperand qo)
{
    //cout<<"debug:"<<qo.type<<","<<qo.value<<endl;
    switch(qo.type)
    {
    case qoperand::IMDCHAR:
        return "\'"+string(1, char(qo.value))+"\'";
    case qoperand::IMDINT:
        return tostr(qo.value);
    case qoperand::GLB_OBJ:
    case qoperand::LCL_OBJ:
        return getitem(qo).name;
    case qoperand::TMP:
        return "$"+tostr(qo.value);
    case qoperand::BLANK:
        return "";
    case qoperand::STRING:
        return "\""+strtbl[qo.value]+"\"";
    case qoperand::LABEL:
        return ":L"+tostr(qo.value);
    }
    return "ERROR";
}

void tbl_dump()
{
    string mnemo[] = {
        "ADD", "SUB", "NEG", "MUL", "DIV", "JLT", "JGT", "JLE", "JGE", "JEQ", "JNE", "JZ", "JMP",
        "ASSIGN", "ARRAYLOAD", "ARRAYASS", "PUSH", "CALL", "RD", "WR", "RET", "LABEL",
    };
    ostream &out = *tbl_output;

    out << "Global table:" << endl;
    for(auto it: glbtbl)
        out << "    "
            << setw(2) << it.objtype << "  "
            << setw(15) << it.name << "  "
            << setw(3) << it.datatype << "  "
            << setw(1) << it.isarray << "  "
            << setw(4) << it.value << "  "
            << it.addr << endl;
    out << endl;

    out << "String table:" << endl;
    for(auto it: strtbl)
        out << "    " << "\"" << it << "\"" << endl;
    out << endl;

    context = 0;
    for(auto fi: functbl)
    {
        out << "In function " << glbtbl[fi.index].name << "():" << endl;
        out << "index: " << fi.index << " argnum: " << fi.argnum
            << " varsize: " << fi.varsize << " tmpamt: " << fi.tmpamt << endl;
        out << "Local table:" << endl;
        for(auto it: fi.lcltbl)
            out << "    "
                << setw(2) << it.objtype << "  "
                << setw(15) << it.name << "  "
                << setw(3) << it.datatype << "  "
                << setw(1) << it.isarray << "  "
                << setw(4) << it.value << "  "
                << it.addr << endl;
        out << "Quaternary:" << endl;
        for(qi q : fi.qilist)
        {
            /*out<<"debug:"<<q.op<<","<<q.A.type<<","<<q.A.value<<","
                <<q.B.type<<","<<q.B.value<<","
                <<q.D.type<<","<<q.D.value<<endl;*/
            if(q.op == qi::LABEL)
                out << "    L" << q.A.value << ":" << endl;
            else
                out << "    "
                    << setw(9) << mnemo[q.op] << ", "
                    << setw(6) << qotostr(q.A) << ", "
                    << setw(6) << qotostr(q.B) << ", "
                    << setw(6) << qotostr(q.D) << endl;
        }
        out << endl;
        ++context;
    }
}
