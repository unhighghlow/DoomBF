static inline uint8_t fast_proc_rol_inst(struct revertable_stream *program_in, struct vector *program_out, struct loop_data *ld) {
        character inst;
        READ_CHAR_NOEOF(&inst, program_in);
        uint64_t count_limit = 1<<PAGE_SIZE_POWER;

        character cur;
        uint32_t count = 1;
        
        /* this loop is the most important part to loading optimization */
        register size_t pushback_pos = program_in->pushback_pos;
        register size_t length = program_in->pushback->length;
        register uint8_t *ptr = program_in->pushback->ptr;
        while (1) {
                if (pushback_pos >= length) {
                        /* this happens rarely */
                        program_in->pushback_pos = pushback_pos;
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
                        pushback_pos = program_in->pushback_pos;
                        length = program_in->pushback->length;
                        ptr = program_in->pushback->ptr;
                }
                cur = ptr[pushback_pos++];

                if (cur != inst) 
                        break;

                if (count >= count_limit)
                        break;

                count++;
        }
        program_in->pushback_pos = pushback_pos - 1;
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
