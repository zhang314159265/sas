#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#define USE_READLINE 1
#if USE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#define bool int
#define true 1
#define false 0

#define DEBUG 0

void usage() {
  printf("A valid input is a line containing exactly 4 non-ngative integers.\n"
    "E.g., '4 4 10 10'\n");
}

int parse_input(char *line, int nums[4]) {
  int cnt = 0;
  char *cur = line;
  char *nxt;
  while (1) {
    while (isspace(*cur)) {
      ++cur;
    }
    if (!*cur) {
      break;
    }
    if (!isdigit(*cur) || cnt >= 4) {
      return 0;
    }
    nxt = cur + 1;
    while (isdigit(*nxt)) {
      ++nxt;
    }

    // add a number
    char oldch = *nxt;
    *nxt = '\0';
    nums[cnt++] = atoi(cur);
    *nxt = oldch;

    cur = nxt;
  }
  return cnt == 4;
}

double apply_op(char op, double a, double b) {
  switch (op) {
  case '+': return a + b;
  case '-': return a - b;
  case '*': return a * b;
  case '/': return a / b; // caller should avoid divide by 0
  }
  return -1;
}

void swap(int *pa, int *pb) {
  int tmp = *pa;
  *pa = *pb;
  *pb = tmp;
}

// define some global variables on purpose to exercise thru the assembler.
// negative value in solstk represents an operation, positive value in
// solstk represents an input value.
//
// evalstk is like eager execution; solstk/strstk is like lazy execution.
int solstk[7];
int solstksize;

int bt(double* evalstk, int evalstksize, int nums[4], int nums_pos) {
  if (evalstksize == 1 && nums_pos == 4) {
    // we've explored a possible expression
    return fabs(evalstk[0] - 24) <= 1e-8;
  }

  // Like an LR parser, we now have the choise of either shift or reduce
  // 1. shift: put the next number into the evalstk
  // 2, reduce: apply one of '+-*/' to the top 2 values on the stack (if exists)
  //
  // Reduce has higher priority than shift so that for
  // a + b + c
  // '(a + b) + c' will be tried before 'a + (b + c)'.
  // If a solutin is found, we can omit the parenthesis around 'a + b'.

  // reduce
  if (evalstksize >= 2) {
    double a = evalstk[evalstksize - 2];
    double b = evalstk[evalstksize - 1];

    int nop = 4;
    if (fabs(b) <= 1e-8) {
      nop = 3; // div by 0
    }
    for (int i = 0; i < nop; ++i) {
      char op = "+-*/"[i];
      double c = apply_op(op, a, b);
      evalstk[evalstksize - 2] = c;
      solstk[solstksize++] = -op;
      if (bt(evalstk, evalstksize - 1, nums, nums_pos)) {
        return true;
      }
      --solstksize;
    }
    // recover
    evalstk[evalstksize - 2] = a;
    evalstk[evalstksize - 1] = b;
  }

  // shift
  for (int i = nums_pos; i < 4; ++i) {
    swap(&nums[nums_pos], &nums[i]);
    
    evalstk[evalstksize] = nums[nums_pos];
    solstk[solstksize++] = nums[nums_pos];
    if (bt(evalstk, evalstksize + 1, nums, nums_pos + 1)) {
      return true;
    }
    --solstksize;
    swap(&nums[nums_pos], &nums[i]);
  }
  return false;
}

enum PRECEDENCE {
  ADD_SUB = 0,
  MUL_DIV = 1,
  ITEM = 2,
};

void print_solution() {
  char* strstk[4];
  int precstk[4];
  int strstksize = 0;
  
  for (int i = 0; i < solstksize; ++i) {
    char item = solstk[i];
    char* nextstr = malloc(1024); // assume 1024 is enough
    if (item >= 0) { // a value
      sprintf(nextstr, "%d", item);
      precstk[strstksize] = ITEM;
    } else { // an operator
      char op = -item;
      int prec = (op == '+' || op == '-') ? ADD_SUB : MUL_DIV;
      bool lhs_paren = (prec > precstk[strstksize - 2]);

      // >= rather than > because of left-associative
      bool rhs_paren = (prec >= precstk[strstksize - 1]);
      sprintf(nextstr, "%s%s%s %c %s%s%s",
        lhs_paren ? "(" : "", 
        strstk[strstksize - 2],
        lhs_paren ? ")" : "",
        op,
        rhs_paren ? "(" : "",
        strstk[strstksize - 1],
        rhs_paren ? ")" : "");
      free(strstk[strstksize - 2]);
      free(strstk[strstksize - 1]);
      strstksize -= 2;
      precstk[strstksize] = prec;
    }
    strstk[strstksize++] = nextstr;
  }

  assert(strstksize == 1);
  printf("%s\n", strstk[0]);
  free(strstk[0]);
}

void solve(int nums[4]) {
  double evalstk[4];
  int evalstksize = 0;

  solstksize = 0;
  bool possible = bt(evalstk, 0, nums, 0);
  if (possible) {
    print_solution();
  } else {
    printf("IMPOSSIBLE:");
    for (int i = 0; i < 4; ++i) {
      printf(" %d", nums[i]);
    }
    printf("\n");
  }
}

bool empty_line(char *line) {
  for (int i = 0; line[i]; ++i) {
    if (!isspace(line[i])) {
      return false;
    }
  }
  return true;
}

// Check readline tutorial at: https://eli.thegreenplace.net/2016/basics-of-using-the-readline-library/
int main(void) {
  #if USE_READLINE
  // readline will malloc a buffer and return it
  char *line;
  #else
  char line[256];
  #endif
  int nums[4];
  while (1) {
    #if USE_READLINE
    if (!(line = readline("calc24> "))) {
      break;
    }
    #else
    printf("calc24> ");
    if (!fgets(line, sizeof(line), stdin)) {
      break;
    }
    #endif
    #if DEBUG
    printf("Got line: %s\n", line);
    #endif

    if (!empty_line(line)) {
      add_history(line);
    }
    if (!parse_input(line, nums)) {
      usage();
      continue;
    }
    
    #if DEBUG
    printf("Numbers parsed from user input:\n");
    for (int i = 0; i < 4; ++i) {
      printf(" %d", nums[i]);
    }
    printf("\n");
    #endif

    solve(nums);

    #if USE_READLINE
    free(line);
    #endif
  }
  printf("\nbye\n");
  return 0;
}
