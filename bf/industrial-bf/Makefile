ibf: src/
	${CC} src/main.c -g -O3 -o ibf ${CCFLAGS}

bfm: src/
	${CC} src/macros.c -g -O3 -o bfm ${CCFLAGS}

bld: src/
	${CC} src/loader.c -g -O3 -o bld ${CCFLAGS}

debug: src/
	${CC} src/main.c -DDEBUG -g -o ibf ${CCFLAGS}

run: ibf
	./ibf

.PHONY: run debug
