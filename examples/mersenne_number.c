#include <stdio.h>
#include <assert.h>

// a dumb implementation
int is_prime(int n) {
  for (int i = 2; i < n; ++i) {
    if (n % i == 0) {
      return 0;
    }
  }
  return 1;
}

int main(void) {
  int found = -1;
  for (int i = 2; i <= 30; ++i) {
    if (is_prime(i) && !is_prime((1 << i) - 1)) {
      found = i;
      break;
    }
  }
  assert(found > 0);
  printf("The first non prime mersene number with prime power is: 2**%d - 1\n",  found);
  return 0;
}
