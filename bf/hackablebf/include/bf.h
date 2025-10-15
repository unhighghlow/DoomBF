#ifndef BF_H__
#define BF_H__

#include <config.h>

typedef enum bf_op {
    OP_START_LOOP,
    OP_END_LOOP,
    OP_MOVE_LEFT,
    OP_MOVE_RIGHT,
    OP_WRITE,
    OP_READ,
    OP_INC,
    OP_DEC
} bf_op_t;

typedef uint8_t arg_t;

DECL_VEC(bf_op_t);
DECL_VEC(arg_t);

void run_brainfuck_program(load_code_func_t, init_func_t, input_func_t, output_func_t);

#endif
