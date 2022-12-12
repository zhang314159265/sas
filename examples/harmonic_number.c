/*
 * ln(n+1) <= H(n) <= 1 + ln(n)
 */

#include <stdio.h>
#include <math.h>

// #define SCALAR_TYPE float
#define SCALAR_TYPE double

SCALAR_TYPE harmonic(int n) {
  SCALAR_TYPE sum = 0;
  for (int i = 1; i <= n; ++i) {
    sum += 1.0 / i;
  }
  return sum;
}

int main(void) {
  int nfail = 0;
  for (int i = 1; i <= 100; ++i) {
    SCALAR_TYPE h = harmonic(i);
    if (h < log(i + 1) || h > 1 + log(i)) {
      ++nfail;
    }
  }
  printf("%s\n", nfail ? "FAIL" : "PASS");
  return 0;
}
