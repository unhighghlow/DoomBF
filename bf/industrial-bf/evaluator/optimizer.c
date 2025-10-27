// bfm - Converts BrainF to BrainFMacros

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

short *optimize(char program_in[]) {
        struct vector program_out = vector_create(0);

        long ind = 0;
        char last_char = 0;
        char cur_char;
        int count = -1;

        while (cur_char = program_in[ind++]) {
#ifdef ASSERTS
                char assert_active;

                if (assert_active && !(
                    (cur_char >= '0' && cur_char <= '9')
                 || (cur_char >= 'a' && cur_char <= 'f')
                )) {
                        assert_active = 0;
                }

                if (
                    (cur_char == '@')
                 || (cur_char == '!')
                ) {
                        assert_active = 1;
                }
#endif

                if (cur_char != '+'
                 && cur_char != '-'
                 && cur_char != '>'
                 && cur_char != '<'
                 && cur_char != '['
                 && cur_char != ']'
                 && cur_char != '.'
                 && cur_char != ','
#ifdef DEBUGGER
                 && cur_char != '#'
#endif
#ifdef ASSERTS
		 && !assert_active
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
                        }

                        /* 2. Start writing the next block */
                        vector_push(&program_out, cur_char);
                        count = 0;
                }
                last_char = cur_char;
        };

        vector_push(&program_out, (char)count);
        vector_push(&program_out, (char)0x00);
        vector_push(&program_out, (char)0x00);

        return vector_unwrap(&program_out);
}
