//syn.c
//语法测试：完成其他的一些语法语义结构测试
//本程序能够正常执行到底，输出之前输入的数即正确

//测试要点：
//常量变量定义和使用，函数定义，以及其他未覆盖到的一些文法

const char chreqv = 'e' ;
const char chreqv2 = 'e', ch = '4' ;
const char chradd = 'a', chrsub = 's', chrmult = 'm', chrdiv = 'd';
const int a = +23 ;
const int c = +23 , d=-23;
const int num = 0, add = 1, c = -2, mult = 3, div = 4, eql = 5, hehe=+0;
const char chreqvf = '1', ch3 = '4' ;
const int pi=3;

int k ,l,mm[4], n,o;
char ca[23],b,e,f,g;
int ka ,l3,m4[4], n5,o0;
char h[6], i[-1];
int p,q;
char r;
int r[123];
char c_[ 23],_b[5],e__,__f [3 ],g___[0];

int the(int a, int b, int c){ int the; return 0; }
void none() {}
char cc(int b, char e, int f,int g){}
int isNum(char c){m[1+2]=0;}
void getsym(int i){}
int getterm(int val, int type){}
char getexpr(char val, int type){}
void func1(){}
void scope(int c){}
void func3(char d){}
void func4(int a, char e, int c){}
void func6(char a, int e){}
int one(){return (1);}

void scope()
{
    const char chradd = 'a', chrsub = 's', chrmult = 'm', chrdiv = 'd';
    const char chreqv = 'e' ;
    const char chreqv2333333 = 'e', ch = '4' ;
    const int num = 0, add = 1, sub = -2, mult = 3, div = 4, eql = 5, hehe= 0;
    const int a = +23 ;
    const int c = +23 , d=-23;
    const char chreqvf = '1', ch3 = '4' ;
    char z[23],b,e,f,g;
    int k ,l,m[4], n,o;
    char h[6], i[5];
    int p,q;
    char j;
    int r[123];

    char y[ 23],w[5],ee,ff [3 ],gg[1];
}

void main()
{
    const char c = 'c';
    int _a[3],b,d,e0;
    
    if(2333>9987)
        return;
    if('a'>='b'+one())
        return;
    if(1==+2)
    {
        b=1;
        return;
    }
    if(c<=1)
    {
        
    }
    if(0<-233)
    {
        return;
    }
    
    switch(0)
    {
        case 0: ;
        default: return;
    }
    
    switch(0*2)
    {
        case 0: ;
        case 1: ;
    }
    
    none();
    
    b=0; _a[b]=b*3;
    while(_a[b])
        return;
    
    d = -'a'+'4';
    d = + b + c + _a[b]*b*c;
    _a[1] = - (_a[b]) / 2 + 3 * pi/180 / ( 2 - 1 );
    
    scanf(b); printf(b);
    scanf(b,d); printf(b); printf(d);
    scanf(b,d,e0); printf(b); printf(d); printf(e0);
    
    ;
    
    return;
}
