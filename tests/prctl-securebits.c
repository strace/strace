/*
 * Check decoding of prctl PR_GET_SECUREBITS/PR_SET_SECUREBITS operations.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include <linux/prctl.h>

#if defined __NR_prctl && defined PR_GET_SECUREBITS && defined PR_SET_SECUREBITS

# include <stdio.h>
# include <unistd.h>

# include "xlat.h"
# include "xlat/secbits.h"

static const char *errstr;

static long
prctl(kernel_ulong_t arg1, kernel_ulong_t arg2)
{
	static const kernel_ulong_t bogus_arg =
		(kernel_ulong_t) 0xdeadbeefbadc0dedULL;
	long rc = syscall(__NR_prctl, arg1, arg2, bogus_arg);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	static const kernel_ulong_t bits1 =
		(kernel_ulong_t) 0xdeadc0defacebeefULL;
	static const kernel_ulong_t bits2 =
		(kernel_ulong_t) 0xbadc0ded00000000ULL;
	static const kernel_ulong_t bits3 =
		(kernel_ulong_t) 0xffULL;

	prctl(PR_SET_SECUREBITS, 0);
	printf("prctl(PR_SET_SECUREBITS, 0) = %s\n", errstr);

	prctl(PR_SET_SECUREBITS, bits1);
	printf("prctl(PR_SET_SECUREBITS, SECBIT_NOROOT|SECBIT_NOROOT_LOCKED|"
	       "SECBIT_NO_SETUID_FIXUP|SECBIT_NO_SETUID_FIXUP_LOCKED|"
	       "SECBIT_KEEP_CAPS_LOCKED|SECBIT_NO_CAP_AMBIENT_RAISE|"
	       "SECBIT_NO_CAP_AMBIENT_RAISE_LOCKED|%#llx) = %s\n",
	       (unsigned long long) bits1 & ~0xffULL, errstr);

	if (bits2) {
		prctl(PR_SET_SECUREBITS, bits2);
		printf("prctl(PR_SET_SECUREBITS, %#llx /* SECBIT_??? */)"
		       " = %s\n", (unsigned long long) bits2, errstr);
	}

	prctl(PR_SET_SECUREBITS, bits3);
	printf("prctl(PR_SET_SECUREBITS, SECBIT_NOROOT|SECBIT_NOROOT_LOCKED|"
	       "SECBIT_NO_SETUID_FIXUP|SECBIT_NO_SETUID_FIXUP_LOCKED|"
	       "SECBIT_KEEP_CAPS|SECBIT_KEEP_CAPS_LOCKED|"
	       "SECBIT_NO_CAP_AMBIENT_RAISE|SECBIT_NO_CAP_AMBIENT_RAISE_LOCKED)"
	       " = %s\n", errstr);

	long rc = prctl(PR_GET_SECUREBITS, bits1);
	printf("prctl(PR_GET_SECUREBITS) = %s", errstr);
	if (rc > 0) {
		printf(" (");
		printflags(secbits, rc, NULL);
		printf(")");
	}

	puts("");

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_prctl && PR_GET_SECUREBITS && PR_SET_SECUREBITS")

#endif
