//功能性测试：本程序分为3个测试。第一个是一个简单的表达式计算器，可以计算整数之间的加减乘除（无括号）
//的四则混合运算。输入一个算式，以e（因为程序中无法使用'='）结尾回车即可
//第2个是ascii码转换器，输出一张ascii表，根据输入的ascii码或字符进行转换
//第3个是输出斐波那契数列前30项
const char chradd = '+', chrsub = '-', chrmult = '*', chrdiv = '/';
const char chreqv = 'e' ;
const int num = 0, add = 1, sub = 2, mult = 3, div = 4, eql = 5;    //符号类型
char newln;
int fibo[20];

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
    scanf(c);//跳过测试码后面的换行符
    
    while(1)
    {
        error=0;
        scanf(c);//预读一个字符
        getsym();//预读一个符号
        result = getexpr(0, add);
        if(symtype!=eql)
            error=3;
        if(error==0)
            printf(result,newln);
        if(error!=0)
        {
            printf("ERROR:", error,newln);
            while(c!=10)// '\n'
                scanf(c);
        }
    }
}

void ascii()
{
    int i;
    char c;
    c=0;
    i=32;
    while(i<128)//print ascii table
    {
        c=i;
        printf(i," ",c,"    ");
        if(i-i/16*16 >= (15))
            printf(newln);
        i=i+1;
    }
    printf("input '0 number' or '1 char' to convert:",newln);
    while(1+2* 3/4 -(5*6/7) == + (-2))    //while(true)
    {
        scanf(i);
        switch(i)
        {
            case 0: if(2) { scanf(i); c=i; printf(c, newln); }
            case 1: if(1) { scanf(c); scanf(c); i=c; printf(i, newln); scanf(c); }
            default: switch(i){
                case 2: {printf("2 is not valid", newln);}
                case 3: {printf("3 is not valid", newln);}
                default: {printf("not valid", newln);}
            }
        }
    }
    printf("ERROR!");
}

void fibo1()
{
    int i;
    int fibo[20];
    fibo[0]=1;
    fibo[1]=1;
    i=2;
    while(i<20){
        fibo[i]=fibo[i-1]+fibo[i-2];
        i=i+1;
    }
    i=0;
    while(i<20){
        printf(fibo[i]);
        printf(" ");
        i=i+1;
    }
    printf(newln);
}

void fibo2()
{
    int i;

    i=0;
    while(i<20){
        printf(fibo[i]);
        printf(" ");
        i=i+1;
    }
    printf(newln);
    fibo[0]=1;
    fibo[1]=1;
    i=2;
    while(i<20){
        fibo[i]=fibo[i-1]+fibo[i-2];
        i=i+1;
    }
    i=0;
    while(i<20){
        printf(fibo[i]);
        printf(" ");
        i=i+1;
    }
    printf(newln);
}

int fibo3(int a, int b, int cnt)
{
    if(cnt>=20)
        return (a+b);
    printf(fibo3(b, a+b, cnt+1));
    printf(" ");
    return (a+b);
}

void fibof()
{
    fibo1();
    fibo2();
    fibo3(0, 1, 2);
    printf("1 1");
    printf(newln);
}

void main()
{
    int _ThisIsALongVarToRepresentTestcode;
    newln = 10;
    printf("Hello, test!",newln);
    printf("0-calculator",newln);
    printf("1-ascii",newln);
    printf("2-fibonacci",newln);
    //printf();
    scanf(_ThisIsALongVarToRepresentTestcode);
    switch(_ThisIsALongVarToRepresentTestcode)
    {
        case 0: calc();
        case 1: ascii();
        case 2: fibof();
        default: printf("ERROR");
    }
}