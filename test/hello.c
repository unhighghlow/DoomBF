#include <stdio.h>

#ifdef _BF
int printf(const char *format, ...)
{
    return 0;
}

void _start()
{
    main();
}   
#endif

int main()
{
printf("\nHello World!");
return 0;
}