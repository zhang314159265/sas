#include <stdio.h>
#include <assert.h>

int state[64]; // assume it's enough
int ans;

#define SWAP(a, b) do { \
  int tmp = a; \
  a = b; \
  b = tmp; \
} while (0)

#define ABS(v) ((v) >= 0 ? (v) : -(v))

void bt(int step, int N) {
  if (step == N) {
    ++ans;
    return;
  }
  for (int i = step; i < N; ++i) {
    SWAP(state[step], state[i]);
    int valid = 1;
    for (int j = 0; j < step; ++j) {
      if (step - j == ABS(state[step] - state[j])) {
        valid = 0;
        break;
      }
    }
    if (valid) {
      bt(step + 1, N);
    }
    SWAP(state[step], state[i]);
  }
}

int nqueen(int N) {
  for (int i = 0; i < N; ++i) {
    state[i] = i;
  }
  bt(0, N);
  return ans;
}

int main(void) {
  int N = 8;
  assert(N <= sizeof(state) / sizeof(*state));
  printf("%d queen solution: %d\n", N, nqueen(N));
  return 0;
}
