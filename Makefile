CC = gcc
SDIR = srcs
IDIR = headers
ODIR = objs
TDIR = tests

LIBOBJS += $(ODIR)/t_lib.o

TESTOBJS += $(ODIR)/test01.o
TESTOBJS += $(ODIR)/test01x.o
TESTOBJS += $(ODIR)/test03.o
TESTOBJS += $(ODIR)/test10.o

TSTSRCS += $(TDIR)/test00.c
TSTSRCS += $(TDIR)/test01.c
TSTSRCS += $(TDIR)/test01x.c
TSTSRCS += $(TDIR)/test03.c
TSTSRCS += $(TDIR)/test10.c

LIBSRCS = t_lib.c

EXECS += test00
EXECS += test01
EXECS += test01x
EXECS += test03
EXECS += test10
EXECS += 3test
EXECS += philosophers

CFLAGS = -g -I $(IDIR)

$(ODIR)/t_lib.o: $(SDIR)/t_lib.c
	${CC} ${CFLAGS} -c $(SDIR)/t_lib.c -o $(ODIR)/t_lib.o

t_lib.a: $(ODIR)/t_lib.o
	ar rcs t_lib.a $(ODIR)/t_lib.o


$(ODIR)/test00.o: $(TDIR)/test00.c
	${CC} ${CFLAGS} -c $(TDIR)/test00.c -o $(ODIR)/test00.o

test00: t_lib.a $(ODIR)/test00.o 
	${CC} ${CFLAGS} $(ODIR)/test00.o t_lib.a -o test00


$(ODIR)/test01.o: $(TDIR)/test01.c
	${CC} ${CFLAGS} -c $(TDIR)/test01.c -o $(ODIR)/test01.o

test01: t_lib.a $(ODIR)/test01.o 
	${CC} ${CFLAGS} $(ODIR)/test01.o t_lib.a -o test01


$(ODIR)/test01x.o: $(TDIR)/test01x.c
	${CC} ${CFLAGS} -c $(TDIR)/test01x.c -o $(ODIR)/test01x.o

test01x: t_lib.a $(ODIR)/test01x.o 
	${CC} ${CFLAGS} $(ODIR)/test01x.o t_lib.a -o test01x


$(ODIR)/test03.o: $(TDIR)/test03.c
	${CC} ${CFLAGS} -c $(TDIR)/test03.c -o $(ODIR)/test03.o

test03: t_lib.a $(ODIR)/test03.o 
	${CC} ${CFLAGS} $(ODIR)/test03.o t_lib.a -o test03


$(ODIR)/test10.o: $(TDIR)/test10.c
	${CC} ${CFLAGS} -c $(TDIR)/test10.c -o $(ODIR)/test10.o

test10: t_lib.a $(ODIR)/test10.o 
	${CC} ${CFLAGS} $(ODIR)/test10.o t_lib.a -o test10


$(ODIR)/3test.o: $(TDIR)/3test.c
	${CC} ${CFLAGS} -c $(TDIR)/3test.c -o $(ODIR)/3test.o

3test: t_lib.a $(ODIR)/3test.o 
	${CC} ${CFLAGS} $(ODIR)/3test.o t_lib.a -o 3test


$(ODIR)/philosophers.o: $(TDIR)/philosophers.c
	${CC} ${CFLAGS} -c $(TDIR)/philosophers.c -o $(ODIR)/philosophers.o

philosophers: t_lib.a $(ODIR)/philosophers.o 
	${CC} ${CFLAGS} $(ODIR)/philosophers.o t_lib.a -o philosophers


$(ODIR):
	mkdir -p $@

.PHONY: clean

clean:
	rm -f t_lib.a ${EXECS} ${ODIR}/*.o