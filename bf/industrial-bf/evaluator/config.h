#define PAGE_SIZE 0x10000 /* 1. For performance reasons it's faster if the page size is a power of 2
                         2. The page size must be bigger than of equal to 256 */
#define CELL uint16_t

#define DEBUGGER
#define CELL_FORMAT_STRING "% 4x"
