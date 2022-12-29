CFLAGS=-mpreferred-stack-boundary=2 -fno-pic -fno-stack-protector -fno-inline-small-functions -fno-asynchronous-unwind-tables -DNDEBUG
LDFLAGS=-lm -lreadline

factoring sum calc24 harmonic_number mersenne_number primesieve nqueen:
	gcc -m32 $@.c -S -o /tmp/$@.s $(CFLAGS)
	gcc -m32 /tmp/$@.s -c -o /tmp/$@.o
	objdump -dr /tmp/$@.o > /tmp/$@.asm
	gcc -m32 /tmp/$@.o $(LDFLAGS)
	./a.out
	objdump -d ./a.out > /tmp/$@.fullasm

hello-libc:
	gcc -m32 -c hello-libc.s -static -o /tmp/hello-libc.o
	objdump -dr /tmp/hello-libc.o > /tmp/hello-libc.asm
	gcc -m32 -static /tmp/hello-libc.o
	objdump -d ./a.out > /tmp/hello-libc.fullasm
	./a.out

hello:
	as hello.s -o hello.o
	ld hello.o
	./a.out
