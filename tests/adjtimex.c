/*
 * This file is part of adjtimex strace test.
 *
 * Copyright (c) 2015-2017 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "tests.h"
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
	       ", calcnt=%jd, errcnt=%jd, stbcnt=%jd"
#ifdef HAVE_STRUCT_TIMEX_TAI
	       ", tai=%d"
#endif
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
#endif
	       state);
	printxval(adjtimex_state, (unsigned int) state, NULL);
	puts(")");

	puts("+++ exited with 0 +++");
	return 0;
}
