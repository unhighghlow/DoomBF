ibf: evaluator/
	${CC} evaluator/main.c -g -O3 -o ibf ${CCFLAGS}

bld: loader/
	${CC} loader/loader.c -g -O3 -o bld ${CCFLAGS}

run: ibf
	./ibf

test: ibf
	./test.sh

.PHONY: run
