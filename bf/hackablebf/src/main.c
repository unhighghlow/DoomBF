#include <collection_tests.h>
#include <bf_test.h>

int main(void) {
    run_vec_tests();
    run_deque_tests();
    test_bf();
    return 0;
}
