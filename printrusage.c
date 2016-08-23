/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
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

#include "defs.h"
#include <sys/resource.h>

#include DEF_MPERS_TYPE(rusage_t)

typedef struct rusage rusage_t;

#include MPERS_DEFS

MPERS_PRINTER_DECL(void, printrusage, struct tcb *tcp, long addr)
{
	rusage_t ru;

	if (umove_or_printaddr(tcp, addr, &ru))
		return;

	tprintf("{ru_utime={%llu, %llu}, ru_stime={%llu, %llu}, ",
		zero_extend_signed_to_ull(ru.ru_utime.tv_sec),
		zero_extend_signed_to_ull(ru.ru_utime.tv_usec),
		zero_extend_signed_to_ull(ru.ru_stime.tv_sec),
		zero_extend_signed_to_ull(ru.ru_stime.tv_usec));
	if (abbrev(tcp))
		tprints("...}");
	else {
		tprintf("ru_maxrss=%llu, ", zero_extend_signed_to_ull(ru.ru_maxrss));
		tprintf("ru_ixrss=%llu, ", zero_extend_signed_to_ull(ru.ru_ixrss));
		tprintf("ru_idrss=%llu, ", zero_extend_signed_to_ull(ru.ru_idrss));
		tprintf("ru_isrss=%llu, ", zero_extend_signed_to_ull(ru.ru_isrss));
		tprintf("ru_minflt=%llu, ", zero_extend_signed_to_ull(ru.ru_minflt));
		tprintf("ru_majflt=%llu, ", zero_extend_signed_to_ull(ru.ru_majflt));
		tprintf("ru_nswap=%llu, ", zero_extend_signed_to_ull(ru.ru_nswap));
		tprintf("ru_inblock=%llu, ", zero_extend_signed_to_ull(ru.ru_inblock));
		tprintf("ru_oublock=%llu, ", zero_extend_signed_to_ull(ru.ru_oublock));
		tprintf("ru_msgsnd=%llu, ", zero_extend_signed_to_ull(ru.ru_msgsnd));
		tprintf("ru_msgrcv=%llu, ", zero_extend_signed_to_ull(ru.ru_msgrcv));
		tprintf("ru_nsignals=%llu, ", zero_extend_signed_to_ull(ru.ru_nsignals));
		tprintf("ru_nvcsw=%llu, ", zero_extend_signed_to_ull(ru.ru_nvcsw));
		tprintf("ru_nivcsw=%llu}", zero_extend_signed_to_ull(ru.ru_nivcsw));
	}
}

#ifdef ALPHA
void
printrusage32(struct tcb *tcp, long addr)
{
	struct timeval32 {
		unsigned tv_sec;
		unsigned tv_usec;
	};
	struct rusage32 {
		struct timeval32 ru_utime;	/* user time used */
		struct timeval32 ru_stime;	/* system time used */
		long	ru_maxrss;		/* maximum resident set size */
		long	ru_ixrss;		/* integral shared memory size */
		long	ru_idrss;		/* integral unshared data size */
		long	ru_isrss;		/* integral unshared stack size */
		long	ru_minflt;		/* page reclaims */
		long	ru_majflt;		/* page faults */
		long	ru_nswap;		/* swaps */
		long	ru_inblock;		/* block input operations */
		long	ru_oublock;		/* block output operations */
		long	ru_msgsnd;		/* messages sent */
		long	ru_msgrcv;		/* messages received */
		long	ru_nsignals;		/* signals received */
		long	ru_nvcsw;		/* voluntary context switches */
		long	ru_nivcsw;		/* involuntary " */
	} ru;

	if (umove_or_printaddr(tcp, addr, &ru))
		return;

	tprintf("{ru_utime={%lu, %lu}, ru_stime={%lu, %lu}, ",
		(long) ru.ru_utime.tv_sec, (long) ru.ru_utime.tv_usec,
		(long) ru.ru_stime.tv_sec, (long) ru.ru_stime.tv_usec);
	if (abbrev(tcp))
		tprints("...}");
	else {
		tprintf("ru_maxrss=%lu, ", ru.ru_maxrss);
		tprintf("ru_ixrss=%lu, ", ru.ru_ixrss);
		tprintf("ru_idrss=%lu, ", ru.ru_idrss);
		tprintf("ru_isrss=%lu, ", ru.ru_isrss);
		tprintf("ru_minflt=%lu, ", ru.ru_minflt);
		tprintf("ru_majflt=%lu, ", ru.ru_majflt);
		tprintf("ru_nswap=%lu, ", ru.ru_nswap);
		tprintf("ru_inblock=%lu, ", ru.ru_inblock);
		tprintf("ru_oublock=%lu, ", ru.ru_oublock);
		tprintf("ru_msgsnd=%lu, ", ru.ru_msgsnd);
		tprintf("ru_msgrcv=%lu, ", ru.ru_msgrcv);
		tprintf("ru_nsignals=%lu, ", ru.ru_nsignals);
		tprintf("ru_nvcsw=%lu, ", ru.ru_nvcsw);
		tprintf("ru_nivcsw=%lu}", ru.ru_nivcsw);
	}
}
#endif
