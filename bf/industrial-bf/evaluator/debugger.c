#define BREAK_REASON_INSTRUCTION ((char)1)
#define BREAK_REASON_BREAKPOINT ((char)2)

#define TYPOGRAPHIC_CELL_WIDTH ((sizeof (CELL)) * 2)

enum dbg_state {
        DBG_STEP,
        DBG_RUN,
        DBG_EXIT_SEARCH,
        DBG_EXIT_RUN,
        DBG_SM_NEXT
};

enum dbg_state debugger_state = DBG_STEP;
unsigned long exit_search_depth;
unsigned long exit_target;
unsigned long sm_next_target;

enum addrmap_mode {
        MODE_HEX,
        MODE_DEC,
        MODE_STRING,
        MODE_BYTES
};

struct addrmap_value {
        char name[65];
        enum addrmap_mode mode;
        unsigned long len;
        unsigned long addr;
};

struct addrmap {
        unsigned long count;
        struct addrmap_value *values;
};

struct addrmap active_addrmap = {0};

void debugger_help() {
        printf(
                "? - display help\n"
                "r - run the program until a breakpoint is reached\n"
                "s - step one instruction forward\n"
                "x - run until the current loop ends\n"
                "n - run until the next sourcemap line\n"
                "a - load an addrmap file\n"
                "q - quit\n"
        );
}

void load_addrmap_line(char line[], struct addrmap_value *out) {
        enum addrmap_mode mode;
        unsigned long addr;
        unsigned long len = 1;

        switch (line[0]) {
                case 'x':
                        mode = MODE_HEX;
                        break;
                case 'd':
                        mode = MODE_DEC;
                        break;
                case 's':
                        mode = MODE_STRING;
                        break;
                case 'a':
                        mode = MODE_BYTES;
                        break;
                default:
                        printf("invalid mode: %c", line[0]);
                        return;
        }

        unsigned long ind = 1;
        addr = parse_number(line, &ind);

        if (line[ind] == '[') {
                ind++;
                len = parse_number(line, &ind);
                ind++;
        }
        ind++;

        strcpy(out->name, &line[ind]);
        out->mode = mode;
        out->len = len;
        out->addr = addr;

        printf("@%lx M: %x L: %lx N: %s\n", addr, out->mode, out->len, out->name);
}

unsigned long count_nonempty_lines(char *addrmap, unsigned long length) {
        char newline = 1;
        unsigned long lines = 0;
        for (unsigned long ind = 0; ind < length; ind++) {
                if (addrmap[ind] == '\n') {
                        if (!newline) {
                                lines++;
                        }
                        newline = 1;
                } else {
                        newline = 0;
                }
        }
        if (!newline)
                lines++;
        return lines;
}

void load_addrmap(char *filename) {
        printf("loading addrmap: %s. ", filename);
        char line[65] = {0};
        unsigned long ind;
        unsigned long lineind = 0;
        unsigned long length;
        unsigned long lines;

        char *addrmap = read_file(filename, &length);
        if (!addrmap) return;

        lines = count_nonempty_lines(addrmap, length);
        printf("%ld lines\n", lines);

        if (active_addrmap.values) {
                free(active_addrmap.values);
                printf("unloaded old attrmap\n");
        }
        active_addrmap.count = 0;
        active_addrmap.values = malloc(sizeof (struct addrmap_value) * lines);

        for (ind = 0; ind < length; ind++) {
                if (addrmap[ind] == '\n') {
load_line:
                        line[lineind] = 0;
                        if (line[0]) {
                                load_addrmap_line(
                                        line,
                                        &active_addrmap.values[
                                            active_addrmap.count
                                        ]
                                );
                                active_addrmap.count++;
                        }
                        lineind = 0;
                        line[0] = 0;
                } else {
                        line[lineind] = addrmap[ind];
                        lineind++;
                }
        }
        if (line[0])
                goto load_line;
}

struct sourcemap_entry *read_sm_entry(unsigned long ind) {
        return (
            (struct sourcemap_entry*)
            ntohll(
                *(unsigned long*)
                &sourcemap.entries.ptr[(ind)*8]
            )
        );
}

