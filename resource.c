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

#include "xlat/resources.h"

static const char *
sprint_rlim64(uint64_t lim)
{
	static char buf[sizeof(uint64_t)*3 + sizeof("*1024")];

	if (lim == UINT64_MAX)
		return "RLIM64_INFINITY";

	if (lim > 1024 && lim % 1024 == 0)
		sprintf(buf, "%" PRIu64 "*1024", lim / 1024);
	else
		sprintf(buf, "%" PRIu64, lim);
	return buf;
}

static void
print_rlimit64(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct rlimit_64 {
		uint64_t rlim_cur;
		uint64_t rlim_max;
	} rlim;

	if (!umove_or_printaddr(tcp, addr, &rlim)) {
		tprintf("{rlim_cur=%s,", sprint_rlim64(rlim.rlim_cur));
		tprintf(" rlim_max=%s}", sprint_rlim64(rlim.rlim_max));
	}
}

#if !defined(current_wordsize) || current_wordsize == 4

static const char *
sprint_rlim32(uint32_t lim)
{
	static char buf[sizeof(uint32_t)*3 + sizeof("*1024")];

	if (lim == UINT32_MAX)
		return "RLIM_INFINITY";

	if (lim > 1024 && lim % 1024 == 0)
		sprintf(buf, "%" PRIu32 "*1024", lim / 1024);
	else
		sprintf(buf, "%" PRIu32, lim);
	return buf;
}

static void
print_rlimit32(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct rlimit_32 {
		uint32_t rlim_cur;
		uint32_t rlim_max;
	} rlim;

	if (!umove_or_printaddr(tcp, addr, &rlim)) {
		tprintf("{rlim_cur=%s,", sprint_rlim32(rlim.rlim_cur));
		tprintf(" rlim_max=%s}", sprint_rlim32(rlim.rlim_max));
	}
}

static void
decode_rlimit(struct tcb *const tcp, const kernel_ulong_t addr)
{
	/*
	 * i386 is the only personality on X86_64 and X32
	 * with 32-bit rlim_t.
	 * When current_personality is X32, current_wordsize
	 * equals to 4 but rlim_t is 64-bit.
	 */
	if (current_klongsize == 4)
		print_rlimit32(tcp, addr);
	else
		print_rlimit64(tcp, addr);
}

#else /* defined(current_wordsize) && current_wordsize != 4 */

# define decode_rlimit print_rlimit64

#endif

SYS_FUNC(getrlimit)
{
	if (entering(tcp)) {
		printxval(resources, tcp->u_arg[0], "RLIMIT_???");
		tprints(", ");
	}
	else {
		decode_rlimit(tcp, tcp->u_arg[1]);
	}
	return 0;
}

SYS_FUNC(setrlimit)
{
	printxval(resources, tcp->u_arg[0], "RLIMIT_???");
	tprints(", ");
	decode_rlimit(tcp, tcp->u_arg[1]);

	return RVAL_DECODED;
}

SYS_FUNC(prlimit64)
{
	if (entering(tcp)) {
		tprintf("%d, ", (int) tcp->u_arg[0]);
		printxval(resources, tcp->u_arg[1], "RLIMIT_???");
		tprints(", ");
		print_rlimit64(tcp, tcp->u_arg[2]);
		tprints(", ");
	} else {
		print_rlimit64(tcp, tcp->u_arg[3]);
	}
	return 0;
}

#include "xlat/usagewho.h"

SYS_FUNC(getrusage)
{
	if (entering(tcp)) {
		printxval(usagewho, tcp->u_arg[0], "RUSAGE_???");
		tprints(", ");
	}
	else
		printrusage(tcp, tcp->u_arg[1]);
	return 0;
}

#ifdef ALPHA
SYS_FUNC(osf_getrusage)
{
	if (entering(tcp)) {
		printxval(usagewho, tcp->u_arg[0], "RUSAGE_???");
		tprints(", ");
	}
	else
		printrusage32(tcp, tcp->u_arg[1]);
	return 0;
}
#endif /* ALPHA */

#include "xlat/priorities.h"

SYS_FUNC(getpriority)
{
	printxval(priorities, tcp->u_arg[0], "PRIO_???");
	tprintf(", %d", (int) tcp->u_arg[1]);

	return RVAL_DECODED;
}

SYS_FUNC(setpriority)
{
	printxval(priorities, tcp->u_arg[0], "PRIO_???");
	tprintf(", %d, %d", (int) tcp->u_arg[1], (int) tcp->u_arg[2]);

	return RVAL_DECODED;
}
