#include "stdafx.h"

//do some small but efficient optimization
//such as copy-spread, dead-code deletion, and other thing
void general_optmize()
{
    buildbblks();

}

void optimize()
{
    opt_DAG();

    general_optmize();
}
