// bfm - Converts BrainF to BrainFMacros

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct instruction {
        char cmd;
        unsigned char arg;
};

union short_to_instruction {
        short r;
        struct instruction i;
};

struct instruction to_instruction_struct(short v) {
        union short_to_instruction v2;
        v2.r = v;
        return v2.i;
}

#define vec_read(vec, i) (to_instruction_struct(*((short*)(vec->ptr+i))))

#define assert_match(param, vec, i, cmdv) \
        if (vec_read(vec, i).param != cmdv) { \
                return 0; \
        }

#define shift(pos) \
        if (pos < 2) return 0; \
        pos-=2

#define MOVE_ORDER_MINUS_PLUS 1
#define MOVE_ORDER_PLUS_MINUS 2
char apply_copying_shorthand(struct vector *prog) {
        long pos = prog->length-2;
        char move_order = 0;
        char move_offset = 0;
        struct instruction inst;

        // ending bracket
        assert_match(cmd, prog, pos, ']');
        shift(pos);


        // [<+>-] move order suppoprt
        inst = vec_read(prog, pos);
        if (inst.cmd == '-' && inst.arg == 0) {
                move_order = MOVE_ORDER_PLUS_MINUS;
                shift(pos);
        } else {
                move_order = MOVE_ORDER_MINUS_PLUS;
        }


        // exit from the move destination
        inst = vec_read(prog, pos);
        if (inst.cmd == '>' && inst.arg < 127) {
                move_offset = -(inst.arg+1);
        } else if (inst.cmd == '<' && inst.arg < 127) {
                move_offset = (inst.arg+1);
        } else return 0;
        shift(pos);


        // the move instruction
        assert_match(cmd, prog, pos, '+');
        assert_match(arg, prog, pos, 0);
        shift(pos);


        // entrance to the move destination
        if (move_offset > 0) {
                assert_match(cmd, prog, pos, '>');
                assert_match(arg, prog, pos, move_offset-1);
        } else {
                assert_match(cmd, prog, pos, '<');
                assert_match(arg, prog, pos, -move_offset-1);
        }
        shift(pos);


        // - if the move order is -+
        if (move_order == MOVE_ORDER_MINUS_PLUS) {
                assert_match(cmd, prog, pos, '-');
                assert_match(arg, prog, pos, 0);
                shift(pos);
        }


        assert_match(cmd, prog, pos, '[');

        vector_truncate(prog, pos);
        vector_push(prog, '^');
        vector_push(prog, move_offset);

        return 1;
}

void apply_shorthands(struct vector *prog) {
        char changed;
        do {
                changed = 0;
                //changed |= apply_loop_cancellation(prog);
                changed |= apply_copying_shorthand(prog);
        } while (changed);
}

short *optimize(char program_in[]) {
        struct vector program_out = vector_create(0);

        long ind = 0;
        char last_char = 0;
        char cur_char;
        int count = -1;

        while (cur_char = program_in[ind++]) {
                if (cur_char != '+' &&
                    cur_char != '-' &&
                    cur_char != '>' &&
                    cur_char != '<' &&
                    cur_char != '[' &&
                    cur_char != ']' &&
                    cur_char != '.' &&
                    cur_char != ','
#ifdef DEBUGGER
                    && cur_char != '#'
#endif
                    ) continue;

                count++;
                /* This will be TRUE for the first iteration */
                if (
                                cur_char != last_char
                              || count == 256
                              || last_char == '['
                              || last_char == ']'
                              || last_char == '.'
                              || last_char == ',') {
                        /* 1. End writing the last block (if it exists) */
                        /* This will the FALSE for the first iteration */
                        if (count) {
                                count--;
                                vector_push(&program_out, count);
                                apply_shorthands(&program_out);
                        }

                        /* 2. Start writing the next block */
                        vector_push(&program_out, cur_char);
                        count = 0;
                }
                last_char = cur_char;
        };

        vector_push(&program_out, (char)count);
        apply_shorthands(&program_out);
        vector_push(&program_out, (char)0x00);
        vector_push(&program_out, (char)0x00);

        return vector_unwrap(&program_out);
}
