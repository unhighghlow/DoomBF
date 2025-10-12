#include <collection_tests.h>
#include <collections.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>

typedef int test_int_t;
typedef double test_double_t;

DECL_VEC(test_int_t);
DECL_VEC(test_double_t);
DECL_DEQUE(test_int_t);

void test_vec_init(void) {
    printf("Running test_vec_init... ");
    VEC_TYPE(test_int_t) vec = VEC_INIT();
    assert(vec.data == NULL);
    assert(vec.length == 0);
    assert(vec.capacity == 0);
    printf("OK\n");
}

void test_vec_push_and_grow(void) {
    printf("Running test_vec_push_and_grow... ");
    VEC_TYPE(test_int_t) vec = VEC_INIT();
    bool res;

    for (size_t i = 0; i < 8; i++) {
        VEC_PUSH(vec, i, res);
        assert(res);
        assert(vec.capacity == 8);
    }

    VEC_PUSH(vec, 8, res);
    assert(res);
    assert(vec.length == 9);
    assert(vec.capacity == 16);

    VEC_FREE(vec);
    printf("OK\n");
}

void test_vec_reserve(void) {
    printf("Running test_vec_reserve... ");
    VEC_TYPE(test_int_t) vec = VEC_INIT();
    bool res;

    VEC_RESERVE(vec, 100, res);
    assert(res);
    assert(vec.capacity == 100);
    assert(vec.length == 0);

    VEC_RESERVE(vec, 50, res);
    assert(res);
    assert(vec.capacity == 50);
    assert(vec.length == 0);

    for (int i = 0; i < 60; i++) {
        VEC_PUSH(vec, i, res);
        assert(res);
    }
    assert(vec.capacity == 100);
    assert(vec.length == 60);

    VEC_RESERVE(vec, 50, res);
    assert(res);
    assert(vec.capacity == 60);
    assert(vec.length == 60);

    VEC_FREE(vec);
    printf("OK\n");
}

void test_vec_shrink_to_fit(void) {
    printf("Running test_vec_shrink_to_fit... ");
    VEC_TYPE(test_int_t) vec = VEC_INIT();
    bool res;

    for (int i = 0; i < 10; i++) {
        VEC_PUSH(vec, i, res);
        assert(res);
    }

    VEC_SHRINK_TO_FIT(vec, res);
    assert(res);
    assert(vec.capacity == vec.length);

    VEC_FREE(vec);
    printf("OK\n");
}

void test_vec_get(void) {
    printf("Running test_vec_get... ");
    VEC_TYPE(test_int_t) vec = VEC_INIT();
    bool res;
    int out;

    VEC_PUSH(vec, 42, res);
    assert(res);

    VEC_PUSH(vec, 33, res);
    assert(res);

    VEC_PUSH(vec, 91, res);
    assert(res);

    VEC_GET(vec, 0, out, res);
    assert(res);
    assert(out == 42);

    VEC_GET(vec, 2, out, res);
    assert(res);
    assert(out == 91);

    VEC_GET(vec, 3, out, res);
    assert(!res);

    VEC_FREE(vec);
    printf("OK\n");
}

void test_vec_remove(void) {
    printf("Running test_vec_remove... ");
    VEC_TYPE(test_int_t) vec = VEC_INIT();
    bool res;

    for (int i = 0; i < 5; i++) {
        VEC_PUSH(vec, i, res);
        assert(res);
    }

    VEC_REMOVE(vec, 2, res);
    assert(res);
    assert(vec.length == 4);

    int val;
    VEC_GET(vec, 2, val, res);
    assert(res);
    assert(val == 3);

    VEC_REMOVE(vec, 10, res);
    assert(!res);

    VEC_FREE(vec);
    printf("OK\n");
}

void test_vec_pop(void) {
    printf("Running test_vec_pop... ");
    VEC_TYPE(test_int_t) vec = VEC_INIT();
    bool res;
    int out;

    VEC_PUSH(vec, 42, res);
    assert(res);

    VEC_POP(vec, out, res);
    assert(res);
    assert(out == 42);
    assert(vec.length == 0);

    VEC_POP(vec, out, res);
    assert(!res);

    VEC_FREE(vec);
    printf("OK\n");
}

