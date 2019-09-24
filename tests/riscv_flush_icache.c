/*
 * Check decoding of riscv_flush_icache syscall.
 *
 * Copyright (c) 2018-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"


#include "scno.h"

#ifdef __NR_riscv_flush_icache

# include <stdint.h>
# include <stdio.h>
# include <unistd.h>

int main(void)
{
	static struct {
		kernel_ulong_t addr;
		const char *str;
	} addrs[] = {
		{ (kernel_ulong_t) (uintptr_t) ARG_STR(NULL) },
		{ (kernel_ulong_t) 0xbadc0deddeadf157ULL,
			sizeof(kernel_ulong_t) == 8 ? "0xbadc0deddeadf157" :
			"0xdeadf157" },
	};
	static struct {
		kernel_ulong_t val;
		const char *str;
	} flags[] = {
		{ ARG_STR(0) },
		{ 1, "SYS_RISCV_FLUSH_ICACHE_LOCAL" },
		{ (kernel_ulong_t) 0xfacefeedfffffffeULL,
			sizeof(kernel_ulong_t) == 8 ?
			"0xfacefeedfffffffe /* SYS_RISCV_FLUSH_ICACHE_??? */" :
			"0xfffffffe /* SYS_RISCV_FLUSH_ICACHE_??? */" },
		{ (kernel_ulong_t) 0xfacefeedffffffffULL,
			sizeof(kernel_ulong_t) == 8 ?
			"SYS_RISCV_FLUSH_ICACHE_LOCAL|0xfacefeedfffffffe" :
			"SYS_RISCV_FLUSH_ICACHE_LOCAL|0xfffffffe" },
	};

	for (size_t i = 0; i < ARRAY_SIZE(addrs); i++) {
		for (size_t j = 0; j < ARRAY_SIZE(addrs); j++) {
			for (size_t k = 0; k < ARRAY_SIZE(flags); k++) {
				long rc = syscall(__NR_riscv_flush_icache,
						  addrs[i].addr,
						  addrs[j].addr,
						  flags[k].val);

				printf("riscv_flush_icache(%s, %s, %s) = %s\n",
				       addrs[i].str, addrs[j].str, flags[k].str,
				       sprintrc(rc));
			}
		}
	}

	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_riscv_flush_icache");

#endif
