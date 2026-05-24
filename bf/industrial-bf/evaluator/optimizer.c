// bfm - Converts BrainF to BrainFMacros

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct loop_data {
        uint32_t sp;
        uint64_t stack[0x1000];
};

#define LD_PUSH(ld, i) \
        if (ld->sp == 0xfff) { \
                printf("error: stack overflow\n"); \
                return 1; \
        } \
        ld->stack[ld->sp] = i; \
        ld->sp++

#define LD_POP(ld, i) \
        if (ld->sp == 0) { \
                printf("error: stack underflow\n"); \
                return 1; \
        } \
        ld->sp--; \
        i = ld->stack[ld->sp];

#define READ_CHAR(ptr, stream) { \
        if (fread(ptr, 1, 1, stream) != 1) { \
                if (feof(stream)) { \
                        *ptr = EOF; \
                } else { \
                        perror("reading file"); \
                        return 1; \
                } \
        } \
}

#define READ_CHAR_NOEOF(ptr, stream) { \
        READ_CHAR(ptr, stream); \
        if (*(ptr) == EOF) { \
                printf("error: unexpected EOF\n"); \
                return 1; \
        } \
}

#define PEEK_CHAR(ptr, stream) { \
        size_t pos = ftell(stream); \
        READ_CHAR(ptr, stream); \
        fseek(stream, pos, SEEK_SET); \
}

uint8_t proc_rol_inst(FILE *program_in, struct vector *program_out, struct loop_data *ld) {
        character inst;
        READ_CHAR_NOEOF(&inst, program_in);
        character cur;
        uint32_t count = 1;

#ifdef DEBUGGER
#define is_ignored(a) (is_whitespace(a) && a != '\n')
#else
#define is_ignored is_comment
#endif

        while (1) {
                PEEK_CHAR(&cur, program_in);

                if (cur == EOF)
                        break; // If reached EOF, exit

                if (cur != inst
                 && !is_ignored(cur)) 
                        break;

                if (count >= 256)
                        break;

                READ_CHAR(&cur, program_in);

                if (!is_ignored(cur))
                        count++;
        }
        vector_push(program_out, inst);
        vector_push(program_out, (uint8_t)count-1);
        return 0;
}

uint8_t proc_unrol_inst(FILE *program_in, struct vector *program_out, struct loop_data *ld) {
        character chr;
        READ_CHAR_NOEOF(&chr, program_in);
        vector_push(program_out, chr);
        return 0;
}

uint8_t proc_open_loop(FILE *program_in, struct vector *program_out, struct loop_data *ld) {
        character chr;
        READ_CHAR_NOEOF(&chr, program_in);

        LD_PUSH(ld, program_out->length);
        vector_push_ex(
                program_out,
                uint64_t,
                0xaaaaaaaaaaaaaaaa
        ); // Mock instruction
        return 0;
}

uint8_t proc_close_loop(FILE *program_in, struct vector *program_out, struct loop_data *ld) {
        character chr;
        READ_CHAR_NOEOF(&chr, program_in);

        uint64_t start_ind;
        if (program_out->length & 0xff00000000000000) {
                printf("error: index overflow\n");
                return 1;
        }

        LD_POP(ld, start_ind);

        write_long(
                program_out->ptr + start_ind,
                program_out->length | (((int64_t)'[') << (8*7))
        );

        vector_push_ex(
                program_out,
                uint64_t,
                start_ind | (((int64_t)']') << (8*7))
        );
        return 0;
}

uint8_t proc_assert(FILE *program_in, struct vector *program_out, struct loop_data *ld) {
        printf("not implemented: proc_assert\n");
        return 1;
        /*
        uint8_t inst = program_in[*ind];
        int8_t digit;
        (*ind)++;
        uint64_t val = parse_number(program_in, ind);
        if (val&0xff00000000000000) {
                printf("error: `%c` assert value overflow: %lx\n", inst, val);
                return 1;
        }

        vector_push_ex(
                program_out,
                uint64_t,
                val | (((int64_t)inst) << (8*7))
        );
        return 0;
        */
}

uint8_t proc_zero(FILE *program_in, struct vector *program_out, struct loop_data *ld) {
        character chr;
        READ_CHAR_NOEOF(&chr, program_in);

        READ_CHAR_NOEOF(&chr, program_in);
        if (
                chr != '-' &&
                chr != '+'
        ) return -1;

        READ_CHAR_NOEOF(&chr, program_in);
        if (chr != ']') return -1;

        vector_push(program_out, '0');
        return 0;
}

