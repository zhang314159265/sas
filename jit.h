#pragma once

#include <sys/mman.h>

void jit_make_exec(struct str* bin_code) {
	void *fnaddr = bin_code->buf;

	// mprotect requries the address to be aligned on page boundary
	uint32_t mem_addr = (uint32_t) fnaddr;
	int mem_len = bin_code->len;
	if (mem_addr & 0xFFF) {
		mem_len += (mem_addr & 0xFFF);
		mem_addr = (mem_addr & ~0xFFF);
	}
	int status = mprotect((void*) mem_addr, mem_len, PROT_READ | PROT_WRITE | PROT_EXEC);
	if (status != 0) {
		perror("mprotect fail");
		assert(false && "mprotect fail");
	}
}

void jit_run(struct str* bin_code) {
  jit_make_exec(bin_code);
	int retval = ((int(*)()) bin_code->buf)();
	printf("jit_run ret %d\n", retval);
}
