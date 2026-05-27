static inline uint8_t CMD_cmd(uint8_t *inst) {
        return inst[0];
}

static inline uint8_t CMD_simple_arg(uint8_t *inst) {
        return inst[1];
}

static inline uint16_t CMD_rol_arg(uint8_t *inst) {
        return ((uint16_t)inst[1])+1;
}

static inline uint64_t CMD_rol_wide(uint8_t *inst) {
        return ntohl((*(uint32_t*)&inst[0]))&0x00ffffff;
}

// copy: 0 1 2 3 4 5 6 7 8
//       ^ v o o o o o o o
static inline int64_t CMD_copy_offset(uint8_t *inst) {
        uint64_t raw = ntohll((*(uint64_t*)&inst[0]))&0x0000ffffffffffff;
        if (raw&(1ULL<<47)) {
                raw = ~raw;
                raw &= 0x0000ffffffffffff;
                raw += 1;
                raw = -(int64_t)raw;
        }
        return raw;
}

static inline int8_t CMD_copy_val(uint8_t *inst) {
        return inst[1];
}

static inline uint64_t CMD_wide_arg(uint8_t *inst) {
        return ntohll((*(uint64_t*)&inst[0]))&0x00ffffffffffffff;
}
