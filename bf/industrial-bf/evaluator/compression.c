static inline uint64_t comp_read(struct revertable_stream *program_in) {
        character chr;
        PEEK_CHAR(&chr, program_in);
        if (chr != '{') return 0;
        program_in->pushback_pos++;
        uint64_t val = stream_read_number(program_in);
        READ_CHAR(&chr, program_in);
        if (chr == '}')
                return val - 1;
        printf("unknown digit: %c\n", chr);
        exit(1);
}

static inline uint8_t proc_rol_inst_comp(struct revertable_stream *program_in, struct vector *program_out, struct loop_data *ld) {
        character inst;
        READ_CHAR_NOEOF(&inst, program_in);
        uint64_t limit = 256;
        if (inst == '>' || inst == '<') {
                limit = 1<<PAGE_SIZE_POWER;
        }
        uint64_t count = 1;
        uint64_t res;
        res = comp_read(program_in);
        count += res;
        while (count > 0) {
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
                if (count < limit)
                        count = 0;
                else
                        count -= limit;
        }
        return 0;
}
