#ifndef SEMANTICS_H_INCLUDED
#define SEMANTICS_H_INCLUDED

struct tblitem
{
    enum obj_t { CONST, VAR, FUNCTION };
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
    enum opr_t { IMMEDIATE, GLB_OBJ, LCL_OBJ, TMP, LABEL };

    int type;   //the type of the operand, one of opr_t
    int value;  //value of immediate number; index of object in table;
                //No. of temp variables; No. of label
};

struct qinstruction
{
    enum oprt_t {
        ADD, SUB, NEG, MUL, DIV, JLT, JGT, JLE, JGE, JEQ, JNE, JZ, JMP,
        ASSIGN, ARRAYLOAD, ARRAYASS, PUSH, CALL, RD, WR, RET, LABEL,
    };
    qoperand A, B, D;
};

struct funcitem
{
    int index;  //index of the function in glbtbl
    int argnum; //amount of arguments
    int varamt;  //amount of local variables
    int tmpamt;  //amount of temp variables
    //TODO: int stored_space;
    vector<tblitem> lcltbl;//local symbol table
    vector<qinstruction> qi;//quad-element instruction table
};
extern vector<funcitem> functbl;//function table

void tbl_dump();
bool insertobj(int objtyp, const string& nm, int typ, bool isarray, int val=0);

#endif // SEMANTICS_H_INCLUDED
