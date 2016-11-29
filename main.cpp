#include "stdafx.h"

bool flg_lexonly = false;
bool flg_syxonly = false;
bool flg_tbl = false;
istream *src_input = nullptr;
ostream *lex_output = nullptr;
ostream *syx_output = nullptr;
ostream *tbl_output = nullptr;

void usage()
{
    cout << "Usage: hcc [-l FILENAME|-s FILENAME|-t FILENAME] SOURCEFILE" << endl
        << endl
        << "       Read extended C0 source code from SOURCEFILE file and do" << endl
        << "       semantic analysis. The relevant tables will be dumped into" << endl
        << "       std-out by default." << endl
        << endl
        << "  -l FILENAME" <<endl
        << "       Only do lexical analysis and output the result to FILENAME." << endl
        << "       Assign FILENAME as - to indicate std-out." << endl
        << endl
        << "  -s FILENAME" <<endl
        << "       Only do syntax analysis and output the result to FILENAME." << endl
        << "       Assign FILENAME as - to indicate std-out." << endl
        << endl
        << "  -t FILENAME" <<endl
        << "       Do semantic analysis and dump the relevant tables to FILENAME." << endl
        << "       Assign FILENAME as - to indicate std-out." << endl
        << endl;
    exit(EXIT_FAILURE);
}

istream *open_input(string fname)
{
    //from std-in
    if(fname == "-")
        return &cin;

    //from file
    istream *inputfs = new ifstream(fname);
    if(*inputfs)
        return inputfs;
    else
    {
        cerr << "ERROR: Could not open input file." << endl;
        exit(EXIT_FAILURE);
    }
}

ostream *open_output(string fname)
{
    //to std-out
    if(fname == "-")
        return &cout;

    //to file
    ostream *outputfs = new ofstream(fname);
    if(*outputfs)
        return outputfs;
    else
    {
        cerr << "ERROR: Could not open output file." << endl;
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv)
{
    if (argc == 2)
    {
        src_input = open_input(argv[1]);
        tbl_output = open_output("-");
        lex_init();
        syx_init();
        program();
        tbl_dump();
        return 0;
    }
    if (argc == 4 && string(argv[1]) == "-l")
    {
        flg_lexonly = true;
        src_input = open_input(argv[3]);
        lex_output = open_output(argv[2]);
        lex_init();
        lex_dump();
        return 0;
    }
    if (argc == 4 && string(argv[1]) == "-s")
    {
        flg_syxonly = true;
        src_input = open_input(argv[3]);
        syx_output = open_output(argv[2]);
        lex_init();
        syx_init();
        program();
        return 0;
    }
    if (argc == 4 && string(argv[1]) == "-t")
    {
        flg_tbl = true;
        src_input = open_input(argv[3]);
        tbl_output = open_output(argv[2]);
        lex_init();
        syx_init();
        program();
        tbl_dump();
        return 0;
    }
    usage();
    return 0;
}
