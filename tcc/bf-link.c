#ifdef TARGET_DEFS_ONLY

#define EM_TCC_TARGET EM_386

/* relocation type for 32 bit data relocation */
#define R_DATA_32   R_386_32
#define R_DATA_PTR  R_386_32
#define R_JMP_SLOT  R_386_JMP_SLOT
#define R_GLOB_DAT  R_386_GLOB_DAT
#define R_COPY      R_386_COPY
#define R_RELATIVE  R_386_RELATIVE

#define R_NUM       R_386_NUM

#define ELF_START_ADDR 0x08048000
#define ELF_PAGE_SIZE  0x1000

#define PCRELATIVE_DLLPLT 0
#define RELOCATE_DLLPLT 0

#else /* !TARGET_DEFS_ONLY */

#include "tcc.h"
ST_FUNC int code_reloc (int reloc_type)
{
    printf("code_reloc!!!");
    return 0;
}

ST_FUNC int gotplt_entry_type (int reloc_type)
{
        printf("gotplt_entry_type!!!");
        return 0;
}

ST_FUNC unsigned create_plt_entry(TCCState *s1, unsigned got_offset, struct sym_attr *attr)
{
        printf("create_plt_entry!!!");
        return 0;
}

ST_FUNC void relocate_init(Section *sr)
{
        printf("relocate_init!!!");
}

ST_FUNC void relocate(TCCState *s1, ElfW_Rel *rel, int type, unsigned char *ptr, addr_t addr, addr_t val)
{
        printf("relocate!!!");
}

ST_FUNC void relocate_plt(TCCState *s1)
{
        printf("relocate_plt!!!");
}


#endif