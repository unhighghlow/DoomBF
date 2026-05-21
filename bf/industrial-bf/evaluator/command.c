static inline uint8_t CMD_cmd(uint8_t *inst) {
        return inst[0];
}

static inline uint8_t CMD_rol_arg(uint8_t *inst) {
        return inst[1];
}

static inline ROLLING_TYPE CMD_rol_arg_big(uint8_t *inst) {
        return *((ROLLING_TYPE *)(inst + 1));
}

static inline int16_t CMD_copy_offset(uint8_t *inst) {
        return _ntoh_custom(*(ROLLING_TYPE*)&inst[1], ROLLING_SIZE);
}

static inline int8_t CMD_copy_val(uint8_t *inst) {
        return inst[1 + ROLLING_SIZE];
}

static inline uint64_t CMD_wide_arg(uint8_t *inst) {
        return ntohll((*(uint64_t*)&inst[0]))&0x00ffffffffffffff;
}
