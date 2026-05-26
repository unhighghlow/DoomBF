#include <immintrin.h>

// This function was written by a clanker (AI)
// Return number of bytes from s (up to maxlen) that equal byte c.
// Requires -mavx2.
size_t run_eq_avx2(const void *s_, unsigned char c, size_t maxlen) {
    const unsigned char *s = (const unsigned char*)s_;
    size_t idx = 0;

    // Prologue: handle initial bytes until 32-byte aligned or until small input
    while (idx < maxlen && ((uintptr_t)(s + idx) & 31)) {
        if (s[idx] != c) return idx;
        idx++;
    }

    const size_t stride = 32;
    __m256i vcmp = _mm256_set1_epi8((char)c);

    while (idx + stride <= maxlen) {
        __m256i v = _mm256_loadu_si256((const __m256i*)(s + idx));
        __m256i eq = _mm256_cmpeq_epi8(v, vcmp);
        unsigned int mask = (unsigned int)_mm256_movemask_epi8(eq); // 32-bit mask, 1 = equal
        if (mask == 0xFFFFFFFFu) { // all equal in this block
            idx += stride;
            continue;
        }
        // Not all equal: we need first non-equal byte.
        // movemask bit i == 0 indicates mismatch at byte i.
        unsigned int inv = ~mask; // bits set where mismatch
        unsigned int first_bit = inv & -inv; // isolate lowest set bit
        // find index of lowest set bit:
        unsigned int bit_index = __builtin_ctz(inv); // 0..31
        return idx + bit_index;
    }

    // Tail: remaining bytes
    while (idx < maxlen) {
        if (s[idx] != c) break;
        idx++;
    }
    return idx;
}

static inline uint8_t fast_proc_rol_inst(struct revertable_stream *program_in, struct vector *program_out, struct loop_data *ld) {
        character inst;
        READ_CHAR_NOEOF(&inst, program_in);
        uint64_t count_limit = 1<<PAGE_SIZE_POWER;

        character cur;
        uint32_t count = 1;
        size_t block;
        
        /* this loop is the most important part to loading optimization */
        size_t maxlen;
        while (1) {
                if (program_in->pushback_pos >= program_in->pushback->length) {
                        if (!read_revertable_direct(&cur, program_in)) {
                                /* EOF or error */
                                if (feof(program_in->fd) || !errno) {
                                        /* EOF */
                                        break;
                                } else {
                                        perror("reading file");
                                        return 1;
                                }
                        }
                }
                if (count >= count_limit)
                        break;

                maxlen = program_in->pushback->length - program_in->pushback_pos;
                if (maxlen > (count_limit - count)) {
                        maxlen = count_limit - count;
                }

                block = run_eq_avx2(&program_in->pushback->ptr[program_in->pushback_pos], inst, maxlen);
                if (!block) {
                        break;
                }
                count += block;
                program_in->pushback_pos += block;
        }

        if (count <= 256) {
                vector_push(program_out, inst);
                vector_push(program_out, (uint8_t)count-1);
        } else {
                if (inst == '>') inst = 'r';
                if (inst == '<') inst = 'l';
                vector_push_ex(
                        program_out,
                        uint64_t,
                        count | (((int64_t)inst) << (8*7))
                );
        }
        return 0;
}