unsigned long get_current_sm_ind(unsigned long pc) {
        unsigned long count = sourcemap.entries.length/8;
        unsigned long ind = 0;
        while (1) {
                ind++;
                if (ind >= count)
                        break;
                if ((read_sm_entry(ind)->ind) > pc)
                        break;
        }
        ind--;
        return ind;
}

void debugger_cmd(unsigned long pc) {
        char buf[4];
        char filename[17];
        unsigned long len;

        while (1) {
                printf("$ ");
skip_prompt:
                fgets(buf, 3, stdin);

                switch (buf[0]) {
                        case '\n':
                        case '\r':
                                goto skip_prompt;
                        case '?':
                                debugger_help();
                                break;
                        case 'r':
                                debugger_state = DBG_RUN;
                                goto continue_execution;
                        case 's':
                                debugger_state = DBG_STEP;
                                goto continue_execution;
                        case 'x':
                                exit_search_depth = 0;
                                exit_target = 0;
                                debugger_state = DBG_EXIT_SEARCH;
                                goto continue_execution;
                        case 'n':
                                sm_next_target = get_current_sm_ind(pc)+1;
                                debugger_state = DBG_SM_NEXT;
                                goto continue_execution;
                        case 'a':
                                printf("addrmap: ");
                                fgets(filename, 16, stdin);
                                len = strlen(filename);
                                if (filename[len-1] == 0xa) {
                                        filename[len-1] = 0;
                                }
                                load_addrmap(filename);
                                break;
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
        debugger_cmd(0);
}

void show_char(char chr) {
        if (chr == '"') {
                printf("\\\"");
                return;
        }
        if (chr == '\\') {
                printf("\\\\");
                return;
        }
        if (chr >= ' ' && chr <= '~') {
                printf("%c", chr);
                return;
        }
        printf("\\%o", chr);
}

void show_string(CELL tape[], unsigned long addr, unsigned long length, unsigned long dp) {
        char elipsis = 0;
        char string_open = 0;
        char start = 1;
        char val;
        unsigned long cur_addr;

#define OPEN_STRING { \
        if (!string_open) { \
                if (!start) \
                        printf(" "); \
                printf("\""); \
        } \
        string_open = 1; \
}

#define CLOSE_STRING { \
        if (string_open) { \
                printf("\""); \
                if (val) \
                        printf("!"); \
        } \
        string_open = 0; \
}

#define ELIPSIS { \
        if (!elipsis) { \
                CLOSE_STRING \
                printf("..."); \
        } \
        elipsis = 1; \
}

        for (unsigned long i = 0; i < length; i++) {
                cur_addr = addr + i;
                if (is_addr_loaded(dp, cur_addr)) {
                        val = tape[cur_addr%(PAGE_SIZE*4)];
                        OPEN_STRING
                        if (val)
                                show_char(val);
                        else
                                CLOSE_STRING
                        elipsis = 0;
                } else {
                        ELIPSIS
                        val = 0;
                }
                start = 0;
        }
        CLOSE_STRING
} 

void show_bytes(CELL tape[], unsigned long addr, unsigned long length, unsigned long dp) {
        char comma = 0;
        char elipsis = 0;
        unsigned long addr2;
        for (int i = 0; i < length; i++) {
#define PRINT_COMMA \
                if (comma) \
                        printf(", "); \
                comma = 1;

                addr2 = addr+i;
                if (is_addr_loaded(dp, addr2)) {
                        PRINT_COMMA
                        printf("0x%x", tape[addr2%(PAGE_SIZE*4)]);
                        elipsis = 0;
                } else {
                        if (!elipsis) {
                                PRINT_COMMA
                                printf("...");
                        }
                        elipsis = 1;
                }
        }
}

