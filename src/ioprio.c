/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2014-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "xstring.h"

#include "xlat/ioprio_who.h"
#include "xlat/ioprio_class.h"

#define IOPRIO_CLASS_SHIFT	(13)
#define IOPRIO_PRIO_MASK	((1ul << IOPRIO_CLASS_SHIFT) - 1)

#define IOPRIO_PRIO_CLASS(mask)	((mask) >> IOPRIO_CLASS_SHIFT)
#define IOPRIO_PRIO_DATA(mask)	((mask) & IOPRIO_PRIO_MASK)

static const char *
sprint_ioprio(unsigned int ioprio)
{
	static char outstr[256];
	char class_buf[64];
	unsigned int class, data;

	class = IOPRIO_PRIO_CLASS(ioprio);
	data = IOPRIO_PRIO_DATA(ioprio);
	sprintxval(class_buf, sizeof(class_buf), ioprio_class, class,
		   "IOPRIO_CLASS_???");
	xsprintf(outstr, "IOPRIO_PRIO_VALUE(%s, %d)", class_buf, data);

	return outstr;
}

void
print_ioprio(unsigned int ioprio)
{
	if (xlat_verbose(xlat_verbosity) != XLAT_STYLE_ABBREV)
		PRINT_VAL_X(ioprio);

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW)
		return;

	const char *str = sprint_ioprio(ioprio);

	(xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE
		? tprints_comment : tprints_string)(str);
}

static void
ioprio_print_who(struct tcb *tcp, int which, int who)
{
	switch (which)
	{
	case IOPRIO_WHO_PROCESS:
		printpid(tcp, who, PT_TGID);
		break;
	case IOPRIO_WHO_PGRP:
		printpid(tcp, who, PT_PGID);
		break;
	default:
		PRINT_VAL_D(who);
		break;
	}
}

SYS_FUNC(ioprio_get)
{
	if (entering(tcp)) {
		/* which */
		printxval(ioprio_who, tcp->u_arg[0], "IOPRIO_WHO_???");
		tprint_arg_next();

		/* who */
		ioprio_print_who(tcp, tcp->u_arg[0], tcp->u_arg[1]);
		return 0;
	} else {
		if (syserror(tcp))
			return 0;
		if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW)
			tcp->auxstr = NULL;
		else
			tcp->auxstr = sprint_ioprio(tcp->u_rval);
		return RVAL_STR;
	}
}

SYS_FUNC(ioprio_set)
{
	/* which */
	printxval(ioprio_who, tcp->u_arg[0], "IOPRIO_WHO_???");
	tprint_arg_next();

	/* who */
	ioprio_print_who(tcp, tcp->u_arg[0], tcp->u_arg[1]);
	tprint_arg_next();

	/* ioprio */
	if (xlat_verbose(xlat_verbosity) != XLAT_STYLE_ABBREV)
		PRINT_VAL_D((int) tcp->u_arg[2]);

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW)
		return RVAL_DECODED;

	(xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE
		? tprints_comment : tprints_string)(sprint_ioprio(tcp->u_arg[2]));

	return RVAL_DECODED;
}
