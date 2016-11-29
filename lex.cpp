#include "stdafx.h"

//work variables
signed char ch = ' ';    //last character, initialized with space
int cc_start = 0; //character counter before the valid token
int cc = 0; //character counter
int lc = 1; //line counter
map<char, int> singletoken;
map<string, int> keyword;

//lexical output variables
int tkntyp; //token type
int tknval; //number value, operator subtype
char tknchar;   //char literal
string tknstr;  //string literal, identifier name

//Safely get char from src_input. When at EOF, return one more char and throw
//exception at next calling. This function is aimed to handling the tail problem
signed char getcs()
{
    static int eofamt = 0;
    char c = ' ';
    if(*src_input)//there are more chars to read
    {
        src_input->get(c);
        return (c>0)? c: ' ';
    }
    if(eofamt<2)
    {
        ++eofamt;
        return -1;
    }
    FATAL_ERROR("Source input incomplete.");//fatal error
    return ' ';
}

void lex_init()
{
    //fill in the table
    singletoken[';']=semicln_tk;
    singletoken[',']=comma_tk;
    singletoken['[']=lbrkt_tk;
    singletoken[']']=rbrkt_tk;
    singletoken['(']=lprt_tk;
    singletoken[')']=rprt_tk;
    singletoken['{']=lbrc_tk;
    singletoken['}']=rbrc_tk;
    singletoken[':']=cln_tk;
    keyword[string("const")]   = const_tk;
    keyword[string("int")]     = int_tk;
    keyword[string("char")]    = char_tk;
    keyword[string("void")]    = void_tk;
    keyword[string("if")]      = if_tk;
    keyword[string("while")]   = while_tk;
    keyword[string("switch")]    = switch_tk;
    keyword[string("case")]    = case_tk;
    keyword[string("default")] = default_tk;
    keyword[string("return")]  = return_tk;

    //read the first char
    ch = getcs(), ++cc;
}

//assuming that ch_before=='/'&&ch=='*'
void jump_multiline_comment()
{
    char ch_before = '/'; //the old value of ch
    do
    {
        ch_before = ch;
        ch = getcs(), ++cc;
        if(ch == '\n')
            cc=0, ++lc; //update counter
    }while(!(ch_before=='*' && ch=='/'));
    ch = getcs(), ++cc;
}

//read string literal. assuming that ch = '"'
bool getstrlit()
{
    bool iswarned = false;
    tknstr.clear();
    while(true)
    {
        ch = getcs(), ++cc;
        if(ch == '\n')
        {
            ERROR("String cannot get cross over lines.");//TODO: ERROR( string cannot get cross over lines )
            return true;
        }
        if(ch == '"')//end of string
        {
            ch = getcs(), ++cc;
            return true;
        }
        if(!iswarned && (ch < 32 || ch == 34 || ch >126))
        {
            iswarned = true;
            ERROR("Unsupported char found in the string.");//TODO: WARN( unsupported char );
        }
        tknstr += ch;
    }
}

//read char literal. assuming that ch = '\''
bool getchrlit()
{
    tknchar = ' ';
    ch = getcs(), ++cc;
    if(ch == '\'')
    {
        ERROR("Empty chracter literal.");//TODO:ERROR(empty character literal)
        ch = getcs(), ++cc;
        return true;
    }
    if(ch == '\n')
    {
        ERROR("Char literal cannot get cross over lines.");//TODO:ERROR( char literal cannot get cross over lines )
        return true;
    }
    tknchar = ch;
    if(!(ch == '+' || ch == '*' || ch == '_' || (ch >= 'a' && ch <='z') ||
          (ch >= 'A' && ch <='Z') || (ch >= '0' && ch <='9')))
        ERROR("This char isn't valid.");//TODO:ERROR( char set isn't allowed)
    ch = getcs(), ++cc;
    if(ch != '\'' && ch != '\n')
        ERROR("Char iteral is too long.");//TODO:ERROR( literal is too long )
    while(ch != '\'')
    {
        if(ch == '\n')
        {
            ERROR("Char literal cannot get cross over lines.");//TODO:ERROR( char literal cannot get cross over lines )
            return true;
        }
        ch = getcs(), ++cc;
    }
    ch = getcs(), ++cc;
    return true;
}

//read integer literal. assuming that ch is the first digit
bool getintlit()
{
    tknval = (ch - '0');
    ch = getcs(), ++cc;
    if(tknval == 0 && ch >= '0' && ch <='9')//number start with 0
        ERROR("Number cannot be started with 0.");//TODO:ERROR( number cannot be started with 0
    while(ch >= '0' && ch <='9')
    {
        tknval *= 10;
        tknval += (ch - '0');
        ch = getcs(), ++cc;
    }
    return true;
}

