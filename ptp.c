/*
 * Copyright (c) 2014 Stefan Sørensen <stefan.sorensen@spectralink.com>
 * Copyright (c) 2014-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2014-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
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
