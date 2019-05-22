/*
 * This file is part of adjtimex strace test.
 *
 * Copyright (c) 2015-2019 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/timex.h>

#include "xlat.h"
#include "xlat/adjtimex_state.h"
#include "xlat/adjtimex_status.h"

int
main(void)
{
	int state = adjtimex(NULL);
	printf("adjtimex(NULL) = %s\n", sprintrc(state));

	TAIL_ALLOC_OBJECT_CONST_PTR(struct timex, tx);
	memset(tx, 0, sizeof(*tx));

	state = adjtimex(tx);
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
#ifdef HAVE_STRUCT_TIMEX_TAI
	       tx->tai,
#else
	       *(const int *)((const void *) tx + offsetofend(struct timex, stbcnt)),
#endif
	       state);
	printxval(adjtimex_state, (unsigned int) state, NULL);
	puts(")");

	puts("+++ exited with 0 +++");
	return 0;
}
