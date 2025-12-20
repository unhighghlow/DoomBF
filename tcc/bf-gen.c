/*
 *  Brainfuck code generator for TCC
 */

/******************************************************/

#ifdef TARGET_DEFS_ONLY

/* number of available registers */
#define NB_REGS         5
#define NB_ASM_REGS     8
#define CONFIG_TCC_ASM

/* a register can belong to several classes. The classes must be
   sorted from more general to more precise (see gv2() code which does
   assumptions on it). */
#define RC_INT     0x0001 /* generic integer register */
#define RC_FLOAT   0x0002 /* generic float register */
#define RC_EAX     0x0004
#define RC_ST0     0x0008 
#define RC_ECX     0x0010
#define RC_EDX     0x0020

#define RC_IRET    RC_EAX /* function return: integer register */
#define RC_LRET    RC_EDX /* function return: second integer register */
#define RC_FRET    RC_ST0 /* function return: float register */

/* pretty names for the registers */
enum {
    TREG_R0 = 0,
    TREG_R1,
    TREG_R2,
    TREG_R3,
    TREG_R4,
    TREG_R5,
    TREG_R6,
    TREG_R7
};

/* return registers for function */
#define REG_IRET TREG_R0 /* single word int return register */
#define REG_LRET TREG_R1 /* second word return register (for long long) */
#define REG_FRET TREG_R2 /* float return register */

/* defined if function parameters must be evaluated in reverse order */
#define INVERT_FUNC_PARAMS

/* defined if structures are passed as pointers. Otherwise structures
   are directly pushed on stack. */
/* #define FUNC_STRUCT_PARAM_AS_PTR */

/* pointer size, in bytes */
#define PTR_SIZE 4

/* long double size and alignment, in bytes */
#define LDOUBLE_SIZE  12
#define LDOUBLE_ALIGN 4
/* maximum alignment (for aligned attribute support) */
#define MAX_ALIGN     8

/******************************************************/
#else /* ! TARGET_DEFS_ONLY */
/******************************************************/
#include "tcc.h"

ST_FUNC void g(int c)
{
    printf("g(%d)", c);
}

ST_FUNC void o(unsigned int c)
{
    printf("o(%d:", c);
    while (c) {
        g(c);
        c = c >> 8;
    }
    printf(")");
}

ST_FUNC void gen_le16(int v)
{
    printf("gen_le16(%d:", v);
    g(v);
    g(v >> 8);
    printf(")");
}

ST_FUNC void gen_le32(int c)
{
    printf("gen_le32(%d:", c);
    g(c);
    g(c >> 8);
    g(c >> 16);
    g(c >> 24);
    printf(")");
}

/* output a symbol and patch all calls to it */
ST_FUNC void gsym_addr(int t, int a)
{
    printf("gsym_addr(%d/%d:", t, a);
    while (t) {
        unsigned char *ptr = cur_text_section->data + t;
        uint32_t n = read32le(ptr); /* next value */
        write32le(ptr, a - t - 4);
        t = n;
    }
    printf(")");
}

ST_FUNC void gsym(int t)
{
    printf("gsym(%d:", t);
    gsym_addr(t, ind);
    printf(")");
}

/* load 'r' from value 'sv' */
ST_FUNC void load(int r, SValue *sv)
{
    printf("load(r%d <- )", r);
}

/* store register 'r' in lvalue 'v' */
ST_FUNC void store(int r, SValue *v)
{
    printf("store(r%d ->)", r);
}

/* Return the number of registers needed to return the struct, or 0 if
   returning via struct pointer. */
ST_FUNC int gfunc_sret(CType *vt, int variadic, CType *ret, int *align, int *regsize)
{
    printf("gfunc_sret()");
    return 0;
}

ST_FUNC void gfunc_call(int nb_args)
{
    printf("gfunc_call(%d)", nb_args);
}

ST_FUNC void gfunc_prolog(CType *func_type)
{
    func_type->ref->type.t = (func_type->ref->type.t & (~VT_BTYPE)) | VT_INT;
    printf("gfunc_prolog()");
}

ST_FUNC void gfunc_epilog(void)
{
    printf("gfunc_epilog()");
}

ST_FUNC int gjmp(int t)
{
    printf("gjmp(%d)", t);
    return 0;
}

ST_FUNC void gjmp_addr(int a)
{
    printf("gjmp_addr(%d)", a);
}

ST_FUNC int gtst(int inv, int t)
{
    printf("gtst(%d, %d)", inv, t);
    return 0;
}

ST_FUNC void gen_opi(int op)
{
    printf("gen_opi(%d)", op);
}

ST_FUNC void gen_opf(int op)
{
    printf("gen_opf(%d)", op);
}

ST_FUNC void gen_cvt_itof(int t)
{
    printf("gen_cvt_itof(%d)", t);
}

ST_FUNC void gen_cvt_ftoi(int t)
{
    printf("gen_cvt_ftoi(%d)", t);
}

ST_FUNC void gen_cvt_ftof(int t)
{
    printf("gen_cvt_ftof(%d)", t);
}

ST_FUNC void ggoto(void)
{
    printf("ggoto()");
}

ST_FUNC void gen_vla_sp_save(int addr)
{
    printf("gen_vla_sp_save(%d)", addr);
}

ST_FUNC void gen_vla_sp_restore(int addr)
{
    printf("gen_vla_sp_restore(%d)", addr);
}

ST_FUNC void gen_vla_alloc(CType *type, int align)
{
    printf("gen_vla_alloc(%d)", align);
}

ST_FUNC void gen_expr32(ExprValue *pe)
{
    printf("gen_expr32()");
}


/* If T (a token) is of the form "%reg" returns the register
   number and type, otherwise return -1.  */
ST_FUNC int asm_parse_regvar (int t)
{
    printf("asm_parse_regvar(%d)", t);
    return 0;
}

ST_FUNC void asm_opcode(TCCState *s1, int opcode)
{
    printf("asm_opcode(%d)",opcode);
}

ST_FUNC void subst_asm_operand(CString *add_str, SValue *sv, int modifier)
{
    printf("subst_asm_operand(%d)", modifier);
}

ST_FUNC void asm_gen_code(ASMOperand *operands, int nb_operands, int nb_outputs, int is_output, uint8_t *clobber_regs, int out_reg)
{
    printf("asm_gen_code()");
}

ST_FUNC void asm_clobber(uint8_t *clobber_regs, const char *str)
{
    printf("asm_clobber()");
}

ST_FUNC void asm_compute_constraints(ASMOperand *operands, int nb_operands, int nb_outputs, const uint8_t *clobber_regs, int *pout_reg)
{
        printf("asm_compute_constraints()");
}

#endif




