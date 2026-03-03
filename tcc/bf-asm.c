/* This file doesn't do anything. The brainfuck inline assembly
 * is implemented in tccasm.c`tcc_assemble_internal`, because,
 * unlike other ASMs, brainfuck doesn't have opcodes, so a
 * special parsing procedure had to be made */

#ifdef TARGET_DEFS_ONLY

#define CONFIG_TCC_ASM
#define NB_ASM_REGS 0

ST_FUNC void g(int c);
ST_FUNC void gen_le16(int c);
ST_FUNC void gen_le32(int c);

/*************************************************************/
#else
/*************************************************************/
#define USING_GLOBALS
#include "tcc.h"

static void asm_error(void)
{
    tcc_error("The functions in bf-asm.c should never get called");
}

ST_FUNC void g(int c)
{
    asm_error();
}

ST_FUNC void gen_le16 (int i)
{
    asm_error();
}

ST_FUNC void gen_le32 (int i)
{
    asm_error();
}

ST_FUNC void gen_expr32(ExprValue *pe)
{
    asm_error();
}

ST_FUNC void asm_opcode(TCCState *s1, int opcode)
{
    asm_error();
}

ST_FUNC void subst_asm_operand(CString *add_str, SValue *sv, int modifier)
{
    asm_error();
}

/* generate prolog and epilog code for asm statement */
ST_FUNC void asm_gen_code(ASMOperand *operands, int nb_operands,
                         int nb_outputs, int is_output,
                         uint8_t *clobber_regs,
                         int out_reg)
{
}

ST_FUNC void asm_compute_constraints(ASMOperand *operands,
                                    int nb_operands, int nb_outputs,
                                    const uint8_t *clobber_regs,
                                    int *pout_reg)
{
    printf("asm_compute_constraints(nb_operands: %d, nb_outputs: %d)\n", nb_operands, nb_outputs);
    ASMOperand *op;
    int i, reg;
    char dig;
    for (i = 0; i < nb_operands; i++) {
        op = &operands[i];
        if (op->constraint[0] != 'r') goto invalid;
        if (op->constraint[2] != '\0') goto invalid;
        dig = op->constraint[1];
        if (dig < '0' || dig > '7') goto invalid;

        op->reg = dig - '0';

        continue;
invalid:
        tcc_error("constraint: '%s'", op->constraint);
    }
}

ST_FUNC void asm_clobber(uint8_t *clobber_regs, const char *str)
{
    asm_error();
}

ST_FUNC int asm_parse_regvar (int t)
{
    asm_error();
    return -1;
}

#endif
