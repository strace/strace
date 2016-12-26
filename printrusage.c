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

MPERS_PRINTER_DECL(void, printrusage,
		   struct tcb *const tcp, const kernel_ulong_t addr)
{
	rusage_t ru;

	if (umove_or_printaddr(tcp, addr, &ru))
		return;

	tprints("{ru_utime=");
	MPERS_FUNC_NAME(print_struct_timeval)(&ru.ru_utime);
	tprints(", ru_stime=");
	MPERS_FUNC_NAME(print_struct_timeval)(&ru.ru_stime);
	if (abbrev(tcp))
		tprints(", ...");
	else {
#define PRINT_RUSAGE_MEMBER(member) \
		tprintf(", " #member "=%llu", zero_extend_signed_to_ull(ru.member))
		PRINT_RUSAGE_MEMBER(ru_maxrss);
		PRINT_RUSAGE_MEMBER(ru_ixrss);
		PRINT_RUSAGE_MEMBER(ru_idrss);
		PRINT_RUSAGE_MEMBER(ru_isrss);
		PRINT_RUSAGE_MEMBER(ru_minflt);
		PRINT_RUSAGE_MEMBER(ru_majflt);
		PRINT_RUSAGE_MEMBER(ru_nswap);
		PRINT_RUSAGE_MEMBER(ru_inblock);
		PRINT_RUSAGE_MEMBER(ru_oublock);
		PRINT_RUSAGE_MEMBER(ru_msgsnd);
		PRINT_RUSAGE_MEMBER(ru_msgrcv);
		PRINT_RUSAGE_MEMBER(ru_nsignals);
		PRINT_RUSAGE_MEMBER(ru_nvcsw);
		PRINT_RUSAGE_MEMBER(ru_nivcsw);
#undef PRINT_RUSAGE_MEMBER
	}
	tprints("}");
}

#ifdef ALPHA
void
printrusage32(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct rusage32 {
		timeval32_t ru_utime;		/* user time used */
		timeval32_t ru_stime;		/* system time used */
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

	tprints("{ru_utime=");
	print_timeval32_t(&ru.ru_utime);
	tprints(", ru_stime=");
	print_timeval32_t(&ru.ru_stime);
	if (abbrev(tcp))
		tprints(", ...");
	else {
# define PRINT_RUSAGE_MEMBER(member) \
		tprintf(", " #member "=%lu", ru.member)
		PRINT_RUSAGE_MEMBER(ru_maxrss);
		PRINT_RUSAGE_MEMBER(ru_ixrss);
		PRINT_RUSAGE_MEMBER(ru_idrss);
		PRINT_RUSAGE_MEMBER(ru_isrss);
		PRINT_RUSAGE_MEMBER(ru_minflt);
		PRINT_RUSAGE_MEMBER(ru_majflt);
		PRINT_RUSAGE_MEMBER(ru_nswap);
		PRINT_RUSAGE_MEMBER(ru_inblock);
		PRINT_RUSAGE_MEMBER(ru_oublock);
		PRINT_RUSAGE_MEMBER(ru_msgsnd);
		PRINT_RUSAGE_MEMBER(ru_msgrcv);
		PRINT_RUSAGE_MEMBER(ru_nsignals);
		PRINT_RUSAGE_MEMBER(ru_nvcsw);
		PRINT_RUSAGE_MEMBER(ru_nivcsw);
# undef PRINT_RUSAGE_MEMBER
	}
	tprints("}");
}
#endif
