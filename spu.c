/*
 * PowerPC SPU-specific syscalls decoders.
 *
 * Copyright (c) 2018 The strace developers.
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

#if defined POWERPC || defined POWERPC64

/* defined in arch/powerpc/include/asm/spu.h */
#include "xlat/spu_create_flags.h"
#include "xlat/spu_run_events.h"
#include "xlat/spu_run_status_flags.h"

SYS_FUNC(spu_create)
{
	/* pathname */
	printpath(tcp, tcp->u_arg[0]);

	/* flags */
	tprints(", ");
	printflags_ex(tcp->u_arg[1], "SPU_CREATE_???", XLAT_STYLE_VERBOSE,
		      spu_create_flags, NULL);

	/* mode */
	tprints(", ");
	print_numeric_umode_t(tcp->u_arg[2]);

	/* neighbor_fd */
	if (tcp->u_arg[1] & SPU_CREATE_AFFINITY_SPU) {
		tprints(", ");
		printfd(tcp, tcp->u_arg[3]);
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
					<< SPU_STOP_STATUS_SHIFT;
	};

	static char outstr[spu_run_status_flags->flags_strsz
			   + sizeof("|0x3fff<<SPU_STOP_STATUS_SHIFT|")
			   + sizeof(status) * 2 + 2];

	const unsigned long st = status & spu_run_status_flags->flags_mask;
	const unsigned long stop = 0;
	const char *flags;
	char *p = outstr;

	/* As this is intended for auxstr, we don't have to support xlat
	 * styles */
	flags = sprintflags_ex("", spu_run_status_flags, status, "",
			       XLAT_STYLE_ABBREV);

	if (!flags && !(status - st))
		return NULL;

	if (flags)
		p = xappendstr(outstr, p, "%s|", flags);

	if (st & SPU_STATUS_STOPPED_BY_STOP) {
		stop = status & SPU_STOP_MASK_SHIFTED;
		p = xappendstr(outstr, p, "|%#x<<SPU_STOP_STATUS_SHIFT",
			       stop >> SPU_STOP_STATUS_SHIFT);
	}

	if (status - st - stop)
		p = xappendstr(outstr, p, "|%#x", status - st - stop);

	return outstr;
}

SYS_FUNC(spu_run)
{
	uint32_t val;

	if (entring(tcp)) {
		/* fd */
		printfd(tcp, tcp->u_arg[0]);

		/* [in, out] uint32_t *npc */
		tprints(", ");
		if (!umove_or_printaddr(tcp, tcp->u_arg[1], &val)) {
			tprintf("[%#x", val);
			set_tcb_priv_ulong(tcp, 1);
		}

		return 0;
	}

	if (get_tcb_priv_ulong(tcp)) {
		if (!umove(tcp, tcp->u_arg[1], &val))
			tprintf("->%#x]", val);
		else
			tprints("->???]");
	}

	/* [out] uint32_t *event */
	tprints(", ");
	if (!umove_or_printaddr(tcp, tcp->u_arg[1], &val)) {
		tprints("[");
		printflags();
		printflags_ex(val, "SPE_EVENT_???", XLAT_STYLE_VERBOSE,
			      spu_run_events, NULL);
		tprints("]");
	}

	if (!syserror(tcp))
		tcp->auxstr = sprint_spu_status(tcp->u_rval);

	return RVAL_STR;
}

#endif /* POWERPC || POWERPC64 */
