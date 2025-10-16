#ifndef CONFIG_H__
#define CONFIG_H__

#include <stddef.h>
#include <stdint.h>
#include <collections.h>

#ifdef USE_UINT16
    typedef uint16_t tape_element_t;
#else
    #ifdef USE_UINT32
        typedef uint32_t tape_element_t;
    #else
        #ifdef USE_UINT64
            typedef uint64_t tape_element_t;
        #else
            typedef uint8_t tape_element_t;
        #endif
    #endif
#endif

typedef size_t code_pointer_t;
DECL_VEC(code_pointer_t);

typedef tape_element_t (*input_func_t)(void);
typedef void (*output_func_t)(tape_element_t);

#endif
