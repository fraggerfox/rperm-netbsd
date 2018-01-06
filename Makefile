CC =	cc
INC =
LIB =
WARNS =	-Wall -Wno-uninitialized
CD=	cd
TESTS=	tests

all:
	make atf
	make kmod
	make test

atf:
	${CD} ${TESTS};	\
	make -f Makefile

kmod:
	make -f Makefile.kmod

test:
	${CC} ${WARNS} ${INC} -c test.c
	${CC} -o test test.o ${LIB}

clean:
	make clean_kmod
	make clean_atf
	make clean_test

clean_atf:
	${CD} ${TESTS}; \
	make -f Makefile clean; \
	rm -f Atffile

clean_kmod:
	make -f Makefile.kmod clean

clean_test:
	rm -f test.o test
