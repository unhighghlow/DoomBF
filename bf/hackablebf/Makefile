BASE_CFLAGS = -I./include

OBJECTS = src/main-notest.o \
	  src/bf.o   \
	  src/bf_test.o

hackablebf : CFLAGS = $(BASE_CFLAGS) -DNOTESTS
src/main-notest.o: src/main.c
	$(CC) $(CFLAGS) -c -o $@ $^

hackablebf: $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o hackablebf

OBJECTS_TEST = \
	  src/main.o \
	  src/bf.o   \
	  src/collection_tests.o \
	  src/bf_test.o

hackablebf-test : CFLAGS = $(BASE_CFLAGS)
hackablebf-test: $(OBJECTS_TEST)
	$(CC) $(LDFLAGS) $^ -o hackablebf-test

test: hackablebf-test
	./hackablebf-test /dev/null

.PHONY: test
