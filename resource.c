/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999-2018 The strace developers.
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

#include "xstring.h"

#include "xlat/resources.h"

/* Normally, RLIM_INFINITY is defined to ~0LU, but we have some very special
 * architectures here.
 */

#if defined ALPHA
/* arch/alpha/include/uapi/asm/resource.h */
# define STRACE_RLIM_INFINITY		0x7fffffffffffffffUL
#endif

#if defined MIPS || defined SPARC || defined SPARC64
# if SIZEOF_LONG > 4
/* arch/{mips,sparc}/include/asm/compat.h */
#  define STRACE_M32_RLIM_INFINITY	0x7fffffff
# else
/* arch/{mips,sparc}/include/uapi/asm/resource.h */
#  define STRACE_RLIM_INFINITY		0x7fffffff
# endif
#endif

/* Generic RLIM_INFINITY definition */
#ifndef STRACE_RLIM_INFINITY
# define STRACE_RLIM_INFINITY ((kernel_ulong_t) ~0ULL)
# ifdef RLIM_INFINITY
static_assert(STRACE_RLIM_INFINITY == RLIM_INFINITY,
	      "Discrepancy in RLIM_INFINITY definition");
# endif
#endif /* !STRACE_RLIM_INFINITY */

#ifndef STRACE_M32_RLIM_INFINITY
# define STRACE_M32_RLIM_INFINITY (~0U)
#endif

#define STRACE_RLIM64_INFINITY	(~0ULL)
#ifdef RLIM64_INFINITY
static_assert(STRACE_RLIM64_INFINITY == RLIM64_INFINITY,
	      "Discrepancy in RLIM64_INFINITY definition");
#endif

#define OLD_GETLIMIT_INFINITY 0x7fffffff

enum rlimit_decode_mode {
	RDM_NORMAL,		/* RLIM_INFINITY or its compat value */
	RDM_OLD_GETRLIMIT,	/* Use INT_MAX instead of RLIM_INFINITY */
	RDM_PRLIMIT64,		/* prlimit64 always uses RLIM64_INFINITY */
};

struct rlimit_64 {
	uint64_t rlim_cur;
	uint64_t rlim_max;
};

static void
print_rlim_t(uint64_t lim, enum rlimit_decode_mode mode)
{
	uint64_t inf;
	const char *inf_str;
	enum xlat_style xst;

	if (xlat_verbose(xlat_verbosity) != XLAT_STYLE_ABBREV)
		tprintf("%" PRIu64, lim);
	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW)
		return;

	switch (mode) {
	case RDM_NORMAL:
		inf = (current_personality == 1) ? STRACE_M32_RLIM_INFINITY
						 : STRACE_RLIM_INFINITY;
		inf_str = "RLIM_INFINITY";
		xst = XLAT_STYLE_VERBOSE;
		break;
	case RDM_OLD_GETRLIMIT:
		inf = OLD_GETLIMIT_INFINITY;
		inf_str = "old getrlimit() infinity";
		xst = XLAT_STYLE_VERBOSE;
		break;
	case RDM_PRLIMIT64:
		inf = STRACE_RLIM64_INFINITY;
		inf_str = "RLIM64_INFINITY";
		xst = xlat_verbosity;
		break;
	}

	if (lim == inf) {
		if (xlat_verbose(xst) == XLAT_STYLE_VERBOSE) {
			if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_ABBREV)
				tprintf("%" PRIu64, lim);
			tprints_comment(inf_str);
		} else {
			tprints(inf_str);
		}
	} else if (lim > 1024 && lim % 1024 == 0) {
		(xlat_verbose(xlat_verbosity) == XLAT_STYLE_ABBREV
			? tprintf : tprintf_comment)("%" PRIu64 "*1024",
						     lim / 1024);
	} else {
		if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_ABBREV)
			tprintf("%" PRIu64, lim);
	}
}

