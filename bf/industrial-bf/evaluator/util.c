long round_up_to_power_of_two(long n) {
        long power = 1;
        while (power < n) power *= 2;
        return power;
}

void *safe_malloc(long size) {
        void *p = malloc(size);
        if (!p) {
                printf("memory allocation failed\n");
                exit(1);
        }
        return p;
}

void *safe_realloc(void *ptr, long size) {
        void *p = realloc(ptr, size);
        if (!p) {
                printf("memory allocation failed\n");
                exit(1);
        }
        return p;
}