void debugger_print_source(unsigned long pc) {
        if (!sourcemap.entries.length) return;

        unsigned long count = sourcemap.entries.length/8;
        unsigned long start;
        unsigned long ind = get_current_sm_ind(pc);
        start = ind;

        printf("SOURCE:\n");
        printf("> %s\n", read_sm_entry(ind)->text);
        for (ind++; ind < count && ind < start+4; ind++) {
                printf("  %s\n", read_sm_entry(ind)->text);
        }
}

void debugger_print_addrmap_vals(CELL tape[], unsigned long dp) {
        if (!active_addrmap.values) return;

        struct addrmap_value *cur;
        CELL val;

        printf("VARIABLES:\n");
        for (int ind = 0; ind < active_addrmap.count; ind++) {
                cur = &active_addrmap.values[ind];
                if (dp >= cur->addr && dp < cur->addr+cur->len)
                        printf(">");
                else
                        printf(" ");
                printf("%s: ", cur->name);

#define ELIPSIS_IF_NOT_LOADED \
                if (!is_addr_loaded(dp, cur->addr)) { \
                        printf("...\n"); \
                        continue; \
                }

                val = tape[cur->addr%(PAGE_SIZE*4)];

                switch(cur->mode) {
                        case MODE_HEX:
                                ELIPSIS_IF_NOT_LOADED
                                printf("0x%x", val);
                                break;
                        case MODE_DEC:
                                ELIPSIS_IF_NOT_LOADED
                                printf("%x", val);
                                break;
                        case MODE_BYTES:
                                show_bytes(tape, cur->addr, cur->len, dp);
                                break;
                        case MODE_STRING:
                                show_string(tape, cur->addr, cur->len, dp);
                                break;
                }

                printf("\n");
        }
}

unsigned char debugger_print_instruction(char inst[]) {
        char cmd = inst[0];
        char arg = inst[1];
        unsigned long full = ntohll(*(unsigned long*)inst);

        switch (cmd) {
                case '+':
                case '-':
                case '>':
                case '<':
                        printf("%c %3u\n", cmd, (unsigned char)arg + 1);
                        return 2;
                case '[':
                case ']':
                        printf("%c %lx\n", cmd, (full&0x00ffffffffffffff)+8);
                        return 8;
                case '!':
                case '@':
                        printf("%c %lx\n", cmd, (full&0x00ffffffffffffff));
                        return 8;
                case '.':
                case ',':
                case '#':
                        printf("%c\n", cmd);
                        return 1;
                default:
                        printf("??? %x %x\n", (unsigned char)cmd, (unsigned char)arg);
                        return 2;
        }
}

void debugger_call(char reason, CELL tape[], char program[], unsigned long dp, unsigned long pc) {
        switch (debugger_state) {
                case DBG_RUN:
                        if (reason == BREAK_REASON_INSTRUCTION)
                                return;
                        break;
                case DBG_STEP:
                        break;
                case DBG_EXIT_SEARCH:
                        if (pc < exit_target)
                                return;
                        if (program[pc] == '[') {
                                exit_search_depth++;
                        }
                        if (program[pc] == ']') {
                                exit_target = pc+8;
                                if (!exit_search_depth) {
                                        debugger_state = DBG_EXIT_RUN;
                                }
                                exit_search_depth--;
                        }
                        return;
                case DBG_SM_NEXT:
                        if (get_current_sm_ind(pc) >= sm_next_target)
				break;
			return;
                case DBG_EXIT_RUN:
                        if (pc == exit_target)
                                break;
                        return;
        }

        printf("PROGRAM: 0x%lx\n", pc);
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

        printf("TAPE: 0x%lx\n", dp);
        for (int offset = -3; offset < 4; offset++) {
                printf(CELL_FORMAT_STRING, tape[(dp+offset)%(PAGE_SIZE*4)]);
                printf(" ");
        }

        printf("\n");
        for (int i = 0; i < (TYPOGRAPHIC_CELL_WIDTH+1) * 4 - 2; i++) {
                printf(" ");
        }
        printf("^\n");

        debugger_print_addrmap_vals(tape, dp);
        debugger_print_source(pc);

        debugger_cmd(pc);
}