uint8_t proc_move(FILE *program_in, struct vector *program_out, struct loop_data *ld) {
        character chr;
        READ_CHAR_NOEOF(&chr, program_in);

        struct vector offset_keys;   // short (signed)
        struct vector offset_values; // char (signed)

        vector_init(&offset_keys, 0);
        vector_init(&offset_values, 0);

        vector_push_ex(&offset_keys, int16_t, 0);
        vector_push(&offset_values, 0);

        int16_t offset = 0;

        while (1) {
                READ_CHAR_NOEOF(&chr, program_in);
                if (chr == ']') break;

                int8_t change;
                int8_t key_found;
                uint64_t key_ind;
                switch (chr) {
                        case '<': offset--; break;
                        case '>': offset++; break;
                        case '+':
                                change = 1;
                                goto write_change;
                        case '-':
                                change = -1;
                                goto write_change;
                        write_change:
                                key_found = 0;

                                for (uint64_t i = 0; i < offset_keys.length/2; i++) {
                                        if (vector_read_ex(&offset_keys, int16_t, i) == offset) {
                                                key_found = 1;
                                                key_ind = i;
                                                break;
                                        }
                                }
                                if (!key_found) {
                                        vector_push_ex(&offset_keys, int16_t, offset);
                                        vector_push_ex(&offset_values, int8_t, change);
                                } else {
                                        change += vector_read_ex(&offset_values, int8_t, key_ind);
                                        vector_write_ex(&offset_values, int8_t, key_ind, change);
                                }

                                break;
                        case '[':
                        case ']':
                        case ',':
                        case '.':
                                return -1;
                }
        }

        if (offset != 0) /* unbalanced loop */
                return -1;


        /* output the instructions */

        if ((int8_t)offset_values.ptr[0] != -1) {
                return -1;
        }

        for (uint32_t i = 1; i < offset_values.length; i++) {
                vector_push(program_out, '^');
                vector_push_ex(program_out, int16_t, vector_read_ex(&offset_keys, int16_t, i));
                vector_push(program_out, offset_values.ptr[i]);
        } 
        vector_push(program_out, '0');

        return 0;
}

/* return values: -1 -- EOF
 *                 0 -- success
 *             other -- error */
uint8_t process_instruction(FILE *program_in, struct vector *program_out, struct loop_data *ld) {

/* fn return values: -1 -- no match occured
 *                    0 -- success
 *                other -- error */
#define CALL_PROC(fn) { \
        size_t pos = ftell(program_in); \
        int8_t out = fn(program_in, program_out, ld); \
        if (out != -1) { \
                return out; \
        } else { \
                fseek(program_in, pos, SEEK_SET); \
        } \
}

        character chr;
        uint8_t rout = fread(&chr, 1, 1, program_in);
        if (feof(program_in)) {
                /* EOF */
                return -1;
        }
        if (rout != 1) {
                perror("reading file");
                return 1;
        }
        fseek(program_in, -1, SEEK_CUR);

#ifdef DEBUGGER
if (option_d)
        sourcemap_process(
                chr,
                program_out->length
        );
#endif
        switch (chr) {
                case '+': case '-': case '>': case '<':
                        CALL_PROC(proc_rol_inst);

                case '.': case ',':
                        CALL_PROC(proc_unrol_inst);
#ifdef DEBUGGER
                case '#':
                        if (!option_d) goto ignore;
                        CALL_PROC(proc_unrol_inst);
#endif
                case '[':
#ifndef DISABLE_ROLLING
                        CALL_PROC(proc_zero);
                        CALL_PROC(proc_move);
#endif
                        CALL_PROC(proc_open_loop);
                case ']':
                        CALL_PROC(proc_close_loop);
#ifdef ASSERTS
                case '@':
                case '!':
                        if (!option_a) goto ignore;
                        CALL_PROC(proc_assert);
#endif
ignore:
                default:
                        // Comment
                        fread(&chr, 1, 1, program_in);
                        return 0;
        }
}

uint8_t *optimize(FILE *program_in) {
        struct vector program_out = vector_create(0);

        uint64_t ind = 0;
        character last_char = 0;
        character cur_char;
        int32_t count = -1;

        // Loop optimization
        struct loop_data ld;
        ld.sp = 0;

#ifdef DEBUGGER
if (option_d) {
        printf("constructing program...\n");
}
#endif
        while (1) {
                int8_t out = process_instruction(program_in, &program_out, &ld);
                if (out == -1) {
                        break;
                }
                if (out) {
                        printf("unable to process file: %d\n", out);
                        exit(out);
                }
        }
        if (ld.sp) {
                printf("error: nonempty stack");
                exit(1);
        }
#ifdef DEBUGGER
if (option_d) {
        printf("done\n");
        sourcemap_end(program_out.length-1);
}
#endif
        for (int32_t i = 0; i < 8; i++) {
                vector_push(&program_out, 0);
        }
        return vector_unwrap(&program_out);
}
