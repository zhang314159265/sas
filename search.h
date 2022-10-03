#pragma once

#include <string.h>

int linear_search(const char *names[], const char* needle, int len) {
	for (int i = 0; names[i]; ++i) {
		const char *cand = names[i];
		int cand_len = strlen(cand);
		if (cand_len == len) {
			if (strncmp(cand, needle, len) == 0) {
				return i;
			}
		}
	}
	return -1;	
}
