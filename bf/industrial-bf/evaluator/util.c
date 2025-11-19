#include <arpa/inet.h>

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

void write_long(char *ptr, unsigned long item) {
	for (int i = 0; i < 8; i++) {
		*(ptr+i) = item>>(8*(7-i));
	}
}

#define htonll(x) ((1==htonl(1)) ? (x) : ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define ntohll(x) ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))
