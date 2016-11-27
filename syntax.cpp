#include"stdafx.h"

map<int, int> token_to_type;
struct function_item{
    string funcname;
    int paraamt;
    int rettyp;
};
vector<function_item> old_functbl;

void syx_out(string msg);
void syx_init();
void nvtype(int &type);
void integer(int &val);
void constant(int &consttyp, int &val);
void program();
void constdef();
void declheader(bool allowfunc, int &step);
void vardeftail(int type, string &idt);
void funcdeftail(int type, string &idt);
void nfuncdef();
void paralist(int &paraamt);
void cmpdstmts();
void expression();
void term();
void factor();
void funccall(int &type);
void arglist();
void statement();
void if_stmt();
void while_stmt();
void condition();
void switch_stmt();
void return_stmt();
void printf_stmt();
void scanf_stmt();
void assign_stmt();

inline bool istype()
{
    return (token_to_type.find(tkntyp) != token_to_type.end());
}

bool isfunction(const string &idt)
{
    for(auto item : old_functbl)
        if(item.funcname == idt)
            return true;
    return false;
}

void syx_out(string msg)
{
    if(flg_syxonly)
        *syx_output << "SYNTAX: [Near line " << lc << ", col " << cc << "] " << msg << endl;
}

void syx_init()
{
    token_to_type[void_tk] = void_t;
    token_to_type[int_tk] = int_t;
    token_to_type[char_tk] = char_t;

    gettoken();
}

//read a non-void type token and output type_t, assuming that tkntyp==char_tk||int_tk
void nvtype(int &type)
{
    type = token_to_type[tkntyp];
    gettoken();
}

//read a signed integer. assuming that tkntyp == addsub_tk||intlit_tk
void integer(int &val)
{
    if(tkntyp == intlit_tk)//no leading +|-
        val = tknval;
    else
    {
        assert(tkntyp == addsub_tk);
        bool sign = (tknval == addop);
        gettoken();
        if(tkntyp != intlit_tk)
        {
            ERROR("expected integer literal.");
            val = 0;
            gettoken();
            return;
        }
        val = sign? tknval: -tknval;
        if(!sign && tknval == 0)// -0
            WARNING("-0 is not a good style.");
    }
    gettoken();
}

//read an integer or charlit. NO assuming the correctness
void constant(int &consttyp, int &val)
{
    switch(tkntyp)
    {
    //integer
    case addsub_tk:
    case intlit_tk:
        consttyp = int_t;
        integer(val);
        return;
    //charlit
    case chrlit_tk:
        consttyp = char_t;
        val = (int)tknchar;
        break;

    default:
        ERROR("Expected a constant.");
        val = 0;
    }
    gettoken();
}

//read whole program, stoped with main. assuming one token is read
void program()
{
    int step=0;//0-const 1-var 2-func

    while(tkntyp != eof_tk)
    {
        switch(tkntyp)
        {
        case const_tk:
            if(step>0)
                WARNING("Constant declaration after other declaration.");
            constdef();
            break;

        case void_tk:
            step = 2;
            nfuncdef();
            break;

        default:
            if(istype())//begins with type
                declheader(true, step);
            else
                ERROR("Expected declaration.");
        }
    }
    //TODO: checkmain() in sematic.
    syx_out("Program read finished.");
}

//read constant definition. assuming tkntyp == const_tk
void constdef()
{
    string idt;
    int type = int_t;
    int consttype = int_t;
    int value = 0;
    syx_out("Constant definition begin.");

    gettoken();
    if(!istype() || tkntyp == void_tk)// tkntyp is not a non-void type
    {
        ERROR("Expected a non-void type.");
        gettoken();
    }
    else
        nvtype(type);//read a non-void type
    syx_out("type_t: " + tostr(type));

    while(true)//read all const
    {
        if(tkntyp != id_tk)
        {
            ERROR("Expected an identifier.");
        }
        else
            idt = tknstr;
        syx_out("name: " + idt);//TODO
        gettoken();

        if(tkntyp != assign_tk)
        {
            ERROR("Expected assignment operator '='.");
        }
        gettoken();

        constant(consttype, value);
        if(consttype != type)
            WARNING("The type of initial value mismatches with the const declaring.");
        insertobj(tblitem::CONST, idt, type, false, value);
        syx_out("init: " + tostr(value));

        if(tkntyp == comma_tk)
        {
            gettoken();
            continue;
        }
        else if(tkntyp == semicln_tk)
        {
            gettoken();
            break;
        }
        else
        {
            ERROR("Expected comma or semicolon.");
            while(tkntyp != semicln_tk)
                gettoken();
            gettoken();
            break;
        }
    }
    syx_out("Constant definition end.");
}

