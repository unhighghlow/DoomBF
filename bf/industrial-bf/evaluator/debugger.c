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

void debugger_print_instruction(char inst[]) {
        char cmd = inst[0];
        char arg = inst[1];

        switch (cmd) {
                case '+':
                        printf("+ % 3d", (unsigned int)arg + 1);
                        break;
                case '-':
                        printf("- % 3d", (unsigned int)arg + 1);
                        break;
                case '>':
                        printf("> % 3d", (unsigned int)arg + 1);
                        break;
                case '<':
                        printf("< % 3d", (unsigned int)arg + 1);
                        break;
                case '[':
                        printf("[");
                        break;
                case ']':
                        printf("]");
                        break;
                case '.':
                        printf(".");
                        break;
                case ',':
                        printf(",");
                        break;
                case '#':
                        printf("#");
                        break;
        }
        printf("\n");
}

void debugger_call(char reason, CELL tape[], short program[], unsigned long dp, unsigned long pc) {
        if (reason == BREAK_REASON_INSTRUCTION && !debugger_stepper) return;

        printf("program: 0x%x\n", pc);
        for (int offset = -2; offset < 5; offset++) {
                if ((-offset) <= pc || offset >= 0) {
                        if (!offset) {
                                printf("> ");
                        } else {
                                printf("  ");
                        }
                        printf("%04x:\t", pc+offset);
                        debugger_print_instruction((char*)&program[pc+offset]);
                }
        }

        printf("tape: 0x%x\n", dp);
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
