//calc.c
//功能性测试：本程序是一个简单的表达式计算器，可以计算整数之间的加减乘除（无括号）
//的四则混合运算。输入一个算式，以e（因为程序中无法使用'='）结尾回车即可

//测试要点：
//常量变量定义和使用，函数递归调用，各种语句结构，输入输出语句，等

const char chradd = '+', chrsub = '-', chrmult = '*', chrdiv = '/';
const char chreqv = 'e' ;
const int num = 0, add = 1, sub = 2, mult = 3, div = 4, eql = 5;    //符号类型
char newln;

char c;                 //读取的字符
int symtype, value;     //符号类型、符号数值
int error;              //错误号，0为无错误

int isNum(char c)       //判断c是否为数
{
    if(c < '0')
        return (0);
    if(c > '9')
        return (0);
    return (1);
}

void getsym()           //读一个符号
{
    if(error!=0)
        return;
    
    while(c==32)//space
        scanf(c);
    switch(c)
    {
        case '+': { symtype = add; scanf(c); return; }
        case '-': { symtype = sub; scanf(c); return; }
        case '*': { symtype = mult; scanf(c); return; }
        case '/': { symtype = div; scanf(c); return; }
        case 'e': { symtype = eql; scanf(c); return; }
    }
    if(1-isNum(c))  //!isNum(c)
    {
        error = 1;
        return;
    }
    
    symtype = num;
    value = 0;
    while(isNum(c))
    {
        value = value * 10;
        value = value + (c-'0');
        scanf(c);
    }
}

int getterm(int val, int type) //读入一项，传入之前的结果和待运算符号
{
    int left;int dbg;
    if(error!=0)
        return (0);
    
    if(symtype!=num)
    {
        error = 2;
        return (0);
    }
    left = value;
    getsym();
    if(symtype == mult)
    {
        getsym();
        if(type == mult)
            return (getterm(val * left, mult));
        if(type == div)
            return (getterm(val / left, mult));
    }
    if(symtype == div)
    {
        getsym();
        if(type == mult)
            return (getterm(val * left, div));
        if(type == div)
            return (getterm(val / left, div));
    }
    if(type == mult)
        return (val * left);
    if(type == div)
        return (val / left);
}

int getexpr(int val, int type)//读入一个表达式，传入之前的结果和待运算符号
{
    int left;
    if(error!=0)
        return (0);
    
    left = getterm(1, mult);
    if(symtype == add)
    {
        getsym();
        if(type == add)
            return (getexpr(val + left, add));
        if(type == sub)
            return (getexpr(val - left, add));
    }
    if(symtype == sub)
    {
        getsym();
        if(type == add)
            return (getexpr(val + left, sub));
        if(type == sub)
            return (getexpr(val - left, sub));
    }
    if(type == add)
        return (val + left);
    if(type == sub)
        return (val - left);
}

void calc()
{
    int result;
    
    while(1)
    {
        error=0;
        scanf(c);//预读一个字符
        getsym();//预读一个符号
        result = getexpr(0, add);
        if(symtype!=eql)
            error=3;
        if(error==0)
        {
            printf(result);
            printf(newln);
        }
        if(error!=0)
        {
            printf("ERROR:", error);
            printf(newln);
            while(c!=10)// '\n'
                scanf(c);
        }
    }
}

void main()
{
    newln = 10;
    calc();
}