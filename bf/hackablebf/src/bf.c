#include "config.h"
#include "vec.h"
#include <bf.h>
#include <collections.h>
#include <stddef.h>
#include <stdio.h>

void run_brainfuck_program(
    load_code_func_t load_code,
    init_func_t init_tape,
    input_func_t io_read,
    output_func_t io_write
) {
    tape_element_t* tape = init_tape();
    code_ptr_t code = load_code();
    VEC_TYPE(code_ptr_t) return_stack = VEC_INIT();

    tape_element_t* dp = &tape[0];
    code_ptr_t cp = &code[0];

    code_ptr_t ret_addr;
    bool res;

    while(*cp) {
        switch (*cp) {
            case '+': ++(*dp); break;
            case '-': --(*dp); break;
            case '>': ++dp; break;
            case '<': --dp; break;
            case ',': *dp = io_read(); break;
            case '.': io_write(*dp); break;
            case '[':
                VEC_PUSH(return_stack, cp, res);
                break;
            case ']':
                VEC_POP(return_stack, ret_addr, res);
                if(!res) {
                    printf("Fatal Error! Seems there aren't exist matching opening square bracket\n");
                    goto epilogue;
                }
                if (*dp) {
                    cp = ret_addr;
                    VEC_PUSH(return_stack, ret_addr, res);
                }
                break;
            default: break;
        }
        ++cp;
    }

    epilogue:
    VEC_FREE(return_stack);
    free(code);
    free(tape);
}
