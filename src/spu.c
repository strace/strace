/*
 * PowerPC SPU-specific syscalls decoders.
 *
 * Copyright (c) 2018-2022 Eugene Syromyatnikov <evgsyr@gmail.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#if defined POWERPC || defined POWERPC64

# include "xstring.h"

/* defined in arch/powerpc/include/asm/spu.h */
# include "xlat/spu_create_flags.h"
# include "xlat/spu_run_events.h"
# include "xlat/spu_run_status_flags.h"

SYS_FUNC(spu_create)
{
	kernel_ulong_t pathname = tcp->u_arg[0];
	unsigned int flags = tcp->u_arg[1];
	unsigned short mode = tcp->u_arg[2];
	int neighbor_fd = tcp->u_arg[3];

	/* pathname */
	printpath(tcp, pathname);

	/* flags */
	tprint_arg_next();
	printflags_ex(flags, "SPU_CREATE_???",
		      xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW
			? XLAT_STYLE_RAW : XLAT_STYLE_VERBOSE,
		      spu_create_flags, NULL);

	/* mode */
	tprint_arg_next();
	print_numeric_umode_t(mode);

	/* neighbor_fd */
	if (flags & SPU_CREATE_AFFINITY_SPU) {
		tprint_arg_next();
		printfd(tcp, neighbor_fd);
	}

	return RVAL_DECODED | RVAL_FD;
}

static const char *
sprint_spu_status(unsigned long status)
{
	enum {
		SPU_STOP_STATUS_SHIFT = 16,
		SPU_STOP_STATUS_MASK  = 0x3fff,

		SPU_STATUS_MASK       = (1 << SPU_STOP_STATUS_SHIFT) - 1,
		SPU_STOP_MASK_SHIFTED = SPU_STOP_STATUS_MASK
					<< SPU_STOP_STATUS_SHIFT,
	};

	static char outstr[spu_run_status_flags_flags_strsz
			   + sizeof("|0x3fff<<SPU_STOP_STATUS_SHIFT|")
			   + sizeof(status) * 2 + 2];

	const unsigned long st = status & spu_run_status_flags->flags_mask;
	unsigned long stop = 0;
	const char *flags;
	char *p = outstr;

	flags = sprintflags_ex("", spu_run_status_flags, status, '\0',
			       XLAT_STYLE_DEFAULT | SPFF_AUXSTR_MODE);

	if (!flags && !(status - st))
		return NULL;

	if (flags)
		p = xappendstr(outstr, p, "%s|", flags);

	if (st & SPU_STATUS_STOPPED_BY_STOP) {
		stop = status & SPU_STOP_MASK_SHIFTED;
		p = xappendstr(outstr, p, "|%#lx<<SPU_STOP_STATUS_SHIFT",
			       stop >> SPU_STOP_STATUS_SHIFT);
	}

	if (status - st - stop)
		p = xappendstr(outstr, p, "|%#lx", status - st - stop);

	return outstr;
}

SYS_FUNC(spu_run)
{
	uint32_t val;

	if (entering(tcp)) {
		/* fd */
		printfd(tcp, tcp->u_arg[0]);

		/* [in, out] uint32_t *npc */
		tprint_arg_next();
		if (printnum_int(tcp, tcp->u_arg[1], "%#x"))
			set_tcb_priv_ulong(tcp, 1);

		return 0;
	}

	if (!syserror(tcp) && get_tcb_priv_ulong(tcp)) {
		tprint_value_changed();
		printnum_int(tcp, tcp->u_arg[1], "%#x");
	}

	/* [out] uint32_t *event */
	tprint_arg_next();
	if (!umove_or_printaddr(tcp, tcp->u_arg[2], &val)) {
		tprint_indirect_begin();
		printflags_ex(val, "SPE_EVENT_???",
			      xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW
				? XLAT_STYLE_RAW : XLAT_STYLE_VERBOSE,
			      spu_run_events, NULL);
		tprint_indirect_end();
	}

	if (!syserror(tcp))
		tcp->auxstr = sprint_spu_status(tcp->u_rval);

	return RVAL_HEX | RVAL_STR;
}

#endif /* POWERPC || POWERPC64 */
