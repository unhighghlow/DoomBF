#define BREAK_REASON_INSTRUCTION ((char)1)
#define BREAK_REASON_BREAKPOINT ((char)2)

#define TYPOGRAPHIC_CELL_WIDTH ((sizeof (CELL)) * 2)

char debugger_stepper = 0;

void debugger_help() {
        printf(
                "? - display help\n"
                "r - run the program until a breakpoint is reached\n"
                "s - step one instruction forward\n"
                "q - quit\n"
        );
}

void debugger_cmd() {
        char buf[3];

        while (1) {
                printf("$ ");
skip_prompt:
                fgets(buf, 2, stdin);

                switch (buf[0]) {
                        case '\n':
                        case '\r':
                                goto skip_prompt;
                        case '?':
                                debugger_help();
                                break;
                        case 'r':
                                debugger_stepper = 0;
                                goto continue_execution;
                        case 's':
                                debugger_stepper = 1;
                                goto continue_execution;
                        case 'q':
                                exit(0);
                        default:
                                printf("'?' for help\n");
                                break;
                }
        }
continue_execution:
        return;
}

void debugger_init() {
        debugger_cmd();
}

unsigned char debugger_print_instruction(char inst[]) {
        char cmd = inst[0];
        char arg = inst[1];
        unsigned long full = ntohll(*(unsigned long*)inst);

        switch (cmd) {
                case '+':
                        printf("+ % 3u\n", (unsigned char)arg + 1);
			return 2;
                case '-':
                        printf("- % 3u\n", (unsigned char)arg + 1);
			return 2;
                case '>':
                        printf("> % 3u\n", (unsigned char)arg + 1);
			return 2;
                case '<':
                        printf("< % 3u\n", (unsigned char)arg + 1);
			return 2;
                case '[':
                        printf("[ %lx\n", (full&0x00ffffffffffffff)+8);
			return 8;
                case ']':
                        printf("] %lx\n", (full&0x00ffffffffffffff)+8);
			return 8;
                case '.':
                        printf(".\n");
			return 1;
                case ',':
                        printf(",\n");
			return 1;
                case '#':
                        printf("#\n");
			return 1;
		default:
			printf("??? %x %x\n", (unsigned char)cmd, (unsigned char)arg);
			break;
        }
}

void debugger_call(char reason, CELL tape[], char program[], unsigned long dp, unsigned long pc) {
        if (reason == BREAK_REASON_INSTRUCTION && !debugger_stepper) return;

        printf("program: 0x%lx\n", pc);
	unsigned long offset = 0;
        for (int i = 0; i < 5; i++) {
		if (!i) {
			printf("> ");
		} else {
			printf("  ");
		}
		printf("%04lx:\t", pc+offset);
		offset += debugger_print_instruction(&program[pc+offset]);
		if (!program[pc+offset]) break;
        }

        printf("tape: 0x%lx\n", dp);
        for (int offset = -3; offset < 4; offset++) {
                printf(CELL_FORMAT_STRING, tape[(dp+offset)%(PAGE_SIZE*4)]);
                printf(" ");
        }
        printf("\n");
        for (int i = 0; i < (TYPOGRAPHIC_CELL_WIDTH+1) * 4 - 2; i++) {
                printf(" ");
        }
        printf("^\n");

        debugger_cmd();
}
