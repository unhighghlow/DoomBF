// ibf - Executes BrainF and BrainFMacros programs

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <stdint.h>

#include "util.c"
#include "vector.c"
#include "optimizer.c"
#include "infinite-tape.c"

#ifdef DEBUGGER
#include "debugger.c"
#endif

#include "config.h"

char* read_file(char *filename, unsigned long *program_length);
void evaluate(char *program, CELL *tape);

int main(int argc, char *argv[]) {
        char *filename;
        char addrmap_filename[65];

        if (argc > 2) {
                printf("usage: %s <program>\n", argv[0]);
                return 1;
        } else if (argc == 2) {
                filename = argv[1];
        } else {
                filename = "test.b";
        }
        
        unsigned long program_length;
        char *program_raw = (char*) read_file(filename, &program_length);
        if (!program_raw) exit(1);

        char *program = optimize(program_raw);

        CELL *tape = safe_malloc(HOT_TAPE * (sizeof (CELL)));
        memset(tape, 0, HOT_TAPE * (sizeof (CELL)));

        load_page(tape, PAGE_COUNT);
        load_page(tape, 0);
        load_page(tape, 1);

        strcpy(addrmap_filename, filename);
        strcat(addrmap_filename, ".addr");
#ifdef DEBUGGER
        load_addrmap(addrmap_filename);
#endif

        evaluate(program, tape);
}

union command {
        struct {
                char cmd;
                char arg;
        } d;
        unsigned long raw;
};

const void* jumptable[0x100];

void evaluate(char program[], CELL tape[]) {
#ifdef DEBUGGER
        debugger_init();
#endif
        register unsigned long pc = 0;
        register unsigned long dp = 0;
        register union command inst;
        register char last_page = 0;
#ifdef ASSERTS
        char *assert_name;
        unsigned long assert_expected;
        unsigned long assert_got;
#endif

        jumptable[0] = &&exit;
        jumptable['+'] = &&plus;
        jumptable['-'] = &&minus;
        jumptable['>'] = &&right;
        jumptable['<'] = &&left;
        jumptable['.'] = &&output;
        jumptable['['] = &&loopstart;
        jumptable[']'] = &&loopend;
#ifdef DEBUGGER
        jumptable['#'] = &&breakinst;
#endif
#ifdef ASSERTS
        jumptable['@'] = &&assert_location;
        jumptable['!'] = &&assert_value;
#endif

#ifdef DEBUGGER

#define NEXT \
        inst.raw = *(unsigned long*)(&program[pc]); \
        if (inst.d.cmd != '#') \
                debugger_call(BREAK_REASON_INSTRUCTION, tape, program, dp, pc); \
        goto *(jumptable[inst.d.cmd]);

#else

#define NEXT \
        inst.raw = *(unsigned long*)(&program[pc]); \
        goto *(jumptable[inst.d.cmd]);

#endif

        NEXT

plus:
        tape[dp%HOT_TAPE]+=(unsigned char)inst.d.arg + 1;
        pc+=2;
        NEXT

minus:
        tape[dp%HOT_TAPE]-=(unsigned char)inst.d.arg + 1;
        pc+=2;
        NEXT


right:
        dp+=((unsigned char)inst.d.arg) + 1;
        CHECK_PAGE_TRANSITION(tape, 1, dp, last_page);
        pc+=2;
        NEXT

left:
        dp-=((unsigned char)inst.d.arg) + 1;
        CHECK_PAGE_TRANSITION(tape, -1, dp, last_page);
        pc+=2;
        NEXT

output:
        putchar(tape[dp%HOT_TAPE]);
        pc+=1;
        NEXT

loopstart:
        #define endind ntohll(inst.raw)&0x00ffffffffffffff
        if (!tape[dp%HOT_TAPE])
                pc=endind;
        pc+=8;
        NEXT

loopend:
        #define begind ntohll(inst.raw)&0x00ffffffffffffff
        if (tape[dp%HOT_TAPE])
                pc=begind;
        pc+=8;
        NEXT

#ifdef DEBUGGER
breakinst:
        debugger_call(BREAK_REASON_BREAKPOINT, tape, program, dp, pc);
        pc+=1;
        NEXT
#endif


#ifdef ASSERTS
assert_location:
        assert_name = "location";
        assert_got = dp;
        goto assert_common;

assert_value:
        assert_name = "value";
        assert_got = tape[dp%HOT_TAPE];
        /* fallthrough */

assert_common:
        assert_expected = ntohll(inst.raw)&0x00ffffffffffffff;
        if (assert_expected != assert_got) {
                printf("assertion failed: %s\n", assert_name);
                printf("expected: 0x%lx\n", assert_expected);
                printf("got: 0x%lx\n", assert_got);
                exit(1);
        }
        pc+=8;
        NEXT
#endif

exit:
        return;
}
