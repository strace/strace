/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <sys/resource.h>

#include "xstring.h"

#include "xlat/resources.h"

static void
print_rlim64_t(uint64_t lim) {
	const char *str = NULL;

	if (lim == UINT64_MAX)
		str = "RLIM64_INFINITY";
	else if (lim > 1024 && lim % 1024 == 0) {
		static char buf[sizeof(lim) * 3 + sizeof("*1024")];

		xsprintf(buf, "%" PRIu64 "*1024", lim / 1024);
		str = buf;
	}

	if (!str || xlat_verbose(xlat_verbosity) != XLAT_STYLE_ABBREV)
		PRINT_VAL_U(lim);

	if (!str || xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW)
		return;

	(xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE
                ? tprints_comment : tprints_string)(str);
}

static void
print_rlimit64(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct rlimit_64 {
		uint64_t rlim_cur;
		uint64_t rlim_max;
	} rlim;

	if (!umove_or_printaddr(tcp, addr, &rlim)) {
		tprint_struct_begin();
		PRINT_FIELD_OBJ_VAL(rlim, rlim_cur, print_rlim64_t);
		tprint_struct_next();
		PRINT_FIELD_OBJ_VAL(rlim, rlim_max, print_rlim64_t);
		tprint_struct_end();
	}
}

#if !defined(current_wordsize) || current_wordsize == 4

static void
print_rlim32_t(uint32_t lim) {
	const char *str = NULL;

	if (lim == UINT32_MAX)
		str = "RLIM_INFINITY";
	else if (lim > 1024 && lim % 1024 == 0) {
		static char buf[sizeof(lim) * 3 + sizeof("*1024")];

		xsprintf(buf, "%" PRIu32 "*1024", lim / 1024);
		str = buf;
	}

	if (!str || xlat_verbose(xlat_verbosity) != XLAT_STYLE_ABBREV)
		PRINT_VAL_U(lim);

	if (!str || xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW)
		return;

	(xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE
                ? tprints_comment : tprints_string)(str);
}

static void
print_rlimit32(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct rlimit_32 {
		uint32_t rlim_cur;
		uint32_t rlim_max;
	} rlim;

	if (!umove_or_printaddr(tcp, addr, &rlim)) {
		tprint_struct_begin();
		PRINT_FIELD_OBJ_VAL(rlim, rlim_cur, print_rlim32_t);
		tprint_struct_next();
		PRINT_FIELD_OBJ_VAL(rlim, rlim_max, print_rlim32_t);
		tprint_struct_end();
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
		/* resource */
		printxval(resources, tcp->u_arg[0], "RLIMIT_???");
		tprint_arg_next();
	} else {
		/* rlim */
		decode_rlimit(tcp, tcp->u_arg[1]);
	}
	return 0;
}

SYS_FUNC(setrlimit)
{
	/* resource */
	printxval(resources, tcp->u_arg[0], "RLIMIT_???");
	tprint_arg_next();

	/* rlim */
	decode_rlimit(tcp, tcp->u_arg[1]);

	return RVAL_DECODED;
}

SYS_FUNC(prlimit64)
{
	if (entering(tcp)) {
		/* pid */
		printpid(tcp, tcp->u_arg[0], PT_TGID);
		tprint_arg_next();

		/* resource */
		printxval(resources, tcp->u_arg[1], "RLIMIT_???");
		tprint_arg_next();

		/* new_limit */
		print_rlimit64(tcp, tcp->u_arg[2]);
		tprint_arg_next();
	} else {
		/* old_limit */
		print_rlimit64(tcp, tcp->u_arg[3]);
	}
	return 0;
}

#include "xlat/usagewho.h"

SYS_FUNC(getrusage)
{
	if (entering(tcp)) {
		/* who */
		printxval(usagewho, tcp->u_arg[0], "RUSAGE_???");
		tprint_arg_next();
	} else {
		/* usage */
		printrusage(tcp, tcp->u_arg[1]);
	}
	return 0;
}

#ifdef ALPHA
SYS_FUNC(osf_getrusage)
{
	if (entering(tcp)) {
		/* who */
		printxval(usagewho, tcp->u_arg[0], "RUSAGE_???");
		tprint_arg_next();
	} else {
		/* usage */
		printrusage32(tcp, tcp->u_arg[1]);
	}
	return 0;
}
#endif /* ALPHA */

#include "xlat/priorities.h"

static void
priority_print_who(struct tcb *tcp, int which, int who)
{
	switch (which)
	{
	case PRIO_PROCESS:
		printpid(tcp, who, PT_TGID);
		break;
	case PRIO_PGRP:
		printpid(tcp, who, PT_PGID);
		break;
	default:
		PRINT_VAL_D(who);
		break;
	}
}

SYS_FUNC(getpriority)
{
	/* which */
	printxval(priorities, tcp->u_arg[0], "PRIO_???");
	tprint_arg_next();

	/* who */
	priority_print_who(tcp, tcp->u_arg[0], tcp->u_arg[1]);

	return RVAL_DECODED;
}

SYS_FUNC(setpriority)
{
	/* which */
	printxval(priorities, tcp->u_arg[0], "PRIO_???");
	tprint_arg_next();

	/* who */
	priority_print_who(tcp, tcp->u_arg[0], tcp->u_arg[1]);
	tprint_arg_next();

	/* prio */
	PRINT_VAL_D((int) tcp->u_arg[2]);

	return RVAL_DECODED;
}
