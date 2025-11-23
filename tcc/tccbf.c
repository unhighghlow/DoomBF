/*
 *  TCCBF.C - BF file output for the Tiny C Compiler
 */

#include "tcc.h"

ST_FUNC int bf_load_file(struct TCCState *s1, const char *filename, int fd)
{
    printf("\nTCCBF:%s:%d: not implemented\n", __FILE__, __LINE__);
    return -1;
}

ST_FUNC int bf_output_file(TCCState * s1, const char *filename)
{
    printf("\nTCCBF:%s:%d: not implemented\n", __FILE__, __LINE__);
    return 0;
}

ST_FUNC int bf_putimport(TCCState *s1, int dllindex, const char *name, addr_t value)
{
    printf("\nTCCBF:%s:%d: not implemented\n", __FILE__, __LINE__);
    return 0;
}

ST_FUNC SValue *bf_getimport(SValue *sv, SValue *v2)
{
    printf("\nTCCBF:%s:%d: not implemented\n", __FILE__, __LINE__);
    return NULL;
}
