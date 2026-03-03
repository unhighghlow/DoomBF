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
#define RC_R(n)    (0x4 << n)

#define RC_IRET    RC_R(1) /* function return: integer register */
#define RC_IRE2    RC_R(2) /* function return: second integer register */
#define RC_FRET    RC_R(1) /* function return: float register */

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
  RC_INT | RC_R(0),
  RC_INT | RC_R(1),
  RC_INT | RC_R(2),
  RC_INT | RC_R(3),
  RC_INT | RC_R(4),
  RC_INT | RC_R(5),
  RC_INT | RC_R(6),
  RC_INT | RC_R(7)
};

ST_DATA const char * const target_machine_defs =
    "__brainfuck\0"
    ;

static int cur_block_ind = 0;
static int jump_generated = 0;

/* Output an unsigned 32-bit integer */
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

/* Output a single character */
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

/* Output a relocatable value.
 * It is converted to a sequence of pluses
 * in tccbf.c : bf_output_file */
ST_FUNC void oRELOC(unsigned int c)
{
    oCHAR('\0');
    o(c);
}

/* Output a string */
ST_FUNC void oSTR(char *s)
{
    char chr;
    while ((chr = *(s++))) {
        oCHAR(chr);
    }
}

/* Output a comment (remark), replacing
 * all instructions with `_`
 *
 * Does nothing if do_debug is off*/
ST_FUNC void oREM(char *s)
{
    if (!(TCC_STATE_VAR(do_debug))) return;
    char chr;
    while ((chr = *(s++))) {
        if (chr == '+' || chr == '-' ||
            chr == '<' || chr == '>' ||
            chr == '[' || chr == ']' ||
            chr == '.' || chr == ',') {
            oCHAR('_');
            continue;
        }
        oCHAR(chr);
    }
    oCHAR(' ');
}

/* Generate the movement from the base
 * address to a register */
ST_FUNC void oTO(int reg)
{
    reg++;
    while (reg) {
        oCHAR('>');
        reg--;
    }
}

/* Generate the return from a register
 * to the default location */
ST_FUNC void oFROM(int reg)
{
    reg++;
    while (reg) {
        oCHAR('<');
        reg--;
    }
}

/* Output `x` pluses */
ST_FUNC void oSET(int x)
{
    while (x) {
        oCHAR('+');
        x--;
    }
}

/* Generate the beginning of a block */
ST_FUNC void oBBEG()
{
    /* Block indexing starts at 1,
     * so we increment before we
     * generate the block */
    cur_block_ind++;
    jump_generated = 0;

    oREM("block");
    // next  'block_id   0       0
    oSTR("->[-]+>[-]+<<");
    // next  'block_id   1       1
    oSTR("[>->-<]>[>-]<[->\n");
    // next   block_id   0      '0
}

/* Generate the ending of a block */
ST_FUNC void oBEND()
{
    oREM("end");
    if (!jump_generated) {
        oREM("&jump");
        oSTR("<<<");
        oSET(cur_block_ind + 1);
        oSTR(">>>");
    }
    oSTR("<]<\n");
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
    if (fr & VT_LVAL) {
        oSTR("lvalload to r");
        oCHAR(r+'0');
    } else if (v == VT_CONST) {
        oSTR("constload to r");
        oCHAR(r+'0');
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
        oSET(fc);
        oFROM(r);
    } else
      tcc_error("unimp: load(?)");
}

ST_FUNC void store(int r, SValue *sv)
{
    /* XXX: Implement me */
}

static void gcall(void)
{
    if ((vtop->r & (VT_VALMASK | VT_LVAL)) == VT_CONST &&
        ((vtop->r & VT_SYM) && vtop->c.i == (int)vtop->c.i)) {

        oREM("call");
        oSTR("<<<[-]");

        /* constant symbolic case -> simple relocation */
        greloca(cur_text_section, vtop->sym, ind,
                R_RISCV_CALL_PLT, (int)vtop->c.i);

        oRELOC(1);
        oSTR(">>>\n");
    } else {
        tcc_error("unimp: indirect call");
    }
}

ST_FUNC void gfunc_call(int nb_args)
{
    int i, align, size, aireg;
    aireg = 0;
    for (i = 0; i < nb_args; i++) {
        size = type_size(&vtop[-i].type, &align);
        if (size > 8 || ((vtop[-i].type.t & VT_BTYPE) == VT_STRUCT)
            || is_float(vtop[-i].type.t))
          tcc_error("unimp: call arg %d wrong type", nb_args - i);
        if (aireg >= 8)
          tcc_error("unimp: too many register args");
        vrotb(i+1);
        gv(RC_R(aireg));
        vrott(i+1);
        aireg++;
    }
    vrotb(nb_args + 1);
    gcall();

    jump_generated = 1;
    oBEND();
    oBBEG();

    vtop -= nb_args + 1;
}

ST_FUNC void gfunc_prolog(Sym *func_sym)
{
    oBBEG();
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
    oSTR("return\n");
    vtop--;
}
ST_FUNC void gfunc_epilog(void)
{
    oBEND();
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

