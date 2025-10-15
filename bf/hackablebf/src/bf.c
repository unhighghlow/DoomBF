#include "config.h"
#include "vec.h"
#include <bf.h>
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
    code_ptr_t code_iterator = &code[0];
    
    VEC_TYPE(size_t) return_stack = VEC_INIT();
    VEC_TYPE(bf_op_t) ops = VEC_INIT();
    VEC_TYPE(arg_t) args = VEC_INIT();

    tape_element_t* dp = &tape[0];
    size_t cp = 0;

    size_t ret_addr;
    bool res;
    
    bf_op_t op;
    arg_t arg;
    
    while(*code_iterator) { 
        // preparation of the code for more effective interpretation
        // this includes:
        // 1. skip all comments
        // 2. merge repetitions of '+', '-', '>', '<', ',' or '.'
        arg = 1;
        bool skip_comment = false;
        
        switch(*code_iterator++) {
            case '+': op = OP_INC; break;
            case '-': op = OP_DEC; break;
            case '>': op = OP_MOVE_RIGHT; break;
            case '<': op = OP_MOVE_LEFT; break;
            case '.': op = OP_WRITE; break;
            case ',': op = OP_READ; break;
            case '[': op = OP_START_LOOP; break;
            case ']': op = OP_END_LOOP; break;
            default: skip_comment = true; break;
        }
        
        if (skip_comment) continue;
        
        if (ops.length == 0 || op == OP_START_LOOP || op == OP_END_LOOP) {
            VEC_PUSH(ops, op, res);
            VEC_PUSH(args, arg, res);
            continue;
        }
        
        size_t last_id = ops.length-1;        
        if (ops.data[last_id] == op && args.data[last_id] < 255) {
            ++args.data[last_id];
            continue;
        }
        
        VEC_PUSH(ops, op, res);
        VEC_PUSH(args, arg, res);
    }

    while(cp < ops.length) {        
        op = ops.data[cp];
        arg = args.data[cp];
        
        switch (op) {
            case OP_INC: (*dp) += (tape_element_t) arg; break;
            case OP_DEC: (*dp) -= (tape_element_t) arg; break;
            case OP_MOVE_RIGHT: dp += (size_t) arg; break;
            case OP_MOVE_LEFT: dp -= (size_t) arg; break;
            case OP_READ: for (arg_t i = arg; i >= 1; i--) *dp = io_read(); break;
            case OP_WRITE: for (arg_t i = arg; i >= 1; i--) io_write(*dp); break;
            case OP_START_LOOP:
                if (*dp != 0) {
                    VEC_PUSH(return_stack, cp, res);  
                } else {
                    int32_t bracket_count = 1;
                    ++cp;
                    while(cp < ops.length) {
                        switch (ops.data[cp]) {
                            case OP_START_LOOP: ++bracket_count; break;
                            case OP_END_LOOP: --bracket_count; break;
                            default: break;
                        }
                        
                        if (bracket_count == 0) break;
                        
                        ++cp;
                    }
                }
                break;
            case OP_END_LOOP:
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
    VEC_FREE(args);
    VEC_FREE(ops);
    VEC_FREE(return_stack);
    free(code);
    free(tape);
}
