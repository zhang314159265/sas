#include <stdio.h>

typedef unsigned int int_type;

int main(void) {
	int_type val = 2047;
	printf("Factoring %d into:", val);
	int_type left = val;
	int_type cur = 2;
	while (left != 1) {
		if (left % cur == 0) {
			printf(" %d", cur);
			left /= cur;
			// don't increment cur here to handle multiple occurance of the same factor
			continue;
		} else {
			++cur;
		}
	}
	printf("\n");
	return 0;
}
