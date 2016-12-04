#include"stdafx.h"
int errcnt = 0;

void WARNING(string msg)
{
    cerr << "WARNING: [Near line " << lc << ", col " << cc << "] " << msg << endl;
}

//TODO: improve the error function
void ERROR(string msg)
{
    cerr << "ERROR: [Near line " << lc << ", col " << cc << "] " << msg << endl;

    //TODO: only for debug.
    /*cerr << "tknchar: " << tknchar << endl
        << "tknstr:" << tknstr << endl
        << "tknval:" << tknval << endl
        << "tkntyp:" << tkntyp << endl;*/
    //exit(EXIT_FAILURE);
    ++errcnt;
}

void FATAL_ERROR(string msg)
{
    cerr << "FATAL ERROR: [Near line " << lc << ", col " << cc << "] " << msg << endl;
    cerr << "hcc terminated." << endl;
    exit(EXIT_FAILURE);
}


