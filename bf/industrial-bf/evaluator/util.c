#include <arpa/inet.h>
#include <errno.h>

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

char* read_file(char* filename, unsigned long *program_length) {
        FILE *f = fopen(filename, "rb");
        if (!f) {
                printf("cannot open file: %x\n", errno);
                return NULL;
        }
        fseek(f, 0, SEEK_END);
        unsigned long fsize = ftell(f);
        fseek(f, 0, SEEK_SET);

        char *string = safe_malloc(fsize + 1);
        fread(string, fsize, 1, f);
        fclose(f);

        string[fsize] = 0;
        *program_length = fsize;
        return string;
}

signed char parse_digit(char digit) {
        signed char out;
        if (digit >= '0' && digit <= '9') {
                out = digit-'0';
        } else if (digit >= 'a' && digit <= 'f'){
                out = digit-'a'+0xa;
        } else {
                out = -1;
        }
        return out;
}

unsigned long parse_number(char *arr, unsigned long *ind) {
        unsigned long val = 0;
        signed char digit;
        while (1) {
                digit = parse_digit(arr[*ind]);
                if (digit == -1)
                        break;
                val <<= 4;
                val += digit;
                (*ind)++;
        }
        return val;
}

#define htonll(x) ((1==htonl(1)) ? (x) : ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define ntohll(x) ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))
