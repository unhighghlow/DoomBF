#include "config.h"

#define HOT_TAPE (PAGE_SIZE * 4)

void load_page(CELL *tape, unsigned long page_uid);
void store_page(CELL *tape, unsigned long page_uid);

unsigned long get_page_uid(unsigned long dp) {
	return dp / PAGE_SIZE;
}

char get_page_n(unsigned long dp) {
	return dp / PAGE_SIZE % 4;
}

#define CHECK_PAGE_TRANSITION(tape, expected_direction, dp, last_page) \
{ \
	char cur_page = get_page_n(dp); \
	if (last_page != cur_page) { \
                last_page = cur_page; \
                perform_page_switch(tape, expected_direction, dp); \
	} \
}

void perform_page_switch(CELL tape[], signed char expected_direction, unsigned long dp) {
        // 0      1      2      3  
        // store keep current load
        // current page ^
        // expected direction -->
        
        unsigned long current_page_uid = get_page_uid(dp);
        store_page(tape, current_page_uid - (expected_direction * 2));
        load_page(tape, current_page_uid + expected_direction);
}

void store_page(CELL tape[], unsigned long page_uid) {
        char page_n = page_uid % 4;
        CELL *target = &tape[PAGE_SIZE*page_n];

        char filename[17];
        sprintf(filename, "%016lx", page_uid);
#ifdef DEBUG
        printf("storing page: 0x%s... ", filename);
#endif

	FILE *f = fopen(filename, "wb");
        if (!f) {
                printf("page store failed! (open)\n");
                exit(1);
        }

	if (fwrite(target, 1, PAGE_SIZE, f) < PAGE_SIZE) {
                printf("page store failed! (write)\n");
                exit(1);
        }
#ifdef DEBUG
        printf("ok\n");
#endif
        fclose(f);
}

void load_page(CELL tape[], unsigned long page_uid) {
        char page_n = page_uid % 4;
        CELL *target = &tape[PAGE_SIZE*page_n];

        char filename[17];
        sprintf(filename, "%016lx", page_uid);
#ifdef DEBUG
        printf("loading page: 0x%s... ", filename);
#endif

	FILE *f = fopen(filename, "rb");
        if (!f) {
                memset(target, 0, PAGE_SIZE);
#ifdef DEBUG
                printf("empty\n");
#endif
                return;
        }

	if (fread(target, 1, PAGE_SIZE, f) < PAGE_SIZE) {
                printf("page load failed!\n");
                exit(1);
        }
#ifdef DEBUG
        printf("ok\n");
#endif
        fclose(f);
}
