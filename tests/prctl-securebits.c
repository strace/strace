/*
 * Check decoding of prctl PR_GET_SECUREBITS/PR_SET_SECUREBITS operations.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
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
#include <linux/securebits.h>

#include "xlat.h"
#include "xlat/secbits.h"

#ifdef INJECT_RETVAL
# define INJ_STR " (INJECTED)"
#else
# define INJ_STR ""
#endif

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
main(int argc, char *argv[])
{
	prctl_marker();

#ifdef INJECT_RETVAL
	unsigned long num_skip;
	bool locked = false;

	if (argc < 2)
		error_msg_and_fail("Usage: %s NUM_SKIP", argv[0]);

	num_skip = strtoul(argv[1], NULL, 0);

	for (size_t i = 0; i < num_skip; i++) {
		if (prctl_marker() < 0)
			continue;

		locked = true;
		break;
	}

	if (!locked)
		error_msg_and_fail("Have not locked on prctl(-1, -2, -3, -4"
				   ", -5) returning non-error value");
#endif /* INJECT_RETVAL */

	static const kernel_ulong_t bits1 =
		(kernel_ulong_t) 0xdeadc0defacebeefULL;
	static const kernel_ulong_t bits2 =
		(kernel_ulong_t) 0xbadc0ded00000000ULL;
	static const kernel_ulong_t bits3 =
		(kernel_ulong_t) 0xffULL;

	prctl(PR_SET_SECUREBITS, 0);
	printf("prctl(" XLAT_KNOWN(0x1c, "PR_SET_SECUREBITS") ", 0) = %s"
	       INJ_STR "\n", errstr);

	prctl(PR_SET_SECUREBITS, bits1);
	printf("prctl(" XLAT_KNOWN(0x1c, "PR_SET_SECUREBITS") ", "
	       NABBR("%#llx") VERB(" /* ") NRAW("SECBIT_NOROOT|"
	       "SECBIT_NOROOT_LOCKED|SECBIT_NO_SETUID_FIXUP|"
	       "SECBIT_NO_SETUID_FIXUP_LOCKED|SECBIT_KEEP_CAPS_LOCKED|"
	       "SECBIT_NO_CAP_AMBIENT_RAISE|SECBIT_NO_CAP_AMBIENT_RAISE_LOCKED|"
	       "%#llx") VERB(" */") ") = %s" INJ_STR "\n",
	       XLAT_SEL((unsigned long long) bits1,
	       (unsigned long long) bits1 & ~0xffULL), errstr);

	if (bits2) {
		prctl(PR_SET_SECUREBITS, bits2);
		printf("prctl(" XLAT_KNOWN(0x1c, "PR_SET_SECUREBITS") ", %#llx"
		       NRAW(" /* SECBIT_??? */") ") = %s" INJ_STR "\n",
		       (unsigned long long) bits2, errstr);
	}

	prctl(PR_SET_SECUREBITS, bits3);
	printf("prctl(" XLAT_KNOWN(0x1c, "PR_SET_SECUREBITS") ", "
	       XLAT_KNOWN(0xff, "SECBIT_NOROOT|SECBIT_NOROOT_LOCKED|"
	       "SECBIT_NO_SETUID_FIXUP|SECBIT_NO_SETUID_FIXUP_LOCKED|"
	       "SECBIT_KEEP_CAPS|SECBIT_KEEP_CAPS_LOCKED|"
	       "SECBIT_NO_CAP_AMBIENT_RAISE|SECBIT_NO_CAP_AMBIENT_RAISE_LOCKED")
	       ") = %s" INJ_STR "\n", errstr);

	long rc = prctl(PR_GET_SECUREBITS, bits1);
	printf("prctl(" XLAT_KNOWN(0x1b, "PR_GET_SECUREBITS") ") = ");
	if (rc > 0) {
		printf("%#lx", rc);
		if ((rc & 0xff) && !XLAT_RAW) {
			printf(" (");
			printflags(secbits, rc, NULL);
			printf(")");
		}
		puts(INJ_STR);
	} else {
		printf("%s" INJ_STR "\n", errstr);
	}

	puts("+++ exited with 0 +++");
	return 0;
}
