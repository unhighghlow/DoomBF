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


/// Runs the brainfuck code
void run_brainfuck_program(
    /// Pointer to the source code. Doesn't get mutated in the process
    char* code,
    /// Pointer to the tape which will be mutated in the process
    tape_element_t* tape,
    /// Callback for provision of input for the ',' operator
    input_func_t input,
    /// Callback for doing of output through the '.' operator
    output_func_t output
);

#endif
