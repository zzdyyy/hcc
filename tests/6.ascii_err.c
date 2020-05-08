//ascii.c
//功能性测试：ascii码转换器，输出一张ascii表，根据输入的ascii码或字符进行转换

//测试要点：
//类型相关测试（尤其是字符的转换和输出）

char newln;
char a[];

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

char chr()
{
    return ('');
}

void chartest()
{
    const char d='d';
    char e,f;
    printf(chr());
    a[0]='b';
    a[1]='c';
    printf(a[0],a[1]);
    printf(d);
    e='ba'+3;
    f=102;
    printf(e f);
}

void main()
{
    int _ThisIsALongVarToRepresentTestcode;
    newln = 10;
    printf("Hello, test!",newln);
    printf("1-ascii",newln);
    printf("2-chartest,newln);
    
    scanf(_ThisIsALongVarToRepresentTestcode);
    switch(_ThisIsALongVarToRepresentTestcode)
    {
        case 2: chartest();
        case 1: ascii();
    }
}