void test_vec_iterator(void) {
    printf("Running test_vec_iterator... ");
    VEC_TYPE(test_int_t) vec = VEC_INIT();
    bool res;
    int *begin, *end;

    VEC_GET_ITERATOR(vec, begin, end, res);
    assert(!res);

    VEC_PUSH(vec, 1, res);
    VEC_PUSH(vec, 2, res);
    VEC_PUSH(vec, 3, res);
    VEC_PUSH(vec, 9, res);
    VEC_GET_ITERATOR(vec, begin, end, res);
    assert(res);
    assert(*begin == 1);
    assert(*end == 9);
    assert(begin < end);

    VEC_FREE(vec);
    printf("OK\n");
}

void test_vec_clear(void) {
    printf("Running test_vec_clear... ");
    VEC_TYPE(test_int_t) vec = VEC_INIT();
    bool res;

    VEC_PUSH(vec, 42, res);
    assert(res);
    assert(vec.length == 1);

    VEC_CLEAR(vec);
    assert(vec.length == 0);
    assert(vec.data != NULL);

    VEC_FREE(vec);
    printf("OK\n");
}

void test_vec_double_free(void) {
    printf("Running test_vec_double_free... ");
    VEC_TYPE(test_int_t) vec = VEC_INIT();
    bool res;

    VEC_PUSH(vec, 42, res);
    VEC_FREE(vec);
    VEC_FREE(vec);

    printf("OK\n");
}

void test_vec_different_types(void) {
    printf("Running test_vec_different_types... ");
    VEC_TYPE(test_double_t) vec = VEC_INIT();
    bool res;

    VEC_PUSH(vec, 3.14, res);
    assert(res);

    double out;
    VEC_GET(vec, 0, out, res);
    assert(res);
    assert(out == 3.14);

    VEC_FREE(vec);
    printf("OK\n");
}

void test_vec_edge_cases(void) {
    printf("Running test_vec_edge_cases... ");
    VEC_TYPE(test_int_t) vec = VEC_INIT();
    bool res;

    VEC_RESERVE(vec, SIZE_MAX, res);
    assert(!res);

    if (res) {
        VEC_PUSH(vec, 42, res);
        assert(!res);
    }

    VEC_FREE(vec);
    printf("OK\n");
}

void test_deque_init(void) {
    printf("Running test_deque_init... ");
    DEQUE_TYPE(test_int_t) d = DEQUE_INIT();
    assert(d.front.data == NULL);
    assert(d.front.length == 0);
    assert(d.front.capacity == 0);
    assert(d.back.data == NULL);
    assert(d.back.length == 0);
    assert(d.back.capacity == 0);

    DEQUE_FREE(d);
    printf("OK\n");
}

void test_deque_push_front_and_back(void) {
    printf("Running test_deque_push_front_and_back... ");
    DEQUE_TYPE(test_int_t) d = DEQUE_INIT();
    bool res;
    int val;

    DEQUE_BACK(d, val, res);
    assert(!res);

    DEQUE_FRONT(d, val, res);
    assert(!res);

    DEQUE_PUSH_FRONT(d, 10, res);
    assert(res);

    DEQUE_BACK(d, val, res);
    assert(res);
    assert(val == 10);

    DEQUE_FRONT(d, val, res);
    assert(res);
    assert(val == 10);

    DEQUE_PUSH_BACK(d, 42, res);
    assert(res);
    DEQUE_PUSH_BACK(d, 91, res);
    assert(res);

    DEQUE_FRONT(d, val, res);
    assert(res);
    assert(val == 10);

    DEQUE_BACK(d, val, res);
    assert(res);
    assert(val == 91);

    DEQUE_FREE(d);
    printf("OK\n");
}

void test_deque_length(void) {
    printf("Running test_deque_length... ");
    DEQUE_TYPE(test_int_t) d = DEQUE_INIT();
    assert(DEQUE_IS_EMPTY(d));
    assert(DEQUE_LENGTH(d) == 0);

    bool res;
    DEQUE_PUSH_BACK(d, 42, res);
    assert(res);
    DEQUE_PUSH_BACK(d, 91, res);
    assert(res);
    DEQUE_PUSH_FRONT(d, 10, res);
    assert(res);

    assert(DEQUE_LENGTH(d) == 3);

    DEQUE_FREE(d);
    printf("OK\n");
}

void test_deque_get(void) {
    printf("Running test_deque_get... ");
    DEQUE_TYPE(test_int_t) d = DEQUE_INIT();
    bool res;
    int val;

    DEQUE_PUSH_BACK(d, 42, res);
    assert(res);
    DEQUE_PUSH_BACK(d, 91, res);
    assert(res);
    DEQUE_PUSH_FRONT(d, 10, res);
    assert(res);

    DEQUE_GET(d, 0, val, res);
    assert(res);
    assert(val == 10);

    DEQUE_GET(d, 1, val, res);
    assert(res);
    assert(val == 42);

    DEQUE_GET(d, 2, val, res);
    assert(res);
    assert(val == 91);

    DEQUE_FREE(d);
    printf("OK\n");
}