//read identifier and keyword, assuming that ch is the first letter
bool getidentifier()
{
    tknstr.clear();
    tknstr += ch;
    ch = getcs(), ++cc;
    while(ch == '_' || (ch >= 'a' && ch <='z') || (ch >= 'A' && ch <='Z') || (ch >= '0' && ch <='9'))
    {
        tknstr += ch;
        ch = getcs(), ++cc;
    }
    if(keyword.find(tknstr) != keyword.end())//tknstr is keyword
        tkntyp = keyword[tknstr];
    return true;
}

//Read a token and fill the valid output into output variables,
//pass the EOF exception out.
//One character read before is needed.
void gettoken()
{
    while(true)//loop until one valid char is read
    {
        cc_start = cc;

        //jump the white char
        if(ch==' ' || ch=='\n' || ch=='\t')
        {
            if(ch == '\n')
                cc=0, ++lc; //update counter
            ch = getcs(), ++cc; //read new char
            continue;
        }

        if(ch >= '0' && ch <='9')//number literal
        {
            tkntyp = intlit_tk;
            if(getintlit())
                return;
            else
                continue;
        }

        //identifier or keyword
        if(ch == '_' || (ch >= 'a' && ch <='z') || (ch >= 'A' && ch <='Z'))
        {
            tkntyp = id_tk;
            if(getidentifier())
                return;
            else
                continue;
        }

        //if ch is a single sign, output it
        if(singletoken.find(ch) != singletoken.end())   //ch in the map
        {
            tkntyp = singletoken[ch];
            ch = getcs(), ++cc;
            return;
        }

        switch(ch)
        {
        case '+':
            tkntyp = addsub_tk;
            tknval = addop;
            ch = getcs(), ++cc;
            return;
        case '-':
            tkntyp = addsub_tk;
            tknval = subop;
            ch = getcs(), ++cc;
            return;
        case '*':
            tkntyp = multdiv_tk;
            tknval = multop;
            ch = getcs(), ++cc;
            return;
        case '/':
            ch = getcs(), ++cc;
            if(ch == '/')//single line comment
                while(ch != '\n')
                    ch = getcs(), ++cc;
            else if(ch == '*')//multi-line comment
                jump_multiline_comment();
            else//division operator
            {
                tkntyp = multdiv_tk;
                tknval = divop;
                return;
            }
            break;//after jumping comment, continue to read token
        case '=':
            ch = getcs(), ++cc;
            if(ch == '=')//equals
            {
                tkntyp = rltop_tk;
                tknval = eqlop;
                ch = getcs(), ++cc;
                return;
            }
            else//assign
            {
                tkntyp = assign_tk;
                return;
            }
        case '<':
            ch = getcs(), ++cc;
            if(ch == '=')//less or equal than
            {
                tkntyp = rltop_tk;
                tknval = leop;
                ch = getcs(), ++cc;
                return;
            }
            else//less than
            {
                tkntyp = rltop_tk;
                tknval = ltop;
                return;
            }
        case '>':
            ch = getcs(), ++cc;
            if(ch == '=')//greater or equal than
            {
                tkntyp = rltop_tk;
                tknval = geop;
                ch = getcs(), ++cc;
                return;
            }
            else//greater than
            {
                tkntyp = rltop_tk;
                tknval = gtop;
                return;
            }
        case '!':
            ch = getcs(), ++cc;
            if(ch == '=')//not equal
            {
                tkntyp = rltop_tk;
                tknval = neqlop;
                ch = getcs(), ++cc;
                return;
            }
            ERROR("Unrecognized token '!'.");//TODO: ERROR(jumped unrecognized token '!')
            break;//jump unrecognized token, continue to read next token
        case '"':
            tkntyp = strlit_tk;
            if(getstrlit())
                return;
            break;
        case '\'':
            tkntyp = chrlit_tk;
            if(getchrlit())
                return;
            break;
        default:
            if(ch<0)
            {
                tkntyp = eof_tk;
                ch = getcs(), ++cc;
                return;
            }
            //TODO:ERROR
            ERROR("Unknown char code: "+tostr(int(ch)));
            ch = getcs(), ++cc;
        }

    }
}

