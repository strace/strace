/*
 * Check decoding of prctl PR_RISCV_V_SET_CONTROL and PR_RISCV_V_GET_CONTROL
 * operations.
 *
 * Copyright (c) 2024 Eugene Syromyatnikov <evgsyr@gmail.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/prctl.h>


int
main(void)
{
	static const struct strval_klong ctx_args[] = {
		{ 0, "PR_RISCV_CTX_SW_FENCEI_ON" },
		{ 1, "PR_RISCV_CTX_SW_FENCEI_OFF" },
		{ 2, NULL },
		{ 3, NULL },
		{ 23, NULL },
		{ 0xdeadbeef, NULL },
		{ (kernel_ulong_t) -1ULL, NULL },
	};
	static const struct strval_klong scope_args[] = {
		{ 0, "PR_RISCV_SCOPE_PER_PROCESS" },
		{ 1, "PR_RISCV_SCOPE_PER_THREAD" },
		{ 2, NULL },
		{ 3, NULL },
		{ 42, NULL },
		{ 0xcafefeed, NULL },
		{ (kernel_ulong_t) -2ULL, NULL },
	};

	prctl_marker();

	for (size_t i = 0; i < ARRAY_SIZE(ctx_args); i++) {
		for (size_t j = 0; j < ARRAY_SIZE(scope_args); j++) {
			long rc = syscall(__NR_prctl,
					  PR_RISCV_SET_ICACHE_FLUSH_CTX,
					  ctx_args[i].val, scope_args[j].val,
					  0xbadc0de, 0xbadc0ded);
			const char *errstr = sprintrc(rc);

			printf("prctl("
			       XLAT_KNOWN(0x47, "PR_RISCV_SET_ICACHE_FLUSH_CTX"));
			printf(", %s, ", sprintxlat(ctx_args[i].str,
						  ctx_args[i].val,
						  "PR_RISCV_CTX_???"));
			printf("%s) = ", sprintxlat(scope_args[j].str,
						  scope_args[j].val,
						  "PR_RISCV_SCOPE_???"));
			puts(errstr);
		}
	}

	puts("+++ exited with 0 +++");
	return 0;
}
