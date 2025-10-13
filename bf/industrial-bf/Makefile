ibf: evaluator/
	${CC} evaluator/main.c -g -o ibf ${CCFLAGS}

bld: loader/
	${CC} loader/loader.c -g -o bld ${CCFLAGS}

run: ibf
	./ibf

.PHONY: run
