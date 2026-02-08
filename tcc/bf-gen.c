#ifdef TARGET_DEFS_ONLY

/* pointer size, in bytes */
#define PTR_SIZE 4

/* Number of registers available to allocator */
#define NB_REGS 8 // R0 - R7

/* long double size and alignment, in bytes */
#define LDOUBLE_SIZE  16
#define LDOUBLE_ALIGN 16

/* a register can belong to several classes. The classes must be
   sorted from more general to more precise (see gv2() code which does
   assumptions on it). */
#define RC_INT     0x0001 /* generic integer register */
#define RC_FLOAT   0x0002 /* generic float register */
#define RC_R0      0x0004
#define RC_R1      0x0008 
#define RC_R2      0x0010
#define RC_R3      0x0020
#define RC_R4      0x0040
#define RC_R5      0x0080
#define RC_R6      0x0100
#define RC_R7      0x0200

#define RC_IRET    RC_R1 /* function return: integer register */
#define RC_IRE2    RC_R2 /* function return: second integer register */
#define RC_FRET    RC_R1 /* function return: float register */

/* pretty names for the registers */
enum {
    TREG_R0  = 0,
    TREG_R1,
    TREG_R2,
    TREG_R3,
    TREG_R4,
    TREG_R5,
    TREG_R6,
    TREG_R7,
    TREG_SP  = 8
};

/* return registers for function */
#define REG_IRET TREG_R1 /* single word int return register */
#define REG_IRE2 TREG_R2 /* second word return register (for long long) */
#define REG_FRET TREG_R1 /* float return register */

/* defined if function parameters must be evaluated in reverse order */
#define INVERT_FUNC_PARAMS

/* maximum alignment (for aligned attribute support) */
#define MAX_ALIGN     8

#define CHAR_IS_UNSIGNED

#else

#define USING_GLOBALS
#include "tcc.h"

ST_DATA const int reg_classes[NB_REGS] = {
  RC_INT | RC_R1,
  RC_INT | RC_R2,
  RC_INT | RC_R3,
  RC_INT | RC_R4,
  RC_INT | RC_R5,
  RC_INT | RC_R6,
  RC_INT | RC_R7
};

ST_DATA const char * const target_machine_defs =
    "__brainfuck\0"
    ;

ST_FUNC void o(unsigned int c)
{
    int ind1 = ind + 4;
    if (nocode_wanted)
        return;
    if (ind1 > cur_text_section->data_allocated)
        section_realloc(cur_text_section, ind1);
    write32le(cur_text_section->data + ind, c);
    ind = ind1;
}

ST_FUNC void oCHAR(char c)
{
    int ind1 = ind + 1;
    if (nocode_wanted)
        return;
    if (ind1 > cur_text_section->data_allocated)
        section_realloc(cur_text_section, ind1);
    cur_text_section->data[ind] = c;
    ind = ind1;
}

ST_FUNC void oSTR(char *s)
{
    char chr;
    while ((chr = *(s++))) {
        oCHAR(chr);
    }
}

ST_FUNC void oTO(int reg)
{
    reg++;
    while (reg) {
        oCHAR('>');
        reg--;
    }
}

ST_FUNC void oFROM(int reg)
{
    reg++;
    while (reg) {
        oCHAR('<');
        reg--;
    }
}

// Patch all branches in list pointed to by t to branch to a:
ST_FUNC void gsym_addr(int t_, int a_)
{
    uint32_t t = t_;
    uint32_t a = a_;
    tcc_error("implement me: %s", __FUNCTION__);
    while (t) {
        unsigned char *ptr = cur_text_section->data + t;
        uint32_t next = read32le(ptr);
        if (a - t + 0x8000000 >= 0x10000000)
            tcc_error("branch out of range");
        write32le(ptr, (a - t == 4 ? 0xd503201f : // nop
                        0x14000000 | ((a - t) >> 2 & 0x3ffffff))); // b
        t = next;
    }
}

