char CMD_cmd(char * inst) {
        return inst[0];
}

char CMD_simple_arg(char * inst) {
        return inst[1];
}

uint16_t CMD_simple_arg_1(char * inst) {
        return ((uint16_t)inst[1])+1;
}

int16_t CMD_copy_offset(char * inst) {
        return ntohs(*(uint16_t*)&inst[1]);
}

signed char CMD_copy_val(char * inst) {
        return inst[3];
}

uint64_t CMD_wide_arg(char * inst) {
        return ntohll((*(uint64_t*)&inst[0]))&0x00ffffffffffffff;
}
