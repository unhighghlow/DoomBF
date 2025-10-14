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

char* read_file(char* filename, unsigned long *program_length);
int find_loops(short *program, unsigned long *loops);
void evaluate(short *program, CELL *tape, unsigned long *loops);

int main(int argc, char *argv[]) {
	char *filename;

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
        short *program = optimize(program_raw);

	unsigned long *loops = safe_malloc(program_length * (sizeof (unsigned long)));
	if (find_loops(program, loops)) {
                return 1;
        }

	CELL *tape = safe_malloc(HOT_TAPE * (sizeof (CELL)));
	memset(tape, 0, HOT_TAPE * (sizeof (CELL)));

	load_page(tape, -1);
	load_page(tape, 0);
	load_page(tape, 1);

        evaluate(program, tape, loops);
}

int find_loops(short program[], unsigned long loops[]) {
	unsigned long ind = -1;
	char sp = 0;
	unsigned long stack[256];
	char inst;

	while ((inst = program[++ind])) {
		if (inst == '[') {
			stack[sp++] = ind;
		}
		else if (inst == ']') {
                        if (sp == 0) {
                                puts("loop stack underflow\n");
                                return 1;
                        }
			sp--;
			loops[ind] = stack[sp];
			loops[stack[sp]] = ind;
		}
	}
        if (sp > 0) {
                puts("loop stack overflow\n");
                return 1;
        }
        return 0;
}

union command {
        struct {
                char cmd;
                char arg;
        } d;
        short raw;
};

const void* jumptable[0x100];

void evaluate(short program[], CELL tape[], unsigned long loops[]) {
#ifdef DEBUGGER
        debugger_init();
#endif
	register unsigned long pc = -1;
	register unsigned long dp = 0;
	register union command inst;
	register char last_page = 0;

	for (int i = 0; i < 0x100; i++) {
		jumptable[i] = &&ignore;
	}

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

#ifdef DEBUGGER

#define NEXT \
	inst.raw = program[++pc]; \
        if (inst.d.cmd != '#') \
                debugger_call(BREAK_REASON_INSTRUCTION, tape, program, dp, pc); \
	goto *(jumptable[inst.d.cmd]);

#else

#define NEXT \
	inst.raw = program[++pc]; \
	goto *(jumptable[inst.d.cmd]);

#endif

ignore:
	NEXT

plus:
	tape[dp%HOT_TAPE]+=(short)inst.d.arg + 1;
	NEXT

minus:
	tape[dp%HOT_TAPE]-=(short)inst.d.arg + 1;
	NEXT


right:
	dp+=((unsigned long)inst.d.arg) + 1;
	CHECK_PAGE_TRANSITION(tape, 1, dp, last_page);
	NEXT

left:
	dp-=(short)inst.d.arg + 1;
	CHECK_PAGE_TRANSITION(tape, -1, dp, last_page);
	NEXT

output:
	putchar(tape[dp%HOT_TAPE]);
	NEXT

loopstart:
	if (!tape[dp%HOT_TAPE])
		pc=loops[pc];
	NEXT

loopend:
	if (tape[dp%HOT_TAPE])
		pc=loops[pc];
	NEXT

#ifdef DEBUGGER
breakinst:
        debugger_call(BREAK_REASON_BREAKPOINT, tape, program, dp, pc);
        NEXT
#endif

exit:
	return;
}

char* read_file(char* filename, unsigned long *program_length) {
	FILE *f = fopen(filename, "rb");
        if (!f) {
                printf("cannot open file\n");
                abort();
        }
	fseek(f, 0, SEEK_END);
	unsigned long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *string = safe_malloc(fsize + 1);
	fread(string, fsize, 1, f);
	fclose(f);

	string[fsize] = 0;
	*program_length = fsize;
	return string;
}
