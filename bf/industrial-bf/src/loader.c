// bld - Create a page directory with the specified memory contents

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void store_page(char *page, long page_size, long page_uid);

int main(int argc, char *argv[]) {
	char *filename;
	long page_size;

	if (argc < 2 || argc > 3) {
		printf("usage: %s <input> [page_size=4096]\n", argv[0]);
		return 1;
	}

        filename = argv[1];
	if (argc == 3) {
		page_size = atoi(argv[2]);
	} else {
		page_size = 4096;
	}

	FILE *f = fopen(filename, "rb");
	char *tape = malloc(page_size);
	long page_uid = 0;

	while (fread(tape, 1, page_size, f)) {
		store_page(tape, page_size, page_uid++);
	}
}

void store_page(char *page, long page_size, long page_uid) {
        char filename[17];
        sprintf(filename, "%016lx", page_uid);
        printf("storing page: 0x%s... ", filename);

	FILE *f = fopen(filename, "wb");
        if (!f) {
                printf("page store failed! (open)\n");
                abort();
        }

	if (fwrite(page, 1, page_size, f) < page_size) {
                printf("page store failed! (write)\n");
                abort();
        }
        printf("ok\n");
        fclose(f);
}
