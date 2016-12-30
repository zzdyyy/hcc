#include "stdafx.h"

bool flg_lexonly = false;
bool flg_syxonly = false;
bool flg_tbl = false;
bool flg_opt = false;
istream *src_input = nullptr;
ostream *lex_output = nullptr;
ostream *syx_output = nullptr;
ostream *tbl_output = nullptr;
ostream *asm_output = nullptr;

void usage()
{
    cout << "Usage: hcc [-c FILENAME|-l FILENAME|-s FILENAME|" << endl
        << "            -t FILENAME] [-o] SOURCEFILE" << endl
        << endl
        << "       Read extended C0 source code from SOURCEFILE file and compile" << endl
        << "       to MASM source code. The generated code will be output into" << endl
        << "       std-out by default." << endl
        << endl
        << "  -c FILENAME" <<endl
        << "       Output the generated code to FILENAME. Assign FILENAME as - to" << endl
        << "       indicate std-out." << endl
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
        << endl
        << "  -o" <<endl
        << "       Do optimization to generate more concise code." << endl
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
    set_terminate(handler);

    //start parsing argument
    int n=1;
    if(argc < 2)
        usage();
    while(n<argc)
    {
        string arg(argv[n++]);
        if(arg == "-c")
        {
            if(flg_syxonly || flg_lexonly || asm_output!=nullptr || n>=argc)
                usage();
            arg = string(argv[n++]);
            asm_output = open_output(arg);
        }
        else if(arg == "-l")
        {
            if(lex_output!=nullptr || n>=argc)
                usage();
            arg = string(argv[n++]);
            lex_output = open_output(arg);
            flg_lexonly = true;
        }
        else if(arg == "-s")
        {
            if(flg_lexonly || syx_output!=nullptr || n>=argc)
                usage();
            arg = string(argv[n++]);
            syx_output = open_output(arg);
            flg_syxonly = true;
        }
        else if(arg == "-t")
        {
            if(flg_syxonly || flg_lexonly || tbl_output!=nullptr || n>=argc)
                usage();
            arg = string(argv[n++]);
            tbl_output = open_output(arg);
            flg_tbl = true;
        }
        else if(arg == "-o")
        {
            if(flg_opt)
                usage();
            flg_opt = true;
        }
        else
        {
            if(src_input != nullptr)
                usage();
            src_input = open_input(arg);
        }
    }
    if(src_input==nullptr)
        src_input = open_input("-");
    if(asm_output==nullptr)
        asm_output = open_output("-");

    //start working

    //lexical
    lex_init();
    if(flg_lexonly)
    {
        lex_dump();
        return 0;
    }

    //syntax and semantics
    syx_init();
    program();
    if(flg_syxonly)
        return 0;

    //optimization
    if(flg_opt)
    {
        cerr << "optimization is opened" << endl;
        if(flg_tbl)
        {
            tbl_dump();
            return 0;
        }
        genasm();
    }
    else
    {
        if(flg_tbl)
        {
            tbl_dump();
            return 0;
        }
        genasm();
    }

    return 0;
}
