#include "stdafx.h"

istream *src_input = nullptr;
ostream *lex_output = nullptr;

void usage()
{
    cout << "Usage: hcc [-l FILENAME] SOURCEFILE" << endl
        << endl
        << "       Read extended C0 source code from SOURCEFILE file and do" << endl
        << "       lexical analysis. The result will be output into stdout" << endl
        << "       by default." << endl
        << endl
        << "  -l FILENAME" <<endl
        << "       Only do lexical analysis and output the result to FILENAME." << endl
        << "       Assign FILENAME as - to indicate stdout." << endl
        << endl;
    exit(EXIT_FAILURE);
}

istream *open_input(string fname)
{
    //from stdin
    if(fname == "-")
        return &cin;

    //from file
    istream *inputfs = new ifstream(fname);
    if(*inputfs)
        return inputfs;
    else
    {
        cerr << "ERROR: Couldn't open input file." << endl;
        exit(EXIT_FAILURE);
    }
}

ostream *open_output(string fname)
{
    //to stdout
    if(fname == "-")
        return &cout;

    //to file
    ostream *outputfs = new ofstream(fname);
    if(*outputfs)
        return outputfs;
    else
    {
        cerr << "ERROR: Couldn't open output file." << endl;
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv)
{
    if (argc == 2)
    {
        src_input = open_input(argv[1]);
        lex_output = open_output("-");
        lex_init();
        lex_dump();
        return 0;
    }
    if (argc == 4 && string(argv[1]) == "-l")
    {
        src_input = open_input(argv[3]);
        lex_output = open_output(argv[2]);
        lex_init();
        lex_dump();
        return 0;
    }
    usage();
    return 0;
}
