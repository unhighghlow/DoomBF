// ibf - Executes BrainF and BrainFMacros programs

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <stdint.h>

/* Config */
/*  Invisible configuration */
#define PAGE_SIZE 256 /* 1. For performance reasons it's faster if the page size is a power of 2
                            2. If USE_MACROS is enabled, the page size must be bigger than of equal to 256 */

/*  Compliant configuration */
#define CELL uint8_t

/*  Non-compliant configuration */
// #define USE_MACROS

/* /Config */

#ifdef USE_MACROS
#define P_CELL short
#else
#define P_CELL char 
#endif

#define HOT_TAPE (PAGE_SIZE * 4)

char* read_file(char* filename, unsigned long *program_length);
int find_loops(P_CELL *program, unsigned long *loops);
void evaluate(P_CELL *program, CELL *tape, unsigned long *loops);
void load_page(CELL *tape, unsigned long page_uid);
void store_page(CELL *tape, unsigned long page_uid);

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
	P_CELL *program = (P_CELL*) read_file(filename, &program_length);

	unsigned long *loops = malloc(program_length * (sizeof (unsigned long)));
	if (find_loops(program, loops)) {
                return 1;
        }

	CELL *tape = malloc(HOT_TAPE * (sizeof (CELL)));
	memset(tape, 0, HOT_TAPE * (sizeof (CELL)));

	load_page(tape, -1);
	load_page(tape, 0);
	load_page(tape, 1);

        evaluate(program, tape, loops);
}

int find_loops(P_CELL program[], unsigned long loops[]) {
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

unsigned long get_page_uid(unsigned long dp) {
	return dp / PAGE_SIZE;
}

char get_page_n(unsigned long dp) {
	return dp / PAGE_SIZE % 4;
}

#ifdef USE_MACROS
void check_page_transition(CELL tape[], signed char expected_direction, unsigned long dp, char *last_page) {
	char cur_page = get_page_n(dp);
	if ((*last_page) == cur_page) {
		return;
	}
	*last_page = cur_page;
#else
void check_page_transition(CELL tape[], signed char expected_direction, unsigned long dp) {
	if ((expected_direction == 1) && (dp % PAGE_SIZE != 0)) {
		return;
	}

	if ((expected_direction == -1) && ((dp+1) % PAGE_SIZE != 0)) {
		return;
	}
#endif

        // 0      1      2      3  
        // store keep current load
        // current page ^
        // expected direction -->
        
        unsigned long current_page_uid = get_page_uid(dp);
        store_page(tape, current_page_uid - (expected_direction * 2));
        load_page(tape, current_page_uid + expected_direction);
}

#ifdef USE_MACROS
union command {
        struct {
                char cmd;
                char arg;
        } d;
        short raw;
};
#endif

const void* jumptable[0x100];

void evaluate(P_CELL program[], CELL tape[], unsigned long loops[]) {
	register unsigned long pc = -1;
	register unsigned long dp = 0;
#ifdef USE_MACROS
	char last_page = 0;
#endif

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

#ifdef USE_MACROS

	register union command inst;
#define NEXT \
	inst.raw = program[++pc]; \
	goto *(jumptable[inst.d.cmd]);

#else

#define NEXT \
	goto *(jumptable[(unsigned char)program[++pc]]);
#endif

ignore:
	NEXT

plus:
#ifdef USE_MACROS
	tape[dp%HOT_TAPE]+=(short)inst.d.arg + 1;
#else
	tape[dp%HOT_TAPE]++;
#endif
	NEXT

minus:
#ifdef USE_MACROS
	tape[dp%HOT_TAPE]-=(short)inst.d.arg + 1;
#else
	tape[dp%HOT_TAPE]--;
#endif
	NEXT


right:
#ifdef USE_MACROS
	dp+=((unsigned long)inst.d.arg) + 1;
	check_page_transition(tape, 1, dp, &last_page);
#else
	dp++;
	check_page_transition(tape, 1, dp);
#endif
	NEXT

left:
#ifdef USE_MACROS
	dp-=(short)inst.d.arg + 1;
	check_page_transition(tape, -1, dp, &last_page);
#else
	dp--;
	check_page_transition(tape, -1, dp);
#endif
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

exit:
	return;
}

void store_page(CELL tape[], unsigned long page_uid) {
        char page_n = page_uid % 4;
        CELL *target = &tape[PAGE_SIZE*page_n];

        char filename[17];
        sprintf(filename, "%016lx", page_uid);
#ifdef DEBUG
        printf("storing page: 0x%s... ", filename);
#endif

	FILE *f = fopen(filename, "wb");
        if (!f) {
                printf("page store failed! (open)\n");
                exit(1);
        }

	if (fwrite(target, 1, PAGE_SIZE, f) < PAGE_SIZE) {
                printf("page store failed! (write)\n");
                exit(1);
        }
#ifdef DEBUG
        printf("ok\n");
#endif
        fclose(f);
}

void load_page(CELL tape[], unsigned long page_uid) {
        char page_n = page_uid % 4;
        CELL *target = &tape[PAGE_SIZE*page_n];

        char filename[17];
        sprintf(filename, "%016lx", page_uid);
#ifdef DEBUG
        printf("loading page: 0x%s... ", filename);
#endif

	FILE *f = fopen(filename, "rb");
        if (!f) {
                memset(target, 0, PAGE_SIZE);
#ifdef DEBUG
                printf("empty\n");
#endif
                return;
        }

	if (fread(target, 1, PAGE_SIZE, f) < PAGE_SIZE) {
                printf("page load failed!\n");
                exit(1);
        }
#ifdef DEBUG
        printf("ok\n");
#endif
        fclose(f);
}

char* read_file(char* filename, unsigned long *program_length) {
	FILE *f = fopen(filename, "rb");
	fseek(f, 0, SEEK_END);
	unsigned long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *string = malloc(fsize + 1);
	fread(string, fsize, 1, f);
	fclose(f);

	string[fsize] = 0;
	*program_length = fsize;
	return string;
}
