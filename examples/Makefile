CFLAGS=-mpreferred-stack-boundary=2 -fno-pic -fno-stack-protector -fno-inline-small-functions -fno-asynchronous-unwind-tables -DNDEBUG
LDFLAGS=-lm -lreadline

USE_SAS = 
NORUN =

ifeq ($(USE_SAS), 1)
ASNAME=sas
else
ASNAME=gas
endif

all: hello-libc

sum factoring calc24 harmonic_number mersenne_number primesieve nqueen:
	gcc -m32 $@.c -S -o /tmp/$@.s $(CFLAGS)
ifeq ($(USE_SAS), 1)
	../sas /tmp/$@.s /tmp/$@.$(ASNAME).o
else
	gcc -m32 /tmp/$@.s -c -o /tmp/$@.$(ASNAME).o
endif
	objdump -dr /tmp/$@.$(ASNAME).o > /tmp/$@.asm
	gcc -m32 /tmp/$@.o $(LDFLAGS)
	objdump -d ./a.out > /tmp/$@.fullasm
ifneq ($(NORUN), 1)
	./a.out
endif

hello-libc:
ifeq ($(USE_SAS), 1)
	../sas hello-libc.s /tmp/hello-libc.o 
else
	gcc -m32 -c hello-libc.s -static -o /tmp/hello-libc.o
endif
	objdump -dr /tmp/hello-libc.o > /tmp/hello-libc.asm
	gcc -m32 -static /tmp/hello-libc.o
	objdump -d ./a.out > /tmp/hello-libc.fullasm
ifneq ($(NORUN), 1)
	./a.out
endif

# Don't try this with sas for now since I don't support label expression as
# follows yet:
#   movl $(msgend - msg), %edx
hello:
	as hello.s -o hello.o
	ld hello.o
	./a.out
