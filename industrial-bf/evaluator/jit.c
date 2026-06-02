static jit_state_t *_jit;

typedef int (*pifi)(uint8_t*);

void assert(uint8_t kind, uint64_t exp, uint64_t got, uint64_t com) {
        if (kind == 0)
                got -= (uint64_t)tape;
        if (exp == got) return;

        char *assert_name = "?";
        if (kind == 0) {
                assert_name = "location";
        }
        if (kind == 1) {
                assert_name = "value";
        }
        printf("assertion failed: %s\n", assert_name);
        if (com)
                printf("comment:  0x%016lx\n", com);
        printf("expected: 0x%016lx\n", exp);
        printf("got:      0x%016lx\n", got);
#ifdef DUMP_TAPE
        printf("dumping tape.bin...\n");
        dump_tape();
#endif
        exit(1);
}

void jit_run(uint8_t program[]) {
        jit_node_t  *tape_in;
        pifi         run;
        uint64_t     pc = 0;
        uint8_t     *inst;
        uint8_t      assert_kind = 0;
        uint64_t      assert_expected = 0;
        uint64_t      assert_comment = 0;

        uint32_t    sp = 0;
        jit_node_t  *forward_stack[0x1000];  // Filled with labels
        jit_node_t  *backward_stack[0x1000]; // Filled with forward-labels
        jit_node_t  *jump;

        init_jit("ibf");
        _jit = jit_new_state();

        inst = program;

        jit_prolog();
        tape_in = jit_arg();
        jit_getarg(JIT_V1, tape_in);
        jit_movi(JIT_V2, 0);

        /* V1 -- data pointer */
        /* V2 -- always zero */

        while (*(inst = &program[pc])) {
                switch (CMD_cmd(inst)) {
                        case '+':
                                jit_ldr_uc(JIT_R0, JIT_V1);
                                jit_addi(JIT_R0, JIT_R0, CMD_rol_arg(inst));
                                jit_str_c(JIT_V1, JIT_R0);
                                pc+=2;
                                break;
                        case '-':
                                jit_ldr_uc(JIT_R0, JIT_V1);
                                jit_subi(JIT_R0, JIT_R0, CMD_rol_arg(inst));
                                jit_str_c(JIT_V1, JIT_R0);
                                pc+=2;
                                break;
                        case '>':
                                jit_addi(JIT_V1, JIT_V1, CMD_rol_arg(inst));
                                pc+=2;
                                break;
                        case 'r':
                                jit_addi(JIT_V1, JIT_V1, CMD_wide_arg(inst));
                                pc+=8;
                                break;
                        case '<':
                                jit_subi(JIT_V1, JIT_V1, CMD_rol_arg(inst));
                                pc+=2;
                                break;
                        case 'l':
                                jit_subi(JIT_V1, JIT_V1, CMD_wide_arg(inst));
                                pc+=8;
                                break;
                        case '[':
                                if (sp == 0xfff) {
                                        printf("error: stack overflow\n");
                                        exit(1);
                                }
                                jit_ldr_uc(JIT_R0, JIT_V1);
                                jump = jit_beqi(JIT_R0, 0);
                                backward_stack[sp] = jit_forward();
                                jit_patch_at(jump, backward_stack[sp]);

                                forward_stack[sp] = jit_label();
                                sp++;
                                pc+=8;
                                break;
                        case ']':
                                if (sp == 0) {
                                        printf("error: stack underflow\n");
                                        exit(1);
                                }
                                sp--;
                                jit_ldr_uc(JIT_R0, JIT_V1);
                                jump = jit_bnei(JIT_R0, 0);
                                jit_patch_at(jump, forward_stack[sp]);

                                jit_link(backward_stack[sp]);
                                pc+=8;
                                break;
                        case '^':
                                jit_ldr_uc(JIT_R0, JIT_V1);
                                jit_ldxi_l(JIT_R1, JIT_V1, CMD_copy_offset(inst));
                                jit_muli(JIT_R0, JIT_R0, CMD_copy_val(inst));
                                jit_addr(JIT_R0, JIT_R0, JIT_R1);
                                jit_stxi_c(CMD_copy_offset(inst), JIT_V1, JIT_R0);
                                pc+=8;
                                break;
                        case '0':
                                jit_str_c(JIT_V1, JIT_V2);
                                pc+=1;
                                break;
                        case '.':
                                jit_ldr_uc(JIT_R0, JIT_V1);
                                jit_prepare();
                                jit_pushargi((jit_word_t)"%c");
                                jit_ellipsis();
                                jit_pushargr(JIT_R0);
                                jit_finishi(printf);
                                pc+=1;
                                break;
                        case ',':
                                jit_prepare();
                                jit_pushargi(STDIN_FILENO);
                                jit_pushargr(JIT_V1);
                                jit_pushargi(1);
                                jit_finishi(read);
                                pc+=1;
                                break;

                        case '@':
                                assert_kind = 0;
                                goto assert_common;
                        case '!':
                                assert_kind = 1;
assert_common:
                                assert_expected = CMD_wide_arg(inst);
                                assert_comment = CMD_assert_com(inst);

                                if (assert_kind == 1)
                                        jit_ldr_uc(JIT_R0, JIT_V1);
                                jit_prepare();
                                jit_pushargi(assert_kind);
                                jit_pushargi_l(assert_expected);
                                if (assert_kind == 0) {
                                        jit_pushargr_l(JIT_V1);
                                } else {
                                        jit_pushargr_l(JIT_R0);
                                }
                                jit_pushargi_l(assert_comment);
                                jit_finishi(assert);
                                pc+=16;
                                break;
                        default:
                                printf("unknown instruction: %c\n", CMD_cmd(inst));
                                exit(1);
                }
        }

        run = jit_emit();
        jit_clear_state();

        run(tape);

        jit_destroy_state();
        finish_jit();
}
