#include "stdafx.h"
#define BBLOCK_DEBUG
#define ACTVAR_DEBUG

vector<vector<bblk> > bblktbl;

bool isjmp(qi &q)
{
    switch(q.op)
    {
    case qi::JEQ: case qi::JGE: case qi::JGT: case qi::JLE:
    case qi::JLT: case qi::JNE: case qi::JMP: case qi::JZ:
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

//build basic block for all functions
void buildbblks()
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

#ifdef BBLOCK_DEBUG
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
#endif // BBLOCK_DEBUG
    }
}

#ifdef ACTVAR_DEBUG
string actset2str(set<int> &theset, funcitem &func)
{
    ostringstream o;
    for(int i: theset)
        o<<func.lcltbl[i].name<<" ";
    return "{"+o.str()+"}";
}
#endif // ACTVAR_DEBUG

void actvar(funcitem &func, vector<bblk> &bblks, actvartbl &answer)
{
    vector<qi> &qilist = func.qilist;
    int bblkn = bblks.size();

    vector<set<int>> uses(bblkn, set<int>());
    vector<set<int>> defs(bblkn, set<int>());
    vector<set<int>> ins(bblkn, set<int>());
    vector<set<int>> outs(bblkn, set<int>());

    //fill in use and def
    for(int n=0; n<bblkn; ++n)//for each bblocks
    {
        bblk &b = bblks[n];
        set<int> &use=uses[n];
        set<int> &def=defs[n];

        for(int m=b.entrance; m<=b.exit; ++m)//for each qi in bblocks
        {
            qi &q = qilist[m];
            //use
            switch(q.op)
            {
            case qi::ASSIGN: case qi::NEG:
            case qi::JZ: case qi::RET:
            case qi::PUSH: case qi::WR:
                if(q.A.type == qoperand::LCL_OBJ &&
                   def.find(q.A.value)==def.end())
                    use.insert(q.A.value);
            }
            switch(q.op)
            {
            case qi::ADD: case qi::SUB:
            case qi::DIV: case qi::MUL: //binary arithmatic
            case qi::JEQ: case qi::JGE: case qi::JGT: case qi::JLE:
            case qi::JLT: case qi::JNE: //binary cmp
                if(q.A.type == qoperand::LCL_OBJ &&
                   def.find(q.A.value)==def.end())
                    use.insert(q.A.value);
                if(q.B.type == qoperand::LCL_OBJ &&
                   def.find(q.B.value)==def.end())
                    use.insert(q.B.value);
            }
            switch(q.op)
            {
            case qi::ARRAYASS:
                if(q.D.type == qoperand::LCL_OBJ &&
                   def.find(q.D.value)==def.end())
                    use.insert(q.D.value);
            case qi::ARRAYLOAD:
                if(q.B.type == qoperand::LCL_OBJ &&
                   def.find(q.B.value)==def.end())
                    use.insert(q.B.value);
            }

            //def
            switch(q.op)
            {
            case qi::ASSIGN: case qi::NEG://unary arithmetic
            case qi::ADD: case qi::SUB:
            case qi::DIV: case qi::MUL: //binary arithmetic
            case qi::ARRAYLOAD: case qi::CALL:
                if(q.D.type == qoperand::LCL_OBJ &&
                   use.find(q.D.value)==use.end())
                    def.insert(q.D.value);
                break;
            case qi::RD:
                if(q.A.type == qoperand::LCL_OBJ &&
                   use.find(q.A.value)==use.end())
                    def.insert(q.A.value);
            }

        }
    }

    //loop and expand sets by flow
    bool flag=true;//need another loop?
    int num;
    while(flag)
    {
        flag = false;
        for(int n=bblkn-1; n>=0; --n)//for each bblocks
        {
            bblk &b = bblks[n];
            set<int> &use=uses[n];
            set<int> &def=defs[n];
            set<int> &in=ins[n];
            set<int> &out=outs[n];

            //`out` = Union( `in` of every `to` )
            num=out.size();
            for(int i: b.to)
            {
                if(i>=0)
                    out.insert(ins[i].begin(), ins[i].end());
            }
            if(out.size()-num>0)//changed
                flag = true;

            //`in` = (`out` - `def`) union `use`
            in.clear();
            in.insert(out.begin(), out.end());
            for(int a: def)
            {
                auto iter = in.find(a);
                if(iter!=in.end())
                    in.erase(iter);
            }
            in.insert(use.begin(), use.end());
        }
    }

    #ifdef ACTVAR_DEBUG
    cerr<<"//////////////////////////////////"<<endl;
    cerr<<"Active varible of function "<<glbtbl[func.index].name<<"()"<<endl;
    cerr<<setw(4)<<"BLK"<<setw(15)<<"def"<<setw(15)<<"use"<<setw(15)<<"in"<<setw(15)<<"out"<<endl;
    for(int n=0; n<bblkn; ++n)
        cerr<<setw(4)<<n<<setw(15)<<actset2str(defs[n],func)
            <<setw(15)<<actset2str(uses[n],func)
            <<setw(15)<<actset2str(ins[n],func)
            <<setw(15)<<actset2str(outs[n],func)<<endl;
    cerr<<"//////////////////////////////////"<<endl;
    #endif // ACTVAR_DEBUG

    answer.use.swap(uses);
    answer.def.swap(defs);
    answer.in.swap(ins);
    answer.out.swap(outs);
}





