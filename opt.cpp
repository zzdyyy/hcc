#include "stdafx.h"

#define OPT_DAG //option: open DAG for optimization
#define OPT_GEN //option: open general optimization


int calculate(int a, int b, int op)
{
    switch(op)
    {
        case qi::ADD: return (a+b);
        case qi::SUB: return (a-b);
        case qi::DIV: return (a/b);
        case qi::MUL: return (a*b);
        default: assert(0);
    }
}

void do_substitution(qoperand &qo, map<qoperand, qoperand> &sbsttt)
{
    auto iter = sbsttt.find(qo);
    if(iter != sbsttt.end())
    {
        qo.type = iter->second.type;
        qo.value = iter->second.value;
    }
}

//bblock level optimization
void opt_bblk_level(vector<qi> &qilist, vector<bblk> &bblklist, int bidx, actvartbl &avflow)
{
    bblk &b = bblklist[bidx];

    //variable substitution and constant combination
    map<qoperand, qoperand> sbsttt;//used for variable substitution
    //for each qi
    for(int qidx = b.entrance; qidx<=b.exit; ++qidx)
    {
        qi &q = qilist[qidx];
        //use of qo, do substitution
        switch(q.op)
        {
        case qi::ASSIGN: case qi::NEG:
        case qi::JZ: case qi::RET:
        case qi::PUSH: case qi::WR:
            do_substitution(q.A, sbsttt);
        }
        switch(q.op)
        {
        case qi::ADD: case qi::SUB:
        case qi::DIV: case qi::MUL: //binary arithmatic
        case qi::JEQ: case qi::JGE: case qi::JGT: case qi::JLE:
        case qi::JLT: case qi::JNE: //binary cmp
            do_substitution(q.A, sbsttt);
            do_substitution(q.B, sbsttt);
        }
        switch(q.op)
        {
        case qi::ARRAYASS:
            do_substitution(q.D, sbsttt);//intent to cross case
        case qi::ARRAYLOAD:
            do_substitution(q.B, sbsttt);
        }
        //constant combination
        switch(q.op)
        {
        case qi::ADD: case qi::SUB:
        case qi::DIV: case qi::MUL:
            if(q.A.type == qoperand::IMD && q.B.type == qoperand::IMD)
            {
                q.A = qoperand{qoperand::IMD, calculate(q.A.value, q.B.value, q.op), int_t};
                q.B = BLANKOP;
                q.op = qi::ASSIGN;
            }
            break;
        case qi::NEG:
            if(q.A.type == qoperand::IMD)
            {
                q.A = qoperand{qoperand::IMD, -q.A.value, int_t};
                q.op = qi::ASSIGN;
            }
        }
        //update substitution map
        switch(q.op)//if there is a define point, delete relate substitution in map
        {
        case qi::NEG://unary arithmetic
        case qi::ADD: case qi::SUB:
        case qi::DIV: case qi::MUL: //binary arithmetic
        case qi::ARRAYLOAD: case qi::CALL:
            for(auto it = sbsttt.begin(); it != sbsttt.end(); )
                if(it->first == q.D || it->second == q.D)
                    it = sbsttt.erase(it);
                else
                    ++it;
            break;
        case qi::RD:
            for(auto it = sbsttt.begin(); it != sbsttt.end(); )
                if(it->first == q.A || it->second == q.A)
                    it = sbsttt.erase(it);
                else
                    ++it;
        }
        if(q.op == qi::ASSIGN)
        {
            //stop the spread of q.D
            for(auto it = sbsttt.begin(); it != sbsttt.end(); )
                if(it->second == q.D)
                    it = sbsttt.erase(it);
                else
                    ++it;
            sbsttt[q.D] = q.A;//and spread the new q.D
        }
        else if(q.op == qi::CALL)
        {
            //function call, must assume every global variable may be change
            for(auto it = sbsttt.begin(); it != sbsttt.end(); )
                if(it->first.type == qoperand::GLB_OBJ)
                    it = sbsttt.erase(it);
                else
                    ++it;
        }
    }

    //do reduce

}

//do some small but efficient optimization
//such as copy-spread, dead-code deletion, and other thing
void general_optmize()
{
    buildbblks();
    //for each function
    for(size_t ct=0; ct<functbl.size(); ++ct)
    {
        //do active variable analysis
        vector<qi> &qilist = functbl[ct].qilist;
        vector<bblk> &bblklist = bblktbl[ct];
        actvartbl avflow;//function level
        actvar(functbl[ct], bblklist, avflow);
        //do some optimization to each bblock
        for(size_t bidx=0; bidx<bblklist.size(); ++bidx)
        {
            opt_bblk_level(qilist, bblklist, bidx, avflow);
        }

    }
}

void optimize()
{

#ifdef OPT_DAG
    opt_DAG();
#endif // OPT_DAG

#ifdef OPT_GEN
    general_optmize();
#endif // OPT_GEN
}
