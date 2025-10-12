// bfm - Converts BrainF to BrainFMacros

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int optimize(FILE *f_in, FILE *f_out);

int main(int argc, char *argv[]) {
	char *filename;
	char *filename_out;

	if (argc < 2 || argc > 3) {
		printf("usage: %s <input> [<output>]\n", argv[0]);
		return 1;
	}

        filename = argv[1];
        if (argc == 3) {
                filename_out = argv[2];
        } else {
                filename_out = malloc(strlen(filename)+5);
                memcpy(filename_out, filename, strlen(filename));
                filename_out = strcat(filename_out, ".bfm");
        }

	FILE *f_in = fopen(filename, "rb");
	FILE *f_out = fopen(filename_out, "wb");
        return optimize(f_in, f_out);
}

int optimize(FILE *f_in, FILE *f_out) {
        char last_char = 0;
        char cur_char;
        int count = -1;

        while (1) {
                cur_char = 0;

                fread(&cur_char, 1, 1, f_in);
                if (!cur_char) {
                        break;
                }
                if (cur_char != '+' &&
                    cur_char != '-' &&
                    cur_char != '>' &&
                    cur_char != '<' &&
                    cur_char != '[' &&
                    cur_char != ']' &&
                    cur_char != '.' &&
                    cur_char != ',') continue;

                count++;
                /* This will be TRUE for the first iteration */
                if (
                                cur_char != last_char
                              || count == 256
                              || last_char == '['
                              || last_char == ']'
                              || last_char == '.'
                              || last_char == ',') {
                        /* 1. End writing the last block (if it exists) */ \
                        /* This will the FALSE for the first iteration */ \
                        if (count) { \
                                count--; \
                                fwrite(&count, 1, 1, f_out); \
                        }

                        /* 2. Start writing the next block */
                        fwrite(&cur_char, 1, 1, f_out);
                        count = 0;
                }
                last_char = cur_char;
        };

        fwrite(&count, 1, 1, f_out);

        fclose(f_in);
        fclose(f_out);

        return 0;
}
