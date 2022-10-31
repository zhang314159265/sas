test_jit_sum:
	gcc -m32 -I. tests/test_jit_sum.c
	./a.out

test_jit_access_array:
	gcc -m32 -I. tests/test_jit_access_array.c
	./a.out | grep -q "jit_run ret -2"

test_jit_hello_libc:
	gcc -m32 -I. tests/test_jit_hello_libc.c
	./a.out | grep -q "call libc printf"

test_jit_hello_s:
	gcc -m32 -I. tests/test_jit_hello_s.c
	./a.out | grep -q "hello from jit_runner"

test_jit_only_ret:
	gcc -m32 -I. tests/test_jit_only_ret.c
	./a.out | grep -q "jit_run ret 3"

test: test_jit_access_array test_jit_hello_libc test_jit_hello_s test_jit_only_ret test_jit_sum
