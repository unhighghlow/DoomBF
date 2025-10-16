#include <bf.h>
#include <bf_test.h>
#include <stdio.h>
#include <stdlib.h>

char* test_program;
tape_element_t* tape;

bool read_file(char* file_name, char** result) {
	FILE* f = fopen(file_name, "rb");
    if (!f) {
        printf("cannot open file\n");
        return false;
    }
	fseek(f, 0, SEEK_END);
	size_t len = (size_t) ftell(f);
	fseek(f, 0, SEEK_SET);

	*result = (char*) malloc(len + 1);
	fread(*result, len, 1, f);
	fclose(f);

	(*result)[len] = 0;
	return true;
}

char* get_test_code(void) {
    return test_program;
}

tape_element_t* init_tape(void) {
    tape_element_t* res = (tape_element_t*) malloc(0x100000 * sizeof(tape_element_t));
    for(size_t i = 0; i < 0x100000; i++) {
        res[i] = 0;
    }
    return res;
}

void write_to_terminal(tape_element_t tape_element) {
    printf("%c", tape_element);
}

tape_element_t read_from_terminal(void) {
    char c = (char) getchar();
    return (tape_element_t) c;
}

void test_bf(char* file_name) {
    printf("testing brainfuck runner...\n");
    if (!read_file(file_name, &test_program))
        return;

    tape = init_tape();

    run_brainfuck_program(
        get_test_code(),
        tape,
        read_from_terminal,
        write_to_terminal
    );

    free(tape);
    free(test_program);
}
