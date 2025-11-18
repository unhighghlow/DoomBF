// bfm - Converts BrainF to BrainFMacros

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct loop_data {
	char sp;
	unsigned long stack[256];
	unsigned long *ptr_stack[256];
}

char proc_rol_inst(char program_in[], *unsigned long ind, *struct vector program_out, *struct loop_data) {
	// TODO
	vector_push(program_out, program_in[*ind]);
	vector_push(program_out, 0);
	*ind++;
}

char proc_unrol_inst(char program_in[], *unsigned long ind, *struct vector program_out, *struct loop_data) {
	vector_push(program_out, program_in[*ind]);
	vector_push(program_out, 0);
	*ind++;
}

char process_instruction(char program_in[], *unsigned long ind, *struct vector program_out, *struct loop_data) {
	switch (program_in[*ind]) {
		case '+':
		case '-':
		case '>':
		case '<':
			proc_rol_inst(program_in, ind, program_out, loop_data);
			break;
		case '.':
		case ',':
#ifdef DEBUGGER
		case '#':
#endif
			proc_unrol_inst(program_in, ind, program_out, loop_data);
			break;
	}
}

short *optimize(char program_in[]) {
        struct vector program_out = vector_create(0);

        long ind = 0;
        char last_char = 0;
        char cur_char;
        int count = -1;

	// Loop optimization
	struct loop_data ld;
	ld.sp = 0;

	w

        return vector_unwrap(&program_out);
}