//read declaration of variable or function. allowfunc=allow function? step will be update
//to indicate the order of const, variable, function declaration
//assuming that sym is a type
void declheader(bool allowfunc, int &step)
{
    int type = int_t;
    string idt;
    syx_out("Function/variable definition begin.");

    nvtype(type);
    syx_out("type_t: " + tostr(type));

    if(tkntyp != id_tk)
        ERROR("Expected identifier.");
    else
        idt = tknstr;//saving id
    syx_out("name: " + idt);
    gettoken();

    //time to determine variable or function
    if(tkntyp == lprt_tk)//function
    {
        if(!allowfunc)
        {
            ERROR("Function cannot be defined here.");
            do { gettoken(); } while (tkntyp != rprt_tk);   //jump paralist
            if(tkntyp == lbrc_tk)
                do { gettoken(); } while (tkntyp != rbrc_tk);   //jump function body
            gettoken();
            return;
        }
        step = 2;
        funcdeftail(type, idt); //TODO: call with type and identifier
    }
    else if(tkntyp == lbrkt_tk || tkntyp == comma_tk || tkntyp == semicln_tk)
    {
        if(step>1)
            WARNING("Variable declaring after function declaring / statement.");
        if(step<1)
            step = 1;
        vardeftail(type, idt);   //TODO: call with type and identifier
    }
    else
    {
        ERROR("Unfinished declaration.");
        do { gettoken(); } while (tkntyp != semicln_tk);gettoken();
    }
}

//read var defs. assuming that tkntyp == lbrkt|comma|semicolon
void vardeftail(int type, string &idt)
{
    int sz = 0;//size of an array
    syx_out("Variable declaration go on.");

    if(tkntyp == lbrkt_tk)//array
    {
        gettoken();
        sz = tknval;
        if(tkntyp != intlit_tk || tknval <=0)
        {
            ERROR("Expected a positive int literal as size of array.");
            sz = 1;
            do { gettoken(); } while (tkntyp != rbrkt_tk);
        }
        else
        {
            //TODO
            syx_out("array, size: " + tostr(sz));
            gettoken();// get ]
        }
        gettoken();
    }
    //TODO
    while(tkntyp == comma_tk)
    {
        gettoken();
        if(tkntyp != id_tk)
            ERROR("Expected an identifier.");
        //TODO: save the id
        syx_out("name: " + tostr(tknstr));
        gettoken();

        if(tkntyp == lbrkt_tk)
        {
            gettoken();
            sz = tknval;
            if(tkntyp != intlit_tk || tknval <=0)
            {
                ERROR("Expected a positive int literal as size of array.");
                sz = 1;
                do { gettoken(); } while (tkntyp != rbrkt_tk);
            }
            else
            {
                //TODO
                syx_out("array, size: " + tostr(sz));
                gettoken();// get ]
            }
            gettoken();
        }
        //TODO
    }
    if(tkntyp != semicln_tk)
    {
        ERROR("Expected a semicolon or a comma.");
        do { gettoken(); } while (tkntyp != semicln_tk);
    }
    gettoken();//pre read
    syx_out("Variable declaration end.");
}

//read func defs. assuming that tkntyp == lprt_tk
void funcdeftail(int type, string &idt)
{
    int paraamt = 0;
    syx_out("Function declaration go on.");

    paralist(paraamt);
    old_functbl.push_back(function_item{idt, paraamt, type});
    cmpdstmts();
    syx_out("Function declaration end.");
}

//AT: tkntype = void_t
void nfuncdef()
{
    string idt;
    syx_out("Void function definition begin.");
    gettoken();//skip void

    if(tkntyp != id_tk)
        ERROR("Expected identifier.");
    idt = tknstr;
    syx_out("name: " + idt);
    gettoken();

    if(tkntyp != lprt_tk)
        ERROR("Void function needs parameter list.");
    funcdeftail(void_t, idt);
}

