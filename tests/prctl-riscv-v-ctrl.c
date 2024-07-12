/*
 * Check decoding of prctl PR_RISCV_V_SET_CONTROL and PR_RISCV_V_GET_CONTROL
 * operations.
 *
 * Copyright (c) 2021-2024 The strace developers.
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

#if !XLAT_RAW
static void
print_riscv_v_ctrl_val(kernel_ulong_t val)
{
	switch (val) {
	case PR_RISCV_V_VSTATE_CTRL_DEFAULT:
		printf("PR_RISCV_V_VSTATE_CTRL_DEFAULT");
		break;
	case PR_RISCV_V_VSTATE_CTRL_OFF:
		printf("PR_RISCV_V_VSTATE_CTRL_OFF");
		break;
	case PR_RISCV_V_VSTATE_CTRL_ON:
		printf("PR_RISCV_V_VSTATE_CTRL_ON");
		break;
	default:
		printf("%#llx", (unsigned long long) val);
	}
}

static void
print_riscv_v_ctrl(kernel_ulong_t arg)
{
	const kernel_ulong_t cur = arg & PR_RISCV_V_VSTATE_CTRL_CUR_MASK;
	const kernel_ulong_t next =
			(arg & PR_RISCV_V_VSTATE_CTRL_NEXT_MASK) >> 2;
	const kernel_ulong_t inherit = arg & PR_RISCV_V_VSTATE_CTRL_INHERIT;
	const kernel_ulong_t rest = arg & ~PR_RISCV_V_VSTATE_CTRL_MASK;

	print_riscv_v_ctrl_val(cur);
	printf("|");
	print_riscv_v_ctrl_val(next);
	printf("<<2|%sPR_RISCV_V_VSTATE_CTRL_INHERIT", inherit ? "" : "!");
	if (rest)
		printf("|%#llx", (unsigned long long) rest);
}
#endif /* !XLAT_RAW */

#ifdef INJECT_RETVAL
# define INJ_STR " (INJECTED)"
#else
# define INJ_STR ""
#endif

#if SIZEOF_KERNEL_LONG_T == 8
# define HIBITS(a) a
#else
# define HIBITS(a)
#endif


int
main(int argc, char *argv[])
{
	prctl_marker();

#ifdef INJECT_RETVAL
	unsigned long num_skip;
	long inject_retval;
	bool locked = false;

	if (argc < 3)
		error_msg_and_fail("Usage: %s NUM_SKIP INJECT_RETVAL", argv[0]);

	num_skip = strtoul(argv[1], NULL, 0);
	inject_retval = strtol(argv[2], NULL, 0);

	for (size_t i = 0; i < num_skip; i++) {
		if (prctl_marker() != inject_retval)
			continue;

		locked = true;
		break;
	}

	if (!locked)
		error_msg_and_fail("Have not locked on prctl(-1, -2, -3, -4"
				   ", -5) returning %ld", inject_retval);
#endif /* INJECT_RETVAL */

	static const struct {
		kernel_ulong_t val;
		const char *str;
	} args[] = {
		{ 0, "PR_RISCV_V_VSTATE_CTRL_DEFAULT"
		     "|PR_RISCV_V_VSTATE_CTRL_DEFAULT<<2"
		     "|!PR_RISCV_V_VSTATE_CTRL_INHERIT" },
		{ 6, "PR_RISCV_V_VSTATE_CTRL_ON"
		     "|PR_RISCV_V_VSTATE_CTRL_OFF<<2"
		     "|!PR_RISCV_V_VSTATE_CTRL_INHERIT" },
		{ 11, "0x3|PR_RISCV_V_VSTATE_CTRL_ON<<2"
		      "|!PR_RISCV_V_VSTATE_CTRL_INHERIT" },
		{ 12, "PR_RISCV_V_VSTATE_CTRL_DEFAULT|0x3<<2"
		      "|!PR_RISCV_V_VSTATE_CTRL_INHERIT" },
		{ 17, "PR_RISCV_V_VSTATE_CTRL_OFF"
		      "|PR_RISCV_V_VSTATE_CTRL_DEFAULT<<2"
		      "|PR_RISCV_V_VSTATE_CTRL_INHERIT" },
		{ 31, "0x3|0x3<<2|PR_RISCV_V_VSTATE_CTRL_INHERIT" },
		{ 32, "PR_RISCV_V_VSTATE_CTRL_DEFAULT"
		      "|PR_RISCV_V_VSTATE_CTRL_DEFAULT<<2"
		      "|!PR_RISCV_V_VSTATE_CTRL_INHERIT|0x20" },
		{ 0x57ae57e9, "PR_RISCV_V_VSTATE_CTRL_OFF"
			      "|PR_RISCV_V_VSTATE_CTRL_ON<<2"
			      "|!PR_RISCV_V_VSTATE_CTRL_INHERIT|0x57ae57e0" },
		{ (kernel_ulong_t) -1LLU, "0x3|0x3<<2"
					  "|PR_RISCV_V_VSTATE_CTRL_INHERIT"
					  "|0x" HIBITS("ffffffff") "ffffffe0" },
	};
	long rc;

	for (size_t i = 0; i < ARRAY_SIZE(args); i++) {
		rc = syscall(__NR_prctl, PR_RISCV_V_SET_CONTROL,
			     args[i].val, 1, 2, 3);
		printf("prctl(" XLAT_KNOWN(0x45, "PR_RISCV_V_SET_CONTROL") ", "
		       XLAT_FMT_LL ") = %s" INJ_STR "\n",
		       XLAT_SEL((unsigned long long) args[i].val, args[i].str),
		       sprintrc(rc));

		rc = syscall(__NR_prctl, PR_RISCV_V_GET_CONTROL, 1, 2, 3, 4);
		const char *errstr = sprintrc(rc);
		printf("prctl(" XLAT_KNOWN(0x46, "PR_RISCV_V_GET_CONTROL")
		       ") = ");
		if (rc >= 0) {
			printf("%#lx", rc);
#if !XLAT_RAW
			size_t j;

			for (j = 0; j < ARRAY_SIZE(args); j++) {
				if (args[j].val == (unsigned long) rc) {
					printf(" (%s)", args[j].str);
					break;
				}
			}

			if (j == ARRAY_SIZE(args)) {
				printf(" (");
				print_riscv_v_ctrl(rc);
				printf(")");
			}
#endif /* !XLAT_RAW */
			puts(INJ_STR);
		} else {
			printf("%s" INJ_STR "\n", errstr);
		}
	}

	puts("+++ exited with 0 +++");
	return 0;
}
