EXTRA := 
ifneq ($(origin IBF_AVX2),undefined)
	EXTRA += -mavx2
endif

ifneq ($(origin IBF_JIT),undefined)
	EXTRA += -llightning
endif

ibf: evaluator/
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
