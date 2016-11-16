#ifndef STDAFX_H_INCLUDED
#define STDAFX_H_INCLUDED

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <map>
#include <cstdlib>
using namespace std;

//token type constant definition
enum token_t {
    chrlit_tk, strlit_tk, intlit_tk, id_tk, //complicate types
    const_tk, int_tk, char_tk, void_tk, if_tk, while_tk, case_tk, default_tk, return_tk,    //keywords
    semicln_tk, comma_tk, assign_tk, lbrkt_tk, rbrkt_tk, lprt_tk, rprt_tk, lbrc_tk, rbrc_tk, cln_tk,  //single signs
    addsub_tk, multdiv_tk, rltop_tk     //operators
};
enum addsub_tk_v { addop, subop };      // + -
enum multdiv_tk_v { multop, divop };    // * /
enum rltop_tk_v { ltop, leop, gtop, geop, neqlop, eqlop };  // < <= > >= != ==


//program level global declaration
extern istream *src_input;
extern ostream *lex_output;

//lex level global declaration
extern char ch; //last character
extern int cc;  //character counter
extern int lc;  //line counter

void lex_init();
void gettoken();
void lex_dump();

#endif // STDAFX_H_INCLUDED
