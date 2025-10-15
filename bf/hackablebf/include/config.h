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

typedef char* code_ptr_t;
DECL_VEC(code_ptr_t);
DECL_VEC(size_t);

typedef tape_element_t (*input_func_t)(void);
typedef void (*output_func_t)(tape_element_t);
typedef tape_element_t* (*init_func_t)(void);
typedef code_ptr_t (*load_code_func_t)(void);

#endif