void test_deque_pop_front_and_back(void) {
    printf("Running test_deque_pop_front_and_back... ");
    DEQUE_TYPE(test_int_t) d = DEQUE_INIT();
    bool res;
    int val;

    DEQUE_PUSH_BACK(d, 0, res);
    assert(res);
    DEQUE_FRONT(d, val, res);
    assert(res);
    assert(val == 0);
    DEQUE_BACK(d, val, res);
    assert(res);
    assert(val == 0);

    DEQUE_PUSH_BACK(d, 1, res);
    assert(res);
    DEQUE_FRONT(d, val, res);
    assert(res);
    assert(val == 0);
    DEQUE_BACK(d, val, res);
    assert(res);
    assert(val == 1);

    DEQUE_PUSH_BACK(d, 2, res);
    assert(res);
    DEQUE_FRONT(d, val, res);
    assert(res);
    assert(val == 0);
    DEQUE_BACK(d, val, res);
    assert(res);
    assert(val == 2);

    DEQUE_PUSH_BACK(d, 3, res);
    assert(res);
    DEQUE_FRONT(d, val, res);
    assert(res);
    assert(val == 0);
    DEQUE_BACK(d, val, res);
    assert(res);
    assert(val == 3);

    for(size_t i = 4; i < 8; i++) {
        DEQUE_PUSH_BACK(d, i, res);
        assert(res);
    }

    DEQUE_PUSH_FRONT(d, 0, res);
    assert(res);
    DEQUE_FRONT(d, val, res);
    assert(res);
    assert(val == 0);
    DEQUE_BACK(d, val, res);
    assert(res);
    assert(val == 7);

    DEQUE_PUSH_FRONT(d, 1, res);
    assert(res);
    DEQUE_FRONT(d, val, res);
    assert(res);
    assert(val == 1);
    DEQUE_BACK(d, val, res);
    assert(res);
    assert(val == 7);

    DEQUE_PUSH_FRONT(d, 2, res);
    assert(res);
    DEQUE_FRONT(d, val, res);
    assert(res);
    assert(val == 2);
    DEQUE_BACK(d, val, res);
    assert(res);
    assert(val == 7);

    DEQUE_PUSH_FRONT(d, 3, res);
    assert(res);
    DEQUE_FRONT(d, val, res);
    assert(res);
    assert(val == 3);
    DEQUE_BACK(d, val, res);
    assert(res);
    assert(val == 7);

    assert(DEQUE_LENGTH(d) == 12);

    DEQUE_POP_FRONT(d, val, res);
    assert(res);
    DEQUE_FRONT(d, val, res);
    assert(res);
    assert(val == 2);
    DEQUE_BACK(d, val, res);
    assert(res);
    assert(val == 7);

    DEQUE_POP_FRONT(d, val, res);
    assert(res);
    DEQUE_FRONT(d, val, res);
    assert(res);
    assert(val == 1);
    DEQUE_BACK(d, val, res);
    assert(res);
    assert(val == 7);

    DEQUE_POP_FRONT(d, val, res);
    assert(res);
    DEQUE_FRONT(d, val, res);
    assert(res);
    assert(val == 0);
    DEQUE_BACK(d, val, res);
    assert(res);
    assert(val == 7);

    DEQUE_POP_FRONT(d, val, res);
    assert(res);
    DEQUE_FRONT(d, val, res);
    assert(res);
    assert(val == 0);
    DEQUE_BACK(d, val, res);
    assert(res);
    assert(val == 7);

    DEQUE_POP_FRONT(d, val, res);
    assert(res);
    DEQUE_FRONT(d, val, res);
    assert(res);
    assert(val == 1);
    DEQUE_BACK(d, val, res);
    assert(res);
    assert(val == 7);

    DEQUE_POP_FRONT(d, val, res);
    assert(res);
    DEQUE_FRONT(d, val, res);
    assert(res);
    assert(val == 2);
    DEQUE_BACK(d, val, res);
    assert(res);
    assert(val == 7);

    DEQUE_POP_FRONT(d, val, res);
    assert(res);
    DEQUE_FRONT(d, val, res);
    assert(res);
    assert(val == 3);
    DEQUE_BACK(d, val, res);
    assert(res);
    assert(val == 7);

    DEQUE_POP_FRONT(d, val, res);
    assert(res);
    DEQUE_FRONT(d, val, res);
    assert(res);
    assert(val == 4);
    DEQUE_BACK(d, val, res);
    assert(res);
    assert(val == 7);

    DEQUE_POP_FRONT(d, val, res);
    assert(res);
    DEQUE_FRONT(d, val, res);
    assert(res);
    assert(val == 5);
    DEQUE_BACK(d, val, res);
    assert(res);
    assert(val == 7);

    assert(DEQUE_LENGTH(d) == 3);

    assert(d.front.length == 1);
    assert(d.back.length == 2);

    DEQUE_POP_BACK(d, val, res);
    assert(res);
    DEQUE_FRONT(d, val, res);
    assert(res);
    assert(val == 5);
    DEQUE_BACK(d, val, res);
    assert(res);
    assert(val == 6);

    DEQUE_POP_BACK(d, val, res);
    assert(res);
    DEQUE_FRONT(d, val, res);
    assert(res);
    assert(val == 5);
    DEQUE_BACK(d, val, res);
    assert(res);
    assert(val == 5);

    DEQUE_POP_BACK(d, val, res);
    assert(res);
    assert(val == 5);

    assert(DEQUE_IS_EMPTY(d));

    DEQUE_POP_BACK(d, val, res);
    assert(!res);

    DEQUE_POP_FRONT(d, val, res);
    assert(!res);

    for(size_t i = 0; i < 8; i++) {
        DEQUE_PUSH_FRONT(d, i, res);
        assert(res);
    }

    for(size_t i = 0; i < 4; i++) {
        DEQUE_PUSH_BACK(d, i, res);
        assert(res);
    }

    assert(DEQUE_LENGTH(d) == 12);

    DEQUE_BACK(d, val, res);
    assert(res);
    assert(val == 3);

    DEQUE_FRONT(d, val, res);
    assert(res);
    assert(val == 7);

    for(size_t i = 0; i < 9; i++) {
        DEQUE_POP_BACK(d, val, res);
        assert(res);
    }

    assert(DEQUE_LENGTH(d) == 3);

    DEQUE_BACK(d, val, res);
    assert(res);
    assert(val == 5);

    assert(d.back.length == 1);
    assert(d.front.length == 2);

    for(size_t i = 0; i < 2; i++) {
        DEQUE_POP_FRONT(d, val, res);
        assert(res);
    }

    DEQUE_POP_FRONT(d, val, res);
    assert(res);
    assert(val == 5);

    DEQUE_FREE(d);
    printf("OK\n");
}

