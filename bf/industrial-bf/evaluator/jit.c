static jit_state_t *_jit;

typedef int (*pifi)(uint8_t*);


void jit_run(uint8_t program[], CELL tape[]) {
  jit_node_t  *tape_in;
  pifi         incr;
  uint64_t     pc = 0;
  uint8_t     *inst;

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
  
  /* V1 -- data pointer */

  while (*(inst = &program[pc])) {
          switch (CMD_cmd(inst)) {
                  case '+':
                          jit_ldr(JIT_R0, JIT_V1);
                          jit_addi(JIT_R0, JIT_R0, CMD_rol_arg(inst));
                          jit_str(JIT_V1, JIT_R0);
                          pc+=2;
                          break;
                  case '-':
                          jit_ldr(JIT_R0, JIT_V1);
                          jit_subi(JIT_R0, JIT_R0, CMD_rol_arg(inst));
                          jit_str(JIT_V1, JIT_R0);
                          pc+=2;
                          break;
                  case '>':
                          jit_addi(JIT_V1, JIT_V1, CMD_rol_arg(inst));
                          pc+=2;
                          break;
                  case '<':
                          jit_subi(JIT_V1, JIT_V1, CMD_rol_arg(inst));
                          pc+=2;
                          break;
                  case '[':
                          if (sp == 0xfff) {
                                  printf("error: stack overflow\n");
                                  exit(1);
                          }
                          jit_ldr(JIT_R0, JIT_V1);
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
                          jit_ldr(JIT_R0, JIT_V1);
                          jump = jit_bnei(JIT_R0, 0);
                          jit_patch_at(jump, forward_stack[sp]);

                          jit_link(backward_stack[sp]);
                  case '.':
                          jit_ldr(JIT_R0, JIT_V1);
                          jit_prepare();
                          jit_pushargi((jit_word_t)"%c(%d)");
                          jit_ellipsis();
                          jit_pushargr(JIT_R0);
                          jit_pushargr(JIT_R0);
                          jit_finishi(printf);
                          jit_retr(JIT_R0);
                          pc+=1;
                          break;
          }
  }

  incr = jit_emit();
  jit_clear_state();
  jit_disassemble();

  /* call the generated code, passing 5 as an argument */
  printf("out: %d\n", incr(tape));

  jit_destroy_state();
  finish_jit();
}