static void
print_rlimit(struct tcb *const tcp, const struct rlimit_64 *rlim,
	     enum rlimit_decode_mode mode)
{
	tprints("{rlim_cur=");
	print_rlim_t(rlim->rlim_cur, mode);
	tprints(", rlim_max=");
	print_rlim_t(rlim->rlim_max, mode);
	tprints("}");
}

static void
decode_rlimit(struct tcb *const tcp, const kernel_ulong_t addr,
	      enum rlimit_decode_mode mode)
{
	struct rlimit_64 rlim;

	/*
	 * i386 is the only personality on X86_64 and X32
	 * with 32-bit rlim_t.
	 * When current_personality is X32, current_wordsize
	 * equals to 4 but rlim_t is 64-bit.
	 */
	if (current_klongsize == 4 && mode != RDM_PRLIMIT64) {
		struct rlimit_32 {
			uint32_t rlim_cur;
			uint32_t rlim_max;
		} m32_rlim;

		if (umove_or_printaddr(tcp, addr, &m32_rlim))
			return;

		rlim.rlim_cur = m32_rlim.rlim_cur;
		rlim.rlim_max = m32_rlim.rlim_max;
	} else {
		if (umove_or_printaddr(tcp, addr, &rlim))
			return;
	}

	print_rlimit(tcp, &rlim, mode);
}

static int
do_getrlimit(struct tcb * const tcp, enum rlimit_decode_mode mode)
{
	if (entering(tcp)) {
		printxval(resources, tcp->u_arg[0], "RLIMIT_???");
		tprints(", ");
	} else {
		decode_rlimit(tcp, tcp->u_arg[1], mode);
	}
	return 0;
}

SYS_FUNC(getrlimit)
{
	return do_getrlimit(tcp, RDM_NORMAL);
}

SYS_FUNC(old_getrlimit)
{
	return do_getrlimit(tcp, RDM_OLD_GETRLIMIT);
}

SYS_FUNC(setrlimit)
{
	printxval(resources, tcp->u_arg[0], "RLIMIT_???");
	tprints(", ");
	decode_rlimit(tcp, tcp->u_arg[1], RDM_NORMAL);

	return RVAL_DECODED;
}

SYS_FUNC(prlimit64)
{
	if (entering(tcp)) {
		tprintf("%d, ", (int) tcp->u_arg[0]);
		printxval(resources, tcp->u_arg[1], "RLIMIT_???");
		tprints(", ");
		decode_rlimit(tcp, tcp->u_arg[2], RDM_PRLIMIT64);
		tprints(", ");
	} else {
		decode_rlimit(tcp, tcp->u_arg[3], RDM_PRLIMIT64);
	}
	return 0;
}

#include "xlat/usagewho.h"

SYS_FUNC(getrusage)
{
	if (entering(tcp)) {
		printxval(usagewho, tcp->u_arg[0], "RUSAGE_???");
		tprints(", ");
	} else
		printrusage(tcp, tcp->u_arg[1]);
	return 0;
}

#ifdef ALPHA
SYS_FUNC(osf_getrusage)
{
	if (entering(tcp)) {
		printxval(usagewho, tcp->u_arg[0], "RUSAGE_???");
		tprints(", ");
	} else
		printrusage32(tcp, tcp->u_arg[1]);
	return 0;
}
#endif /* ALPHA */

#include "xlat/priorities.h"

static void
print_priority_who(const char * prefix, int which, int who)
{
	if (which == PRIO_USER && who)
		printuid(prefix, who);

	tprintf("%s%d", prefix, who);
}

SYS_FUNC(getpriority)
{
	printxval(priorities, tcp->u_arg[0], "PRIO_???");
	print_priority_who(", ", tcp->u_arg[0], tcp->u_arg[1]);

	return RVAL_DECODED;
}

SYS_FUNC(setpriority)
{
	printxval(priorities, tcp->u_arg[0], "PRIO_???");
	print_priority_who(", ", tcp->u_arg[0], tcp->u_arg[1]);
	tprintf(", %d", (int) tcp->u_arg[2]);

	return RVAL_DECODED;
}
