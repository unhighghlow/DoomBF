# This is compatible with Debian/GCC multiarch and also notices
# the environment variable TARGET_ARCH.
# You can use:
#       export TARGET_ARCH=-m32
# or
#       export TARGET_ARCH=-m64
#
# GNU lightning may be V1 or V2 and is detected; note, however, that V1 does
# NOT work on a 64bit x64. (You can try TARGET_ARCH=-m32 after installing the
# gcc-multilib package with apt-get. The arch flag -mx32 will NOT work on
# Debian kernels because kernel support is disabled. You will need to add
# "syscall.x32=y" to the kernel command line to re-enabled it.
#
# Dasm only has assembler for x86, x86_64 and x86_32 others have not yet been
# written.
# It is normally a little faster that GNU Lightning.
#
# The JIT interpreters will normally get disabled if their requirements don't
# exist but can be manually disabled too.
#
# To manually disable a 'JITter' add one or more of the options below to the
# make command line.
#
# make DO_DYNASM= DO_LIBDL= DO_TCC= DO_LIGHT=
#
# If you want a static compile please use DO_STATIC=1 to also disable automatic
# use of "-ldl" as this tends to be broken.
#
# OTOH using "STATIC_TOOLS=1" leaves libc as a shared library and static links
# other libraries.
#
# The internal GNU lightning and Dynasm can also be removed by deleting the
# relevent parts of the tools directory.
#
# There are VERSION and VERSIONID strings for you to override eg:
#
#       make VERSION=v1.2.3.4 VERSIONID=Your_Name_or_ORG

# More knobs to twiddle (the makefile uses INSTALL like the cp(1) command)
TARGETFILE=tritium
INSTALL=install -p
ISTRIP=-s
INSTALLDIR=/usr/local/bin
INSTALLEXT=
DO_STATIC=
# DEFS=
# LDFLAGS=

EXEFILE=bfi.out

all: $(EXEFILE)

# for: make DEFS='$(MOREDEFS)'
# Note: This turns on -fsanitize=integer and turns off -fwrapv
#       so expect at least one message.
MOREDEFS=-Wextra -Wfloat-equal -Wundef -Wpointer-arith -Wcast-align \
	 -Wstrict-prototypes -Wstrict-overflow=5 -Wwrite-strings \
	 -Waggregate-return -Wcast-qual -Wswitch-enum -Wconversion \
	 -Wunreachable-code -fno-common -Wlarger-than=512 \
	 -Wmissing-prototypes -Wold-style-definition \
	 -fsanitize=address -fsanitize=undefined \
	 -fno-wrapv -DALLOW_INSANITY

MOREGCC=$(MOREDEFS) -Wstack-usage=512 -Wunsafe-loop-optimizations -Wold-style-declaration
MORECLANG=$(MOREDEFS) -fsanitize=integer -Wpadded

OBJECTS=bfi.o bfi.version.o bfi.ccode.o \
	clock.o taperam.o \
	opt_bfbasic.o md5.o hashmem.o opt_runner.o

CONF=-DCNF $(CONF_DYNASM) $(CONF_LIGHTNING) $(CONF_TCCLIB) $(CONF_SSLLIB) $(CONF_GMPLIB) $(CONF_LIBDL) $(STATIC_CPP)
LDLIBS=$(GNUSTK) $(LIBS_LIGHTNING) $(LIBS_TCCLIB) $(LIBS_SSLLIB) $(LIBS_GMPLIB) $(GNUDYN) $(LIBS_LIBDL) $(STATICLINK)

$(EXEFILE): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(EXEFILE) $(OBJECTS) $(LDLIBS) $(TARGET_ARCH)

clean realclean:
	-rm -f $(EXEFILE) tmp.* *.o .build-opts.*

# This is generated BEFORE the rules start, so "realclean all" would fail
ifeq ($(MAKECMDGOALS),realclean)
	-rm -f bfi.version.h
endif
ifeq ($(MAKECMDGOALS),clean)
	-rm -f bfi.version.h
endif

install: $(EXEFILE)
	$(INSTALL) $(ISTRIP) $(EXEFILE) $(INSTALLDIR)/$(TARGETFILE)$(INSTALLEXT)

bfi.dasm.o:	tmp.dasm.c bfi.tree.h bfi.dasm.h bfi.run.h
	$(CC) $(CFLAGS) $(DASMFLAGS) -I $(TOOLDIR) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ tmp.dasm.c

bfi.gnulit.o:	bfi.gnulit.c bfi.tree.h bfi.gnulit.h bfi.run.h
	$(CC) $(CFLAGS) $(TYPE_LIGHTNING) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ bfi.gnulit.c

bfi.version.o: bfi.version.h
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ bfi.version.c

bfi.o: \
    bfi.c bfi.tree.h bfi.run.h bfi.be.def bfi.ccode.h \
    clock.h ov_int.h \
    bfi.runarray.h bfi.cpp.h
bfi.bf.o: bfi.bf.c bfi.tree.h bfi.bf.h
bfi.runarray.o: bfi.runarray.c bfi.tree.h bfi.run.h bfi.runarray.h \
    clock.h bfi.execloop.h big_int.h
opt_bfbasic.o: opt_bfbasic.c bfi.tree.h
opt_runner.o: opt_runner.c bfi.tree.h bfi.run.h

clock.o: clock.c clock.h
taperam.o: taperam.c bfi.tree.h bfi.run.h
gmp.o: gmp.c gmp.h
md5.o: md5.c md5.h
hashmem.o: hashmem.c hashmem.h
