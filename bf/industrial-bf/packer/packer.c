#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char parse_digit(char digit) {
        char out;
        if (digit >= '0' && digit <= '9') {
                out = digit-'0';
        } else if (digit >= 'a' && digit <= 'f'){
                out = digit-'a'+0xa;
        } else if (digit >= 'A' && digit <= 'F'){
                out = digit-'A'+0xa;
        } else {
                out = -1;
        }
        return out;
}

#define MAX_BLOCK 0x10000
int main(int argc, char *argv[]) {
        char pbuf[MAX_BLOCK+1];
        char rbuf[MAX_BLOCK];
        register size_t rbuf_size = 0;
        register size_t rbuf_pos = 0;
        char buf[2];
        char digit;
        char last_char = ' ';
        char reading_number = 0;
        size_t rep_count = 0;
        size_t block;

        setvbuf(stdout, NULL, _IONBF, 0);
        while (1) {
                if (rbuf_pos >= rbuf_size) {
                        rbuf_size = fread(rbuf, 1, MAX_BLOCK, stdin);
                        rbuf_pos = 0;
                }
                buf[0] = rbuf[rbuf_pos++];
                if (reading_number && buf[0] == '}') {
                        if (!rep_count) rep_count = 1;
                        rep_count--;
                        block = rep_count;
                        if (block > MAX_BLOCK) {
                                block = MAX_BLOCK;
                        }
                        memset(pbuf, last_char, block);
                        while (rep_count) {
                                block = rep_count;
                                if (block > MAX_BLOCK) {
                                        block = MAX_BLOCK;
                                }
                                rep_count -= block;
                                pbuf[block] = 0;
                                fputs(pbuf, stdout);
                                pbuf[block] = last_char;
                        }
                        reading_number = 0;
                }
                else if (reading_number) {
                        digit = parse_digit(buf[0]);
                        if (digit == -1) {
                                fprintf(stderr, "invalid digit: %c\n", buf[0]);
                                exit(1);
                        }
                        rep_count <<= 4;
                        rep_count |= digit;
                }
                else if (buf[0] == '{') {
                        reading_number = 1;
                }
                else
                {
                        last_char = buf[0];
                        puts(buf);
                }
        }
}