ST_FUNC void load(int r, SValue *sv)
{
    int fr = sv->r;
    int v = fr & VT_VALMASK;
    char fc = sv->c.i;
    if (v == VT_CONST) {
        if (fr & VT_SYM)
          tcc_error("unimp: load(sym)");
        if (is_float(sv->type.t))
          tcc_error("unimp: load(float)");
        if (fc != sv->c.i)
          tcc_error("unimp: load(very large const)");
        if (((unsigned)fc + (1 << 11)) >> 12)
          tcc_error("unimp: load(large const) (0x%x)", fc);

        oTO(r);
        oSTR("[-]");
        while (fc--) {
                oCHAR('+');
        }
        oFROM(r);
    } else
      tcc_error("unimp: load(non-const)");
}

ST_FUNC void store(int r, SValue *sv)
{
    /* XXX: Implement me */
}

ST_FUNC void gfunc_call(int nb_args)
{
    tcc_error("implement me: %s", __FUNCTION__);
}

ST_FUNC void gfunc_prolog(Sym *func_sym)
{
    oSTR("prolog");
}
ST_FUNC void gen_va_start(void)
{
    tcc_error("implement me: %s", __FUNCTION__);
}
ST_FUNC void gen_va_arg(CType *t)
{
    tcc_error("implement me: %s", __FUNCTION__);
}
ST_FUNC int gfunc_sret(CType *vt, int variadic, CType *ret,
                       int *align, int *regsize)
{
    tcc_error("implement me: %s", __FUNCTION__);
}
ST_FUNC void gfunc_return(CType *func_type)
{
    oSTR("return");
    vtop--;
}
ST_FUNC void gfunc_epilog(void)
{
    oSTR("epilog");
}
ST_FUNC void gen_fill_nops(int bytes)
{
    tcc_error("implement me: %s", __FUNCTION__);
    if ((bytes & 3))
      tcc_error("alignment of code section not multiple of 4");
}

// Generate forward branch to label:
ST_FUNC int gjmp(int t)
{
    tcc_error("implement me: %s", __FUNCTION__);
}

// Generate branch to known address:
ST_FUNC void gjmp_addr(int a)
{
    tcc_error("implement me: %s", __FUNCTION__);
}

ST_FUNC int gjmp_cond(int op, int t)
{
    tcc_error("implement me: %s", __FUNCTION__);
}

ST_FUNC int gjmp_append(int n, int t)
{
    tcc_error("implement me: %s", __FUNCTION__);
}

ST_FUNC int gtst(int inv, int t)
{
    tcc_error("implement me: %s", __FUNCTION__);
}
ST_FUNC void gen_opi(int op)
{
    tcc_error("implement me: %s", __FUNCTION__);
}

ST_FUNC void gen_opf(int op)
{
    tcc_error("implement me: %s", __FUNCTION__);
}
ST_FUNC void gen_cvt_sxtw(void)
{
    tcc_error("implement me: %s", __FUNCTION__);
}

ST_FUNC void gen_cvt_itof(int t)
{
    tcc_error("implement me: %s", __FUNCTION__);
}

ST_FUNC void gen_cvt_ftoi(int t)
{
    tcc_error("implement me: %s", __FUNCTION__);
}

ST_FUNC void gen_cvt_ftof(int t)
{
    tcc_error("implement me: %s", __FUNCTION__);
}

ST_FUNC void ggoto(void)
{
    tcc_error("implement me: %s", __FUNCTION__);
}
ST_FUNC void gen_vla_sp_save(int addr)
{
    tcc_error("implement me: %s", __FUNCTION__);
}

ST_FUNC void gen_vla_sp_restore(int addr)
{
    tcc_error("implement me: %s", __FUNCTION__);
}

ST_FUNC void gen_vla_alloc(CType *type, int align)
{
    tcc_error("implement me: %s", __FUNCTION__);
}
#endif

