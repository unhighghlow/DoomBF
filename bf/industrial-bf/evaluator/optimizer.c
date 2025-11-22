// bfm - Converts BrainF to BrainFMacros

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct loop_data {
        unsigned char sp;
        unsigned long stack[256];
};

#define LD_PUSH(ld, i) \
        if (ld->sp == 255) { \
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

char proc_rol_inst(char program_in[], unsigned long *ind, struct vector *program_out, struct loop_data *ld) {
        char inst = program_in[*ind];
        unsigned ind1 = *ind;
        char cur;
        unsigned int count = 1;

#ifdef DEBUGGER
#define is_ignored is_whitespace
#else
#define is_ignored is_comment
#endif

        while (1) {
                (*ind)++;
                cur = program_in[*ind];

                if (!cur)
                        break; // If reached EOF, exit

                if (cur != inst
                 && !is_ignored(cur)) 
                        break;

                if (count >= 256)
                        break;

                if (!is_ignored(cur))
                        count++;
        }
        vector_push(program_out, inst);
        vector_push(program_out, (char)count-1);
        return 0;
}

char proc_unrol_inst(char program_in[], unsigned long *ind, struct vector *program_out, struct loop_data *ld) {
        vector_push(program_out, program_in[*ind]);
        (*ind)++;
        return 0;
}

char proc_open_loop(char program_in[], unsigned long *ind, struct vector *program_out, struct loop_data *ld) {
        LD_PUSH(ld, program_out->length);
        vector_push_long(
                program_out,
                0xaaaaaaaaaaaaaaaa
        ); // Mock instruction
        (*ind)++;
        return 0;
}

char proc_close_loop(char program_in[], unsigned long *ind, struct vector *program_out, struct loop_data *ld) {
        unsigned long start_ind;
        if (*ind & 0xff000000) {
                printf("error: index overflow\n");
                return 1;
        }

        LD_POP(ld, start_ind);

        write_long(
                program_out->ptr + start_ind,
                program_out->length | (((long)'[') << (8*7))
        );

        vector_push_long(
                program_out,
                start_ind | (((long)']') << (8*7))
        );
        (*ind)++;
        return 0;
}

char proc_assert(char program_in[], unsigned long *ind, struct vector *program_out, struct loop_data *ld) {
        char inst = program_in[*ind];
        signed char digit;
        (*ind)++;
        unsigned long val = parse_number(program_in, ind);
        if (val&0xff00000000000000) {
                printf("error: `%c` assert value overflow: %lx\n", inst, val);
        }

        vector_push_long(
                program_out,
                val | (((long)inst) << (8*7))
        );
        return 0;
}

char process_instruction(char program_in[], unsigned long *ind, struct vector *program_out, struct loop_data *ld) {
#ifdef DEBUGGER
        sourcemap_process(
                program_in[*ind],
                program_out->length
        );
#endif

        switch (program_in[*ind]) {
                case '+':
                case '-':
                case '>':
                case '<':
                        return
                        proc_rol_inst(program_in, ind, program_out, ld);
                case '.':
                case ',':
#ifdef DEBUGGER
                case '#':
#endif
                        return
                        proc_unrol_inst(program_in, ind, program_out, ld);
                case '[':
                        return
                        proc_open_loop(program_in, ind, program_out, ld);
                case ']':
                        return
                        proc_close_loop(program_in, ind, program_out, ld);
#ifdef ASSERTS
                case '@':
                case '!':
                        return
                        proc_assert(program_in, ind, program_out, ld);
#endif
                default:
                        // Comment
                        (*ind)++;
                        return 0;
        }
}

char *optimize(char program_in[]) {
        struct vector program_out = vector_create(0);

        unsigned long ind = 0;
        char last_char = 0;
        char cur_char;
        int count = -1;

        // Loop optimization
        struct loop_data ld;
        ld.sp = 0;

#ifdef DEBUGGER
        printf("constructing program...\n");
#endif
        while (program_in[ind]) {
                process_instruction(program_in, &ind, &program_out, &ld);
        }
        if (ld.sp) {
                printf("error: nonempty stack");
                exit(1);
        }
#ifdef DEBUGGER
        printf("done\n");
        sourcemap_end(program_out.length-1);
#endif
        for (int i = 0; i < 8; i++) {
                vector_push(&program_out, 0);
        }
/*
        for (unsigned long i = 0; i < program_out.length; i++){
                printf("%lx: %x\n", i, *(program_out.ptr + i));
        }
*/
        return vector_unwrap(&program_out);
}