//output the para amount
void paralist(int &paraamt)
{
    int type = int_t;
    string idt;
    paraamt = 0;

    if(tkntyp != lprt_tk)
        ERROR("Expected left parenthesis.");
    gettoken();

    while(tkntyp != rprt_tk)
    {
        ++paraamt;

        if(!istype() || tkntyp == void_tk)// tkntyp is not a non-void type
        {
            ERROR("Expected a non-void type.");
            gettoken();
        }
        else
            nvtype(type);//read a non-void type
        syx_out("para"+tostr(paraamt)+" type_t: "+tostr(type));

        if(tkntyp != id_tk)
            ERROR("Expected identifier.");
        idt = tknstr;
        syx_out("name: "+idt);
        //...

        gettoken();

        if(tkntyp == comma_tk)
            gettoken();
    }
    gettoken();
}

void cmpdstmts()
{
    int step=0;//0-const 1-var

    if(tkntyp != lbrc_tk)
        ERROR("Expected left brace.");
    gettoken();

    while(tkntyp != rbrc_tk)
    {
        switch(tkntyp)
        {
        case const_tk:
            if(step>0)
                WARNING("Constant declaration after other declaration.");
            constdef();
            break;

        default:
            if(istype())//begins with type
                declheader(false, step);
            else
            {
                step = 2;
                statement();
            }
        }
    }
    gettoken();
}

void expression()
{
    if(tkntyp == addsub_tk)
    {
        //...
        gettoken();
    }

    term();
    while(tkntyp == addsub_tk)
    {
        gettoken();

        term();
    }
}

void term()
{
    factor();
    while(tkntyp == multdiv_tk)
    {
        gettoken();

        factor();
    }
}

void factor()
{
    int type = int_t;
    int value = 0;

    if(tkntyp == id_tk && isfunction(tknstr))//is a function
    {
        funccall(type);
        if(type == void_t)
            ERROR("Function call in a expression cannot return void value.");
        return;
    }
    switch(tkntyp)
    {
    case id_tk://variable
        //save the var
        gettoken();
        if(tkntyp != lbrkt_tk)//simple variable
        {
            return;
        }
        else
        {
            gettoken();
            expression();
            if(tkntyp != rbrkt_tk)
            {
                ERROR("Expected a right bracket.");
                do { gettoken(); } while (tkntyp != rbrkt_tk);
            }
            gettoken();
            return;
        }

    case lprt_tk://(expr)
        gettoken();
        expression();
        if(tkntyp != rprt_tk)
        {
            ERROR("Expected a right parenthesis.");
            do { gettoken(); } while (tkntyp != rprt_tk);
        }
        gettoken();
        return;

    //constant
    case chrlit_tk:
    case addsub_tk:
    case intlit_tk:
        constant(type, value);
        return;

    default:
        ERROR("Not a valid factor.");
    }
}

//return the type of function. AT: tkntyp == id_t(function)
void funccall(int &type)
{
    for(auto item: old_functbl)
        if(item.funcname == tknstr)
        {
            type = item.rettyp;
            break;
        }
    gettoken();

    arglist();//TODO: return length, compare with declaration
}

void arglist()
{
    if(tkntyp != lprt_tk)
        ERROR("Expected argument list.");
    gettoken();

    while(tkntyp != rprt_tk)
    {
        expression();

        if(tkntyp == comma_tk)
            gettoken();//TODO: may cause that , is not needed
    }

    gettoken();
}

void statement()
{
    if(tkntyp == id_tk && isfunction(tknstr))//function call
    {
        int rettyp;
        funccall(rettyp);
        if(tkntyp != semicln_tk)
        {
            ERROR("Expected semicolon."+tostr(tkntyp));
            do { gettoken(); } while (tkntyp != semicln_tk);
        }
        gettoken();
        return;
    }
    switch(tkntyp)
    {
    case if_tk:
        if_stmt();
        break;

    case while_tk:
        while_stmt();
        break;

    case return_tk:
        return_stmt();
        break;

    case switch_tk:
        switch_stmt();
        break;

    case semicln_tk:
        gettoken();
        break;

    case lbrc_tk:
        gettoken();
        while(tkntyp != rbrc_tk)
            statement();
        gettoken();
        break;

    default:
        if(tkntyp != id_tk)
            ERROR("Not a valid statement.");
        if(tknstr == "printf")
            printf_stmt();
        else if(tknstr == "scanf")
            scanf_stmt();
        else
            assign_stmt();
    }
}

