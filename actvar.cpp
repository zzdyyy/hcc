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

//return id of bblock; if the label points the exit, return -1
int lbltobblock(qoperand lbl, vector<qi> &qilist, vector<int> &entrcnt)
{
    size_t n=0;
    size_t len = qilist.size();
    for(n=0; n<len; ++n)
    {
        if(qilist[n].op == qi::LABEL && qilist[n].A.value == lbl.value)
            break;
    }
    assert(n<len);//supposed that would be found
    do{++n;}while(n<len && qilist[n].op == qi::LABEL);//jump the label
    if(n<len)
        return entrcnt[n];
    else
        return -1;
}

//non-repeat push_back
void push_back_nr(vector<int> &vec, int val)
{
    for(int item: vec)
        if(item == val)
            return;
    vec.push_back(val);
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
        //find all entrances
        for(size_t n=0; n<len; ++n)
        {
            if(n==0)
            {
                while(n<len && qilist[n].op == qi::LABEL)
                    ++n;
                if(n<len)
                    bblks.push_back(bblk(cnt,n));
                entrcnt[n]=cnt++;
            }
            //statement with labels or statement after jump instructions
            if(qilist[n].op == qi::LABEL || isjmp(qilist[n]))
            {
                do{++n;}while(n<len && qilist[n].op == qi::LABEL);
                if(n<len)
                {
                    //cerr<<n<<" ";
                    bblks.push_back(bblk(cnt, n));
                    entrcnt[n--]=cnt++;//
                }
            }
        }

        //fill exits and flow info
        for(bblk &b: bblks)
        {
            //fill exits point
            size_t n=b.entrance;
            do{++n;}while(n<len && entrcnt[n]<0);//move n to next entrance
            do{--n;}while(qilist[n].op == qi::LABEL);//move n to the exit of the bblk
            b.exit = n;

            //fill flow info, i.e. to and from
            if(qilist[n].op == qi::RET)
            {
                push_back_nr(b.to,-1);//flow to exit
                continue;
            }
            if(qilist[n].op == qi::JMP)
            {
                int bbid = lbltobblock(qilist[n].D, qilist, entrcnt);
                push_back_nr(b.to, bbid);//flow to destination
                if(bbid>=0)
                    push_back_nr(bblks[bbid].from, b.id);//flow from b
                continue;
            }
            //maybe flow into next bblock
            int next = b.id+1;
            if(size_t(next)>=bblks.size())
                next = -1;//exit block
            push_back_nr(b.to, next);
            if(next>=0)
                push_back_nr(bblks[next].from, b.id);
            //maybe flow to other bblock
            if(isjc(qilist[n]))
            {
                int bbid = lbltobblock(qilist[n].D, qilist, entrcnt);
                push_back_nr(b.to, bbid);
                if(bbid>=0)
                    push_back_nr(bblks[bbid].from, b.id);
            }
        }
        push_back_nr(bblks[0].from,-1);//begin->block0

#ifdef ACTVAR_DEBUG
        cerr<<"BBLOCKs in "<<glbtbl[functbl[ct].index].name<<"():"<<endl;
        for(bblk b : bblks)
        {
            cerr<<setw(4)<<b.id<<setw(4)<<b.entrance<<setw(4)<<b.exit<<" [";
            for(int e: b.from)
                cerr << setw(4) <<e;
            cerr<<"  ] [";
            for(int t: b.to)
                cerr << setw(4) <<t;
            cerr<<"  ]"<<endl;
        }
#endif // ACTVAR_DEBUG
    }
}
