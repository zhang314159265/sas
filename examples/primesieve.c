#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(void) {
  #define N 100
  uint8_t is_prime[N];
  for (int i = 0; i < N; ++i) {
    is_prime[i] = 1;
  }
  is_prime[0] = 0;
  is_prime[1] = 0;
  for (int i = 2; i <= (int) sqrt(N); ++i) {
    if (is_prime[i]) {
      for (int j = i * i; j < N; j += i) {
        is_prime[j] = 0;
      }
    }
  }
  int cnt = 0;
  for (int i = 0; i < N; ++i) {
    cnt += is_prime[i];
  }
  printf("Found %d primes less than %d\n", cnt, N);
  for (int i = 0; i < N; ++i) {
    if (is_prime[i]) {
      printf(" %d", i);
    }
  }
  printf("\n");
  printf("bye\n");
  return 0;
}