//AT: tkntyp == if
void if_stmt()
{
    gettoken();

    if(tkntyp != lprt_tk)
        ERROR("Expected left parenthesis.");
    gettoken();

    if(tkntyp == rprt_tk)
    {
        ERROR("Condition cannot be empty.");
    }
    condition();

    if(tkntyp != rprt_tk)
        ERROR("Expected right parenthesis.");
    gettoken();

    statement();
}

//AT: tkntyp == if
void while_stmt()
{
    gettoken();

    if(tkntyp != lprt_tk)
        ERROR("Expected left parenthesis.");
    gettoken();

    if(tkntyp == rprt_tk)
    {
        ERROR("Condition cannot be empty.");
    }
    condition();

    if(tkntyp != rprt_tk)
        ERROR("Expected right parenthesis.");
    gettoken();

    statement();
}

void condition()
{
    expression();

    if(tkntyp != rltop_tk)
        return;
    gettoken();

    expression();
}

void switch_stmt()
{
    gettoken();

    if(tkntyp != lprt_tk)
        ERROR("Expected left parenthesis.");
    gettoken();

    if(tkntyp == rprt_tk)
    {
        ERROR("Expected expression.");
    }
    expression();

    if(tkntyp != rprt_tk)
        ERROR("Expected right parenthesis.");
    gettoken();

    if(tkntyp != lbrc_tk)
        ERROR("Expected left brace.");
    gettoken();

    do
    {
        gettoken();

        int type = int_t, value = 0;
        constant(type,value);

        if(tkntyp != cln_tk)
        ERROR("Expected colon.");
        gettoken();

        statement();

    }while(tkntyp == case_tk);

    if(tkntyp == default_tk)
    {
        gettoken();

        if(tkntyp != cln_tk)
        ERROR("Expected colon.");
        gettoken();

        statement();
    }


    gettoken();
}

void return_stmt()
{
    gettoken();

    if(tkntyp != semicln_tk)
        expression();

    if(tkntyp != semicln_tk)
        ERROR("Expected semicolon.");
    gettoken();
    syx_out("Return statement.");
}

void printf_stmt()
{
    gettoken();

    if(tkntyp != lprt_tk)
        ERROR("Expected left parenthesis.");
    gettoken();

    if(tkntyp == strlit_tk)
    {
        //TODO:do something
        gettoken();

        if(tkntyp == comma_tk)
        {
            gettoken();

            expression();
        }
    }
    else
    {
        expression();
    }

    if(tkntyp != rprt_tk)
        ERROR("Expected right parenthesis.");
    gettoken();

    if(tkntyp != semicln_tk)
        ERROR("Expected semicolon.");
    gettoken();
}

void scanf_stmt()
{
    gettoken();

    if(tkntyp != lprt_tk)
        ERROR("Expected left parenthesis.");
    gettoken();

    if(tkntyp != id_tk)
        ERROR("Expected identifier.");
    do
    {
        //...
        gettoken();

        if(tkntyp == comma_tk)
            gettoken();

    }while(tkntyp == id_tk);

    if(tkntyp != rprt_tk)
        ERROR("Expected right parenthesis.");
    gettoken();

    if(tkntyp != semicln_tk)
        ERROR("Expected semicolon.");
    gettoken();
}

void assign_stmt()
{
    //save identifier
    gettoken();

    if(tkntyp == lbrkt_tk)//array
    {
        gettoken();

        expression();

        if(tkntyp != rbrkt_tk)
            ERROR("Expected a right bracket.");
        gettoken();
    }

    if(tkntyp != assign_tk)
        ERROR("Expected assignment sign.");
    gettoken();

    expression();

    if(tkntyp != semicln_tk)
        ERROR("Expected semicolon.");
    gettoken();
}

