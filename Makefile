all:
	gcc -g -m32 sas.c -o sas
	./sas examples/hello-libc.s /tmp/my-hello-libc.o
