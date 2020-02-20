#include <stdio.h>
#include <stdlib.h>
int foo()
{
    printf("OK\n");
    return 0;
}

int bar(int a, int b)
{
    printf("%d\n", a + b);
    return 0;
}

int four()
{
    return 4;
}

int five()
{
    return 5;
}

int alloc4(int **p, int a, int b, int c, int d)
{
    *p = malloc(sizeof(int) * 4);
    **p = a;
    *(*p + 1) = b;
    *(*p + 2) = c;
    *(*p + 3) = d;
}

// int main()
// {
//     int *p;
//     alloc4(&p, 1, 2, 4, 8);
//     int *q;
//     q = p + 2;
//     printf("%d\n", *q);
//     q = p + 3;
//     printf("%d\n", *q);
//     return 0;
// }