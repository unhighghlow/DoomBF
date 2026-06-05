has_macro = $(shell $(CC) -E -dD -x c evaluator/config.h 2>/dev/null | grep -E '^\\#define[[:space:]]+$(1)\b' >/dev/null && echo yes || echo no)

EXTRA := 
ifneq ($(call has_macro,FAST_ROL),yes)
	EXTRA += -mavx2
endif

ifneq ($(call has_macro,JIT),yes)
	EXTRA += -llightning
endif

evaluator/config.h:
	cp evaluator/config.def.h evaluator/config.h

ibf: evaluator/ evaluator/config.h
	${CC} evaluator/main.c -g ${EXTRA} -O3 -o ibf ${CCFLAGS}

bld: loader/
	${CC} loader/loader.c -g -O3 -o bld ${CCFLAGS}

bpk: packer/
	${CC} packer/packer.c -g -O3 -o bpk ${CCFLAGS}

run: ibf
	./ibf

test: ibf
	./test.sh

.PHONY: run


clean:
	rm -f *.o ./ibf