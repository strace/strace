/*
 * Copyright (c) 2014 Stefan Sørensen <stefan.sorensen@spectralink.com>
 * Copyright (c) 2014-2015 Dmitry V. Levin <ldv@altlinux.org>
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
#include <linux/ioctl.h>
#include <linux/ptp_clock.h>

#include "xlat/ptp_flags_options.h"

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

		tprintf("{index=%d, flags=", extts.index);
		printflags(ptp_flags_options, extts.flags, "PTP_???");
		tprints("}");
		break;
	}

	case PTP_PEROUT_REQUEST: {
		struct ptp_perout_request perout;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &perout))
			break;

		tprintf("{start={%" PRId64 ", %" PRIu32 "}"
			   ", period={%" PRId64 ", %" PRIu32 "}"
			   ", index=%d, flags=",
			(int64_t)perout.start.sec, perout.start.nsec,
			(int64_t)perout.period.sec, perout.period.nsec,
			perout.index);
		printflags(ptp_flags_options, perout.flags, "PTP_???");
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

			tprintf("{n_samples=%u", sysoff.n_samples);
			return 1;
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
				tprintf("{%" PRId64 ", %" PRIu32 "}",
					(int64_t)sysoff.ts[i].sec,
					sysoff.ts[i].nsec);
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

		tprintf("{max_adj=%d, n_alarm=%d, n_ext_ts=%d, n_per_out=%d, pps=%d}",
			caps.max_adj, caps.n_alarm, caps.n_ext_ts,
			caps.n_per_out, caps.pps);
		break;
	}

	default:
		return RVAL_DECODED;
	}

	return RVAL_DECODED | 1;
}
