#include <collection_tests.h>
#include <bf_test.h>
#include <stdio.h>

int main(int argc, char** argv) {
#ifndef NOTESTS
    run_vec_tests();
    run_deque_tests();
#endif
    
    if (argc != 2) {
        printf("usage: hackablebf test.b\n");
        return 0;
    }
    
    test_bf(argv[1]);
    return 0;
}
