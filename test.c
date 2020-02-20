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

int allocp4(int ***p, int *a, int *b, int *c, int *d)
{
    *p = malloc(sizeof(int*) * 4);
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
    
//     int ** x;
//     allocp4(&x, p, p + 1, p + 2, p + 3);
//     int **y;
//     y = x + (q - p);
//     printf("%d\n", **y);

//     return 0;
// }