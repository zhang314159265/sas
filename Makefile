all:
	gcc -m32 sas.c
	./a.out examples/hello-libc.s
