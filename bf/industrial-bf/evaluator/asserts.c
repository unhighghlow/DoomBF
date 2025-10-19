signed char parse_digit(char digit) {
	signed char out;
	if (digit >= '0' && digit <= '9') {
		out = digit-'0';
	} else if (digit >= 'a' && digit <= 'f'){
		out = digit-'a'+0xa;
	} else {
		out = -1;
	}
	return out;
}

#define ASSERT_READ_EXPECTED(assert_expected, pc, program) { \
	signed char digit; \
	assert_expected = 0; \
	while (1) { \
		inst.raw = program[++pc]; \
		digit = parse_digit(inst.d.cmd); \
		if (digit != -1) { \
			assert_expected *= 16; \
			assert_expected += digit; \
		} else { \
			pc--; \
			break; \
		} \
	} \
}
