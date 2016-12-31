#include "stdafx.h"
//#define DAG_DEBUG
vector<qi> newlist;

struct dagnode
{
    int id;//id number
    int child1;//child 1
    int child2;//child 2
    int fcnt;//father counter
    qi q;//related qi to this node

    qoperand get_qo() const { return q.D; }
    int get_op() const { return q.op; }
};


//return if q is a arithmetic qi which support by DAG
bool isArithmetic(qi &q)
{
    switch(q.op)
    {
    case qi::ADD:
    case qi::SUB:
    case qi::NEG:
    case qi::MUL:
    case qi::DIV:
    case qi::ASSIGN:
        return true;
    default:
        return false;
    }
}

//check if qo is in dag. return existed node id or inserted id.
int getqonode(qoperand &qo, vector<dagnode> &dag, map<qoperand, int> &oprtbl)
{
    auto iter = oprtbl.find(qo);
    if(iter!=oprtbl.end())//found
    {
        return iter->second;
    }
    else
    {
        int id = dag.size();
        dag.push_back(dagnode{
                      id, -1, -1, 0, qi{qi::ASSIGN, qo, BLANKOP, newtmp(int_t)}
                      });
        oprtbl[qo] = id;
        return id;
    }
}

//check if (ai op bi) is in dag. return existed node id or inserted id.
int getansnode(int ai, int bi, int op, vector<dagnode> &dag)
{
    bool asso = (op == qi::ADD || op == qi::MUL);
    for(auto iter = dag.begin(); iter!=dag.end(); ++iter)
    {
        if(op == iter->get_op() && (
            (ai == iter->child1 && bi == iter->child2) ||
            (asso && bi == iter->child1 && ai == iter->child2)
            ))
            return iter->id;
    }

    //not found
    int id = dag.size();
    dag.push_back(dagnode{
                  id, ai, bi, 0, qi{op, dag[ai].get_qo(), dag[bi].get_qo(), newtmp(int_t)}
                  });
    dag[ai].fcnt ++;
    dag[bi].fcnt ++;
    return id;
}

int getansnode(int ai, int op, vector<dagnode> &dag)//unary op
{
    for(auto iter = dag.begin(); iter!=dag.end(); ++iter)
    {
        if(op == iter->get_op() && ai == iter->child1)
            return iter->id;
    }

    //not found
    int id = dag.size();
    dag.push_back(dagnode{
                  id, ai, -1, 0, qi{op, dag[ai].get_qo(), BLANKOP, newtmp(int_t)}
                  });
    dag[ai].fcnt ++;
    return id;
}

//regenerate qi with the DAG
void regen(vector<dagnode> &dag, map<qoperand, int> &oprtbl)
{
    vector<int> thequeue;
    vector<bool> counted(dag.size(), false);
    size_t cnt = 0;
    while(cnt < dag.size())
    {
        for(size_t i = 0; i<dag.size(); ++i)
        {
            if(dag[i].fcnt == 0 && !counted[i])
            {
                //count i and his left child, grandchild, grandgrandchild...
                int t=i;
                do
                {
                    thequeue.push_back(t);
                    counted[t] = true; ++cnt;
                    if(dag[t].child1>=0) dag[dag[t].child1].fcnt --;
                    if(dag[t].child2>=0) dag[dag[t].child2].fcnt --;
                    t = dag[t].child1;
                }while(t>=0 && dag[t].fcnt==0 && !counted[t]);
            }
        }
    }
    //insert qi by a inverse order
    for(auto riter = thequeue.rbegin(); riter != thequeue.rend(); ++riter)
    {
        newlist.push_back(dag[*riter].q);
    }
    //write values back into variable
    for(auto iter = oprtbl.begin(); iter != oprtbl.end(); ++iter)
    {
        if((iter->first).type != qoperand::GLB_OBJ &&
            (iter->first).type != qoperand::LCL_OBJ &&
            (iter->first).type != qoperand::TMP)
            continue;
        newlist.push_back(qi{qi::ASSIGN, dag[iter->second].get_qo(), BLANKOP, iter->first});
    }
}

//do DAG optimization to continuous arithmetic qi
void do_DAG(vector<qi> &oldlist, size_t &n)
{
    vector<dagnode> dag;
    map<qoperand, int> oprtbl;

    while(n<oldlist.size())
    {
        qi &q = oldlist[n];
        int ai, bi, ans;
        switch(q.op)
        {
        case qi::ADD:case qi::SUB:
        case qi::MUL:case qi::DIV:

            ai = getqonode(q.A, dag, oprtbl);
            bi = getqonode(q.B, dag, oprtbl);
            ans = getansnode(ai, bi, q.op, dag);
            oprtbl[q.D] = ans;
            break;

        case qi::NEG:

            ai = getqonode(q.A, dag, oprtbl);
            ans = getansnode(ai, q.op, dag);
            oprtbl[q.D] = ans;
            break;

        case qi::ASSIGN:
            ai = getqonode(q.A, dag, oprtbl);
            oprtbl[q.D] = ai;
            break;

        default:
            goto endlbl;
        }
        ++n;
    }
endlbl:

#ifdef DAG_DEBUG
    cerr << "#########################################" << endl;
    cerr << "DAG node table:" << endl;
    for(size_t i=0; i<dag.size(); ++i)
    {
        cerr << setw(4) << dag[i].id << setw(4) << dag[i].child1 << setw(4) << dag[i].child2
            << setw(4) << dag[i].fcnt << setw(4) << dag[i].get_op() << setw(4)
            << qotostr(dag[i].get_qo()) << endl;
    }
    cerr << "oprand map:" << endl;
    for(auto iter = oprtbl.begin(); iter != oprtbl.end(); ++iter)
    {
        cerr << setw(10) << qotostr(iter->first) << setw(4) << iter->second << endl;
    }
#endif // DAG_DEBUG

    regen(dag, oprtbl);

    return;
}

//do DAG optimization to all functions
void opt_DAG()
{
    //for each function run the optimization
    for(size_t ct=0; ct<functbl.size(); ++ct)
    {
        loadcontext(ct);

        newlist.clear();
        vector<qi> &oldlist = functbl[ct].qilist;

        size_t n = 0;
        while(n<oldlist.size())
        {
            if(!isArithmetic(oldlist[n]))
            {
                newlist.push_back(oldlist[n++]);
                continue;
            }

            //arithmetic qi
            do_DAG(oldlist, n);
        }

        newlist.swap(oldlist);

        exitcontext();
    }
}
