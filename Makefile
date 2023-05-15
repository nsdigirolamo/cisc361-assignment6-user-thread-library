CC = gcc
SDIR = srcs
IDIR = headers
ODIR = objs
TDIR = tests

LIBSRCS = t_lib.c
LIBOBJS += $(ODIR)/t_lib.o

TESTSRCS = $(wildcard $(TDIR)/*.c)
TESTOBJS = $(TESTSRCS:tests/%.c=objs/%.o)
TESTS = $(TESTSRCS:tests/%.c=%)

CFLAGS = -g -I $(IDIR)

.PHONY: make all tests clean $(ODIR)

make: all

all: t_lib.a tests

# Targets for the library
t_lib.a: $(ODIR)/t_lib.o
	ar rcs t_lib.a $(ODIR)/t_lib.o

$(ODIR)/t_lib.o: $(SDIR)/t_lib.c
	$(CC) $(CFLAGS) -c $(SDIR)/t_lib.c -o $(ODIR)/t_lib.o

# Targets for all tests
tests: $(TESTS)

$(TESTS): %: $(ODIR)/%.o t_lib.a
	$(CC) $(CFLAGS) $^ -o $@

$(ODIR)/%.o: $(TDIR)/%.c $(ODIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(ODIR):
	mkdir -p objs

# Target for cleaning
clean:
	rm -f t_lib.a $(TESTS) $(ODIR)/*.o