//repeatedly call gettoken() and output the token into lex_output.
void lex_dump()
{
    int cnt = 0;
    ostream &out = *lex_output;
    map<int, string> tokentypename;
    map<int, string> rltoptypesign;
    map<int, char> signtype;

    tokentypename[chrlit_tk] = string("CharLiteral");
    tokentypename[strlit_tk] = string("StringLit..");
    tokentypename[intlit_tk] = string("IntLiteral");
    tokentypename[id_tk] = string("Identifier");
    tokentypename[const_tk] = string("KW: const");
    tokentypename[int_tk] = string("KW: int");
    tokentypename[char_tk] = string("KW: char");
    tokentypename[void_tk] = string("KW: void");
    tokentypename[if_tk] = string("KW: if");
    tokentypename[while_tk] = string("KW: while");
    tokentypename[switch_tk] = string("KW: switch");
    tokentypename[case_tk] = string("KW: case");
    tokentypename[default_tk] = string("KW: default");
    tokentypename[return_tk] = string("KW: return");
    tokentypename[semicln_tk] = string("Semicolon");
    tokentypename[comma_tk] = string("Comma");
    tokentypename[assign_tk] = string("Assign");
    tokentypename[lbrkt_tk] = string("LBracket");
    tokentypename[rbrkt_tk] = string("RBracket");
    tokentypename[lprt_tk] = string("LeftPar");
    tokentypename[rprt_tk] = string("RightPar");
    tokentypename[lbrc_tk] = string("LBrace");
    tokentypename[rbrc_tk] = string("RBrace");
    tokentypename[cln_tk] = string("Colon");
    tokentypename[addsub_tk] = string("AddSub");
    tokentypename[multdiv_tk] = string("MultDiv");
    tokentypename[rltop_tk] = string("RelationOp");

    rltoptypesign[ltop] = string("<");
    rltoptypesign[leop] = string("<=");
    rltoptypesign[gtop] = string(">");
    rltoptypesign[geop] = string(">=");
    rltoptypesign[neqlop] = string("!=");
    rltoptypesign[eqlop] = string("==");

    signtype[semicln_tk] = ';';
    signtype[comma_tk] = ',';
    signtype[assign_tk] = '=';
    signtype[lbrkt_tk] = '[';
    signtype[rbrkt_tk] = ']';
    signtype[lprt_tk] = '(';
    signtype[rprt_tk] = ')';
    signtype[lbrc_tk] = '{';
    signtype[rbrc_tk] = '}';
    signtype[cln_tk] = ':';
    out << left;

    while(tkntyp!=eof_tk)
    {
        ++cnt;
        gettoken();
        out << setw(4) << lc << setw(4) << cc_start << setw(4) << cc;
        switch (tkntyp)
        {
        case const_tk: case int_tk: case char_tk:
        case void_tk: case if_tk: case while_tk:
        case case_tk: case default_tk: case return_tk:
        case switch_tk:
            out << setw(5) << cnt << " " << setw(12) << "Keyword" << "   "
                << tknstr << endl;
            break;
        case id_tk:
            out << setw(5) << cnt << " " << setw(12) << tokentypename[tkntyp] << "   "
                << tknstr << endl;
            break;
        case intlit_tk:
            out << setw(5) << cnt << " " << setw(12) << tokentypename[tkntyp] << "   "
                << tknval << endl;
            break;
        case chrlit_tk:
            out << setw(5) << cnt << " " << setw(12) << tokentypename[tkntyp] << "   '"
                << tknchar << "'" << endl;
            break;
        case strlit_tk:
            out << setw(5) << cnt << " " << setw(12) << tokentypename[tkntyp] << "   \""
                << tknstr << "\"" << endl;
            break;
        case semicln_tk: case comma_tk: case assign_tk: case lbrkt_tk:
        case rbrkt_tk: case lprt_tk: case rprt_tk: case lbrc_tk:
        case rbrc_tk: case cln_tk:
            out << setw(5) << cnt << " " << setw(12) << tokentypename[tkntyp] << "   "
                << signtype[tkntyp] << endl;
            break;
        case addsub_tk:
            out << setw(5) << cnt << " " << setw(12) << tokentypename[tkntyp] << "   "
                << (tknval == addop ? '+':'-') << endl;
            break;
        case multdiv_tk:
            out << setw(5) << cnt << " " << setw(12) << tokentypename[tkntyp] << "   "
                << (tknval == addop ? '*' : '/') << endl;
            break;
        case rltop_tk:
            out << setw(5) << cnt << " " << setw(12) << tokentypename[tkntyp] << "   "
                << rltoptypesign[tknval] << endl;
            break;
        case eof_tk:
            out << setw(5) << cnt << " " << setw(12) << "EOF" << endl;
            break;
        }
    }
}
