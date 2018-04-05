/*
 * Copyright (c) 2014 Stefan Sørensen <stefan.sorensen@spectralink.com>
 * Copyright (c) 2014-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2014-2018 The strace developers.
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

#ifdef HAVE_STRUCT_PTP_SYS_OFFSET

# include <linux/ioctl.h>
# include <linux/ptp_clock.h>

# include "print_fields.h"
# include "xlat/ptp_flags_options.h"

int
ptp_ioctl(struct tcb *const tcp, const unsigned int code,
	  const kernel_ulong_t arg)
{
	if (!verbose(tcp))
		return RVAL_DECODED;

	switch (code) {
	case PTP_EXTTS_REQUEST: {
		struct ptp_extts_request extts;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &extts))
			break;

		PRINT_FIELD_D("{", extts, index);
		PRINT_FIELD_FLAGS(", ", extts, flags, ptp_flags_options, "PTP_???");
		tprints("}");
		break;
	}

	case PTP_PEROUT_REQUEST: {
		struct ptp_perout_request perout;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &perout))
			break;

		PRINT_FIELD_D("{start={", perout.start, sec);
		PRINT_FIELD_U(", ", perout.start, nsec);
		PRINT_FIELD_D("}, period={", perout.period, sec);
		PRINT_FIELD_U(", ", perout.period, nsec);
		PRINT_FIELD_D("}, ", perout, index);
		PRINT_FIELD_FLAGS(", ", perout, flags, ptp_flags_options, "PTP_???");
		tprints("}");
		break;
	}

	case PTP_ENABLE_PPS:
		tprintf(", %" PRI_kld, arg);
		break;

	case PTP_SYS_OFFSET: {
		struct ptp_sys_offset sysoff;

		if (entering(tcp)) {
			tprints(", ");
			if (umove_or_printaddr(tcp, arg, &sysoff))
				break;

			PRINT_FIELD_U("{", sysoff, n_samples);
			return 0;
		} else {
			unsigned int n_samples, i;

			if (syserror(tcp)) {
				tprints("}");
				break;
			}

			tprints(", ");
			if (umove(tcp, arg, &sysoff) < 0) {
				tprints("???}");
				break;
			}

			tprints("ts=[");
			n_samples = sysoff.n_samples > PTP_MAX_SAMPLES ?
				PTP_MAX_SAMPLES : sysoff.n_samples;
			for (i = 0; i < 2 * n_samples + 1; ++i) {
				if (i > 0)
					tprints(", ");
				PRINT_FIELD_D("{", sysoff.ts[i], sec);
				PRINT_FIELD_U(", ", sysoff.ts[i], nsec);
				tprints("}");
			}
			if (sysoff.n_samples > PTP_MAX_SAMPLES)
				tprints(", ...");
			tprints("]}");
			break;
		}
	}
	case PTP_CLOCK_GETCAPS: {
		struct ptp_clock_caps caps;

		if (entering(tcp))
			return 0;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &caps))
			break;

		PRINT_FIELD_D("{", caps, max_adj);
		PRINT_FIELD_D(", ", caps, n_alarm);
		PRINT_FIELD_D(", ", caps, n_ext_ts);
		PRINT_FIELD_D(", ", caps, n_per_out);
		PRINT_FIELD_D(", ", caps, pps);
		tprints("}");
		break;
	}

	default:
		return RVAL_DECODED;
	}

	return RVAL_IOCTL_DECODED;
}

#endif /* HAVE_STRUCT_PTP_SYS_OFFSET */
