test_jit_harmonic_number:
	gcc -m32 -I. tests/$@.c -lm
	./a.out | grep -q "PASS"

test_jit_mersenne_number:
	gcc -m32 -I. tests/$@.c
	./a.out | grep -q "The first non prime mersene number with prime power is: 2\*\*11 - 1"

test_jit_primesieve:
	gcc -m32 -I. tests/test_jit_primesieve.c
	./a.out | grep -q "Found 25 primes less than 100"

test_jit_nqueen:
	gcc -m32 -I. tests/test_jit_nqueen.c
	./a.out | grep -q "8 queen solution: 92"

test_jit_factoring:
	gcc -m32 -I. tests/test_jit_factoring.c
	./a.out | grep -q "Factoring 2047 into: 23 89"

test_jit_sum:
	gcc -m32 -I. tests/test_jit_sum.c
	./a.out | grep -q "sum is 5050"

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