void test_deque_clear(void) {
    printf("Running test_deque_clear... ");
    DEQUE_TYPE(test_int_t) d = DEQUE_INIT();
    bool res;

    DEQUE_PUSH_BACK(d, 42, res);
    assert(res);
    assert(DEQUE_LENGTH(d) == 1);

    DEQUE_PUSH_FRONT(d, 71, res);
    assert(res);
    assert(DEQUE_LENGTH(d) == 2);

    DEQUE_PUSH_FRONT(d, 101, res);
    assert(res);
    assert(DEQUE_LENGTH(d) == 3);

    DEQUE_CLEAR(d);
    assert(DEQUE_IS_EMPTY(d));
    assert(DEQUE_LENGTH(d) == 0);
    assert(d.front.data != NULL);
    assert(d.back.data != NULL);

    DEQUE_FREE(d);
    printf("OK\n");
}

void test_deque_double_free(void) {
    printf("Running test_deque_double_free... ");
    DEQUE_TYPE(test_int_t) d = DEQUE_INIT();
    bool res;

    DEQUE_PUSH_BACK(d, 42, res);
    DEQUE_FREE(d);
    DEQUE_FREE(d);

    printf("OK\n");
}

void run_vec_tests(void) {
    printf("testing vec...\n");
    test_vec_init();
    test_vec_push_and_grow();
    test_vec_reserve();
    test_vec_shrink_to_fit();
    test_vec_get();
    test_vec_remove();
    test_vec_pop();
    test_vec_iterator();
    test_vec_clear();
    test_vec_different_types();
    test_vec_edge_cases();
    test_vec_double_free();
    printf("All vec tests passed!\n");
}

void run_deque_tests(void) {
    printf("testing deque...\n");
    test_deque_init();
    test_deque_push_front_and_back();
    test_deque_length();
    test_deque_get();
    test_deque_pop_front_and_back();
    test_deque_clear();
    test_deque_double_free();
    printf("All deque tests passed!\n");
}
