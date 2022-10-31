sum:
	gcc -m32 sum.c -fno-pic
	./a.out
	objdump -d ./a.out > /tmp/sum.asm

hello-libc:
	gcc -m32 -static hello-libc.s
	objdump -d ./a.out > /tmp/hello-libc.asm
	./a.out

hello:
	as hello.s -o hello.o
	ld hello.o
	./a.out
