#ifndef __brainfuck

#!/usr/local/bin/tcc -run
#include <tcclib.h>

#else

int main();
void _start() { main(); }
void putc(char chr) {
    __asm__ (
        ">.<"
      :
      : "r0" (chr)
    );
}

#endif

int main()
{
    //printf("Hello World\n");
    return 0;
}
