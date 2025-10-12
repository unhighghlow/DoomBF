#include "config.h"
#include <bf.h>
#include <bf_test.h>
#include <stdio.h>
#include <stdlib.h>

const code_ptr_t test_arithmetic_progression =
    "+>(1)++>(2)+++>(3)++++>(4)+++++(5)[<+>-]<(ADD)[<+>-]<(ADD)[<+>-]<(ADD)[<+>-]<(ADD).(IO_WRITE)";

char* copy_string(const char* literal) {
    /* Вычисляем длину строкового литерала */
    size_t len = strlen(literal);

    /* Выделяем память в куче: длина + 1 для нуль-терминатора */
    char* new_str = (char*)malloc(len + 1);

    /* Проверяем успешность выделения памяти */
    if (new_str == NULL) {
        return NULL;  /* Возвращаем NULL при ошибке */
    }

    /* Копируем содержимое строкового литерала */
    strcpy(new_str, literal);

    return new_str;
}

code_ptr_t get_arithmetic_progression_code(void) {
    return copy_string(test_arithmetic_progression);
}

tape_element_t* init_tape(void) {
    tape_element_t* tape = (tape_element_t*) malloc(0x100000 * sizeof(tape_element_t));
    for(size_t i = 0; i < 0x100000; i++) {
        tape[i] = 0;
    }
    return tape;
}

void write_to_terminal(tape_element_t tape_element) {
    printf("%d", tape_element);
}

tape_element_t read_from_terminal(void) {
    char c = getchar();
    return (tape_element_t) c;
}

void test_bf(void) {
    printf("testing brainfuck runner...\n");
    run_brainfuck_program(
        get_arithmetic_progression_code,
        init_tape,
        read_from_terminal,
        write_to_terminal
    );
}
