#ifndef SEMANTICS_H_INCLUDED
#define SEMANTICS_H_INCLUDED

struct tblitem
{
    enum obj_t { CONST, VAR, FUNCTION };
    static const int NOUSE = 233;

    int objtype;    //type of the object, one of obj_t
    string name;    //name of the object
    int datatype;   //data type of a variable, item type of an array,
                    //and return type of a function. one of type_t
    bool isarray;   //if the object is an array
    int value;      //value of a constant number, index of string table,
                    //size of an array, index of function
    int addr;       //the address (offset) of the object
};
extern vector<tblitem> glbtbl;//global symbol table
extern vector<string> strtbl;//string table

struct qoperand
{
    enum opr_t { IMDINT, IMDCHAR, GLB_OBJ, LCL_OBJ, TMP, LABEL, BLANK, STRING };

    int type;   //the type of the operand, one of opr_t
    int value;  //value of immediate number; index of object in table;
                //No. of temp variables; No. of label
};

struct qi
{
    enum oprt_t {
        ADD, SUB, NEG, MUL, DIV, JLT, JGT, JLE, JGE, JEQ, JNE, JZ, JMP,
        ASSIGN, ARRAYLOAD, ARRAYASS, PUSH, CALL, RD, WR, RET, LABEL,
    };
    int op;
    qoperand A, B, D;
};

struct funcitem
{
    int index;  //index of the function in glbtbl
    int argnum; //amount of arguments
    int varsize;  //size of space used by local variables
    int tmpamt;  //amount of temp variables
    //TODO: int stored_space;
    vector<tblitem> lcltbl;//local symbol table
    vector<qi> qilist;//quad-element instruction table
};
extern vector<funcitem> functbl;//function table

extern int context;
extern qoperand foundopr;
extern const qoperand BLANKOP;

void tbl_dump();
bool findidt(const string &nm);
bool isfunction(const string &idt);
bool insertobj(int objtyp, const string& nm, int typ, bool isarray, int val=0);
bool insertpara(int typ, const string &nm);
void buildcontext(int rettyp, const string& nm);
void exitcontext();
qoperand arrayload(qoperand arropr, qoperand index);
void arrayass(qoperand arr, qoperand index, qoperand val);
void assign(qoperand var, qoperand val);
qoperand muldiv(int op, qoperand f1, qoperand f2);
qoperand addsub(int op, qoperand t1, qoperand t2);
qoperand neg(qoperand t);
void push(qoperand arg);
qoperand call(qoperand func);
void ret(bool withvalue, qoperand value);
void jz(qoperand cond, qoperand lbl);
void jifnot(int op, qoperand e1, qoperand e2, qoperand lbl);
void setlabel(qoperand lbl);
void jmp(qoperand lbl);
void rd(qoperand opr);
qoperand insertstr(const string &str);
void wr(qoperand opr);
qoperand newlabel();
void checkmain();
static inline tblitem &getitem(qoperand opr)
{
    return (opr.type == qoperand::GLB_OBJ)?
            glbtbl[opr.value]:
            functbl[context].lcltbl[opr.value];
}

#endif // SEMANTICS_H_INCLUDED
