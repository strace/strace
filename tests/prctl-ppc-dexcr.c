/*
 * Check decoding of prctl PR_PPC_GET_DEXCR and PR_PPC_SET_DEXCR operations.
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

#include "xlat.h"
#include "xlat/pr_ppc_dexcr_ctrl_flags.h"

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

	static const struct strval_klong aspects[] = {
		{ 0, "PR_PPC_DEXCR_SBHE" },
		{ 1, "PR_PPC_DEXCR_IBRTPD" },
		{ 2, "PR_PPC_DEXCR_SRAPD" },
		{ 3, "PR_PPC_DEXCR_NPHIE" },
		{ 4, NULL },
		{ 4, NULL },
	};
	static const struct strval_klong flags[] = {
		{ 0, "" },
		{ 1, "PR_PPC_DEXCR_CTRL_EDITABLE" },
		{ 2, "PR_PPC_DEXCR_CTRL_SET" },
		{ 3, "PR_PPC_DEXCR_CTRL_EDITABLE|PR_PPC_DEXCR_CTRL_SET" },
		{ 23, "PR_PPC_DEXCR_CTRL_EDITABLE|PR_PPC_DEXCR_CTRL_SET"
		      "|PR_PPC_DEXCR_CTRL_CLEAR"
		      "|PR_PPC_DEXCR_CTRL_CLEAR_ONEXEC" },
		{ 0xe0, NULL },
		{ 0x57ae57e9, "PR_PPC_DEXCR_CTRL_EDITABLE"
			      "|PR_PPC_DEXCR_CTRL_SET_ONEXEC|0x57ae57e0" },
		{ (kernel_ulong_t) -1LLU, "PR_PPC_DEXCR_CTRL_EDITABLE"
					  "|PR_PPC_DEXCR_CTRL_SET"
					  "|PR_PPC_DEXCR_CTRL_CLEAR"
					  "|PR_PPC_DEXCR_CTRL_SET_ONEXEC"
					  "|PR_PPC_DEXCR_CTRL_CLEAR_ONEXEC"
					  "|0x" HIBITS("ffffffff") "ffffffe0" },
	};
	long rc;

	for (size_t i = 0; i < ARRAY_SIZE(aspects); i++) {
		for (size_t j = 0; j < ARRAY_SIZE(flags); j++) {
			rc = syscall(__NR_prctl, PR_PPC_SET_DEXCR,
				     aspects[i].val, flags[j].val, 1, 2);
			const char *errstr = sprintrc(rc);

			printf("prctl(" XLAT_KNOWN(0x49, "PR_PPC_SET_DEXCR")
			       ", %s",
			       sprintxlat(aspects[i].str,aspects[i].val,
					  "PR_PPC_DEXCR_???"));
			printf(", %s, 0x1, 0x2) = %s" INJ_STR "\n",
			       sprintxlat(flags[j].str && flags[j].str[0]
					  ? flags[j].str : NULL,
					  flags[j].val,
					  flags[j].str
					  ? NULL : "PR_PPC_DEXCR_CTRL_???"),
			       errstr);
		}

		for (size_t j = 0; j < 2; j++) {
			rc = syscall(__NR_prctl, PR_PPC_GET_DEXCR,
				     aspects[i].val, j, j, j);
			const char *errstr = sprintrc(rc);

			printf("prctl(" XLAT_KNOWN(0x48, "PR_PPC_GET_DEXCR")
			       ", %s, %#zx, %#zx, %#zx) = ",
			       sprintxlat(aspects[i].str,aspects[i].val,
					  "PR_PPC_DEXCR_???"), j, j, j);

			if (rc > 0) {
				printf("%#lx", rc);
#if !XLAT_RAW
				size_t k;

				for (k = 0; k < ARRAY_SIZE(flags); k++) {
					if (flags[k].val == (unsigned long) rc)
					{
						if (flags[k].str) {
							printf(" (%s)",
							       flags[k].str);
						}
						break;
					}
				}

				if (k == ARRAY_SIZE(flags)
				    && (rc & PR_PPC_DEXCR_CTRL_MASK)) {
					printf(" (");
					printflags(pr_ppc_dexcr_ctrl_flags, rc,
						   NULL);
					printf(")");
				}
#endif /* !XLAT_RAW */
				puts(INJ_STR);
			} else {
				printf("%s" INJ_STR "\n", errstr);
			}
		}
	}

	puts("+++ exited with 0 +++");
	return 0;
}
