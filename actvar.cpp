#include "stdafx.h"
#define ACTVAR_DEBUG

struct bblk
{
    int id=-1;//block number
    int entrance=-1;//entrance address
    int exit=-1;//exit address
    vector<int> from;
    vector<int> to;

    bblk(int theid, int theentrance): id(theid), entrance(theentrance) {}
};
vector<vector<bblk> > bblktbl;

bool isjmp(qi &q)
{
    switch(q.op)
    {
    case qi::JEQ: case qi::JGE: case qi::JGT: case qi::JLE:
    case qi::JLT: case qi::JMP: case qi::JZ: case qi::JNE:
    case qi::RET:
        return true;
    default:
        return false;
    }
}

bool isjc(qi &q)
{
    switch(q.op)
    {
    case qi::JEQ: case qi::JGE: case qi::JGT: case qi::JLE:
    case qi::JLT: case qi::JZ: case qi::JNE:
        return true;
    default:
        return false;
    }
}

void buildbblk()
{
    //clear bblktbl
    vector<vector<bblk> > newbblktbl(functbl.size(), vector<bblk>());
    bblktbl.swap(newbblktbl);
    newbblktbl.clear();

    for(size_t ct=0; ct<functbl.size(); ++ct)
    {//does here need LOADCONTEXT?
        vector<qi> &qilist = functbl[ct].qilist;
        size_t len = qilist.size();
        vector<int> entrcnt(len, -1);
        if(len == 0)
            continue;
        vector<bblk> &bblks = bblktbl[ct];

        int cnt = 0;
        entrcnt[0] = cnt++; bblks.push_back(bblk(0, 0));
        //find all entrances
        for(size_t n=0; n<len; ++n)
        {
            //statement with labels or statement after jump instructions
            if(qilist[n].op == qi::LABEL || isjmp(qilist[n]))
            {
                do{++n;}while(n<len && qilist[n].op == qi::LABEL);
                if(n<len)
                {
                    entrcnt[n--]=cnt++;
                    bblks.push_back(bblk(cnt-1, n+1));
                }
            }
        }

        //build bblks
        for(size_t n=0; n<len; ++n)
        {

        }

#ifdef ACTVAR_DEBUG
        cerr<<glbtbl[functbl[ct].index].name<<"():"<<endl;
        for(bblk b : bblks)
        {
            cerr<<setw(4)<<b.id<<setw(4)<<b.entrance<<setw(4)<<b.exit<<" [";
            for(int e: b.from)
                cerr << setw(4) <<e;
            cerr<<"] [";
            for(int t: b.to)
                cerr << setw(4) <<t;
            cerr<<"]"<<endl;
        }
#endif // ACTVAR_DEBUG
    }
}
