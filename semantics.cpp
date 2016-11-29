#include"stdafx.h"

vector<tblitem> glbtbl;//global symbol table
vector<string> strtbl;//string table
vector<funcitem> functbl;//function table

int context = -1;//the function context (index of functbl), <0 means not in a function context
int size_of[] = { 0, 4, 1, 0};//the size of void_t, int_t, char_t and string_t
qoperand foundopr = qoperand{ qoperand::GLB_OBJ, -1 };//found result by findidt()

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



int local_addr = 0;//address offset of object in stack frame
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
}

void exitcontext()
{
    assert(context >= 0);
    int t = functbl[context].varsize;
    functbl[context].varsize = (t+3)/4*4;//align
    context = -1;
}



void tbl_dump()
{
    ostream &out = *tbl_output;

    out << "Global table:" << endl;
    for(auto it: glbtbl)
        out << setw(2) << it.objtype << "  "
            << setw(15) << it.name << "  "
            << setw(3) << it.datatype << "  "
            << setw(1) << it.isarray << "  "
            << setw(4) << it.value << "  "
            << it.addr << endl;
    out << endl;

    for(auto fi: functbl)
    {
        out << "In function " << glbtbl[fi.index].name << "():" << endl;
        out << "index: " << fi.index << " argnum: " << fi.argnum
            << " varsize: " << fi.varsize << " tmpamt: " << fi.tmpamt << endl;
        out << "Local table:" << endl;
        for(auto it: fi.lcltbl)
            out << setw(2) << it.objtype << "  "
                << setw(15) << it.name << "  "
                << setw(3) << it.datatype << "  "
                << setw(1) << it.isarray << "  "
                << setw(4) << it.value << "  "
                << it.addr << endl;
        out << endl;
    }
}
