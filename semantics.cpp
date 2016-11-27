#include"stdafx.h"

vector<tblitem> glbtbl;//global symbol table
vector<string> strtbl;//string table
vector<funcitem> functbl;//function table

funcitem *context = nullptr;//the function context

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
    assert(context!=nullptr);
    vector<tblitem> &lcltbl = (context->lcltbl);
    for(uint i=0; i < lcltbl.size(); ++i)
        if(lcltbl[i].name == nm)//found
            return i;
    return -1;
}

int findidt(const string &nm)
{
    assert(context!=nullptr);
    //TODO
}

bool insertobj(int objtyp, const string& nm, int typ, bool isarray, int val)
{
    if(context == nullptr)//global object
    {
        if(findglb(nm)>=0)//found object in table
        {
            ERROR("Multi definition: " + nm);
            return false;
        }
        glbtbl.push_back(tblitem{objtyp, nm, typ, isarray, val});
        return true;
    }
    else    //local object
    {
        if(findlcl(nm)>=0)
        {
            ERROR("Multi definition: " + nm);
            return false;
        }
        (context->lcltbl).push_back(tblitem{objtyp, nm, typ, isarray, val});
        return true;
    }
}

void tbl_dump()
{
    ostream &out = *tbl_output;

    out << "Global table:" << endl;
    for(auto it: glbtbl)
        cout << setw(2) << it.objtype << "  "
            << setw(15) << it.name << "  "
            << setw(3) << it.datatype << "  "
            << setw(1) << it.isarray << "  "
            << setw(4) << it.value << "  "
            << it.addr << endl;
}
