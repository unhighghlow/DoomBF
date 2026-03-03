int _start() {
        volatile int a = 2;
        volatile int out = 3;
        asm(
                "!!+++++++++++++++++++++++++++++++++.[-]test!!"
                : "r3" (out)
                : "r2" (a)
        );
        a = out;
        return 1;
}
