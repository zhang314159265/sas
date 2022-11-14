CFLAGS="-mpreferred-stack-boundary=2"

nqueen:
	gcc -m32 nqueen.c -fno-pic -c -o /tmp/nqueen.o $(CFLAGS) -DNDEBUG
	objdump -dr /tmp/nqueen.o > /tmp/nqueen.asm
	gcc -m32 /tmp/nqueen.o
	./a.out
	objdump -d ./a.out > /tmp/nqueen.fullasm

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
