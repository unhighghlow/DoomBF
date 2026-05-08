#include <stdlib.h>
#include <stdio.h>

char parse_digit(char digit) {
        char out;
        if (digit >= '0' && digit <= '9') {
                out = digit-'0';
        } else if (digit >= 'a' && digit <= 'f'){
                out = digit-'a'+0xa;
        } else {
                out = -1;
        }
        return out;
}

int main(int argc, char *argv[]) {
        char buf[1];
        char digit;
        char last_char = ' ';
        char reading_number = 0;
        unsigned int rep_count = 0;

        setvbuf(stdout, NULL, _IONBF, 0);
        while (fread(buf, 1, 1, stdin)) {
                if (reading_number && buf[0] == '}') {
                        if (!rep_count) rep_count = 1;
                        while (--rep_count) {
                                printf("%c", last_char);
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
                        printf("%c", buf[0]);
                }
        }
}
