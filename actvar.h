#ifndef ACTVAR_H_INCLUDED
#define ACTVAR_H_INCLUDED

struct bblk
{
    int id=-1;//block number
    int entrance=-1;//entrance address
    int exit=-1;//exit address
    vector<int> from;
    vector<int> to;

    bblk(int theid, int theentrance): id(theid), entrance(theentrance) {}
};
extern vector<vector<bblk> > bblktbl;

struct actvartbl
{
    vector<set<int> > use;
    vector<set<int> > def;
    vector<set<int> > in;
    vector<set<int> > out;
};

void actvar(funcitem &func, vector<bblk> &bblks, actvartbl &answer);
void buildbblks();

#endif // ACTVAR_H_INCLUDED
