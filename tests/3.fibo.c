//fibo.c
//功能性测试：本程序输出斐波那契数列前20项，用于测试对数组和递归的支持

//测试要点：
//全局和局部数组的使用，函数递归，结构测试

char newln;
int fibo[20];

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

void main()
{
    newln = 10;
    fibo1();
    fibo2();
    fibo3(0, 1, 2);
    printf("1 1");
    printf(newln);
}