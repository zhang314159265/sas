all:
	gcc -g -m32 sas.c
	./a.out examples/hello-libc.s /tmp/my-hello-libc.o
