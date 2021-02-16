/*
 * This file is part of adjtimex strace test.
 *
 * Copyright (c) 2015-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_adjtimex

# include <stddef.h>
# include <stdio.h>
# include <stdint.h>
# include <string.h>
# include <sys/timex.h>
# include <unistd.h>

# include "kernel_old_timex.h"

# include "xlat.h"
# include "xlat/adjtimex_state.h"
# include "xlat/adjtimex_status.h"

static const char *errstr;

static long
k_adjtimex(void *const buf)
{
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const kernel_ulong_t arg1 = (uintptr_t) buf;
	const long rc = syscall(__NR_adjtimex, arg1, bad, bad, bad, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	int state = k_adjtimex(NULL);
	printf("adjtimex(NULL) = %s\n", errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(kernel_old_timex_t, tx);
	memset(tx, 0, sizeof(*tx));

	state = k_adjtimex(tx);
	if (state < 0)
		perror_msg_and_skip("adjtimex");

	printf("adjtimex({modes=0, offset=%jd, freq=%jd, maxerror=%jd"
	       ", esterror=%jd, status=",
	       (intmax_t) tx->offset,
	       (intmax_t) tx->freq,
	       (intmax_t) tx->maxerror,
	       (intmax_t) tx->esterror);
	if (tx->status)
		printflags(adjtimex_status, (unsigned int) tx->status, NULL);
	else
		putchar('0');
	printf(", constant=%jd, precision=%jd"
	       ", tolerance=%jd, time={tv_sec=%lld, tv_usec=%llu}, tick=%jd"
	       ", ppsfreq=%jd, jitter=%jd, shift=%d, stabil=%jd, jitcnt=%jd"
	       ", calcnt=%jd, errcnt=%jd, stbcnt=%jd, tai=%d"
	       "}) = %d (",
	       (intmax_t) tx->constant,
	       (intmax_t) tx->precision,
	       (intmax_t) tx->tolerance,
	       (long long) tx->time.tv_sec,
	       zero_extend_signed_to_ull(tx->time.tv_usec),
	       (intmax_t) tx->tick,
	       (intmax_t) tx->ppsfreq,
	       (intmax_t) tx->jitter,
	       tx->shift,
	       (intmax_t) tx->stabil,
	       (intmax_t) tx->jitcnt,
	       (intmax_t) tx->calcnt,
	       (intmax_t) tx->errcnt,
	       (intmax_t) tx->stbcnt,
	       tx->tai,
	       state);
	printxval(adjtimex_state, (unsigned int) state, NULL);
	puts(")");

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_adjtimex")

#endif
