CFLAGS=-mpreferred-stack-boundary=2 -fno-pic -fno-stack-protector -fno-inline-small-functions -DNDEBUG
LDFLAGS=-lm -lreadline

calc24 harmonic_number mersenne_number primesieve nqueen:
	gcc -m32 $@.c -c -o /tmp/$@.o $(CFLAGS)
	objdump -dr /tmp/$@.o > /tmp/$@.asm
	gcc -m32 /tmp/$@.o $(LDFLAGS)
	./a.out
	objdump -d ./a.out > /tmp/$@.fullasm

factoring:
	gcc -m32 factoring.c -fno-pic
	./a.out
	objdump -d ./a.out > /tmp/factoring.fullasm

sum:
	gcc -m32 sum.c -fno-pic
	./a.out
	objdump -d ./a.out > /tmp/sum.fullasm

hello-libc:
	gcc -m32 -static hello-libc.s
	objdump -d ./a.out > /tmp/hello-libc.fullasm
	./a.out

hello:
	as hello.s -o hello.o
	ld hello.o
	./a.out
