#ifndef STDAFX_H_INCLUDED
#define STDAFX_H_INCLUDED

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <cstdlib>
#include <cassert>
#include <climits>
using namespace std;
typedef unsigned int uint;

//token type constant definition
enum token_t {
    chrlit_tk, strlit_tk, intlit_tk, id_tk, //complicate types
    const_tk, int_tk, char_tk, void_tk, if_tk, while_tk, switch_tk, case_tk, default_tk, return_tk,    //keywords
    semicln_tk, comma_tk, assign_tk, lbrkt_tk, rbrkt_tk, lprt_tk, rprt_tk, lbrc_tk, rbrc_tk, cln_tk,  //single signs
    addsub_tk, multdiv_tk, rltop_tk,     //operators
    eof_tk  //control token
};
enum addsub_tk_v { addop, subop };      // + -
enum multdiv_tk_v { multop, divop };    // * /
enum rltop_tk_v { ltop, leop, gtop, geop, neqlop, eqlop };  // < <= > >= != ==
enum type_t { void_t, int_t, char_t, string_t };
extern int size_of[];


//program level global declaration
extern int errcnt;
extern bool flg_lexonly;
extern bool flg_syxonly;
extern bool flg_tbl;
extern istream *src_input;
extern ostream *lex_output;
extern ostream *syx_output;
extern ostream *tbl_output;
extern ostream *asm_output;

//lex level global declaration
extern signed char ch; //last character
extern int cc;  //character counter
extern int lc;  //line counter
extern int tkntyp; //token type
extern int tknval; //number value, operator subtype
extern char tknchar;   //char literal
extern string tknstr;  //string literal, identifier name

//semantics level declaration
#include "semantics.h"

void lex_init();
void gettoken();
void lex_dump();

void syx_init();
void program();

void genasm();

void WARNING(string msg);
void ERROR(string msg);
void FATAL_ERROR(string msg);


template <typename T>
string tostr(const T &x)
{
    ostringstream o;
    if(o << x)
        return o.str();
    ERROR("error in to_string");
    return "(error)";
}

#endif // STDAFX_H_INCLUDED
