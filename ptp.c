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

#include <linux/ioctl.h>

#include "print_fields.h"
#include "ptp_clock.h"

#define XLAT_MACROS_ONLY
# include "xlat/ptp_ioctl_cmds.h"
#undef XLAT_MACROS_ONLY

#include "xlat/ptp_flags_options.h"
#include "xlat/ptp_pin_funcs.h"

static void
print_ptp_clock_time(const char *prefix, struct strace_ptp_clock_time *t,
		     bool rtc)
{
	if (prefix && prefix[0])
		tprints(prefix);

	PRINT_FIELD_D("{", *t, sec);
	PRINT_FIELD_U(", ", *t, nsec);
	if (t->reserved)
		PRINT_FIELD_X(", ", *t, reserved);
	tprints("}");

	if (rtc && xlat_verbose(xlat_verbosity) != XLAT_STYLE_RAW)
		tprints_comment(sprinttime_nsec(t->sec, t->nsec));
}

#define PRINT_FIELD_PTP_CLOCK_TIME(prefix_, where_, field_, rtc_) \
	print_ptp_clock_time(prefix_ #field_ "=", &((where_).field_), (rtc_))

#define PRINT_FIELD_RSV(prefix_, where_, field_) \
	do { \
		if (!IS_ARRAY_ZERO(where_.field_)) \
			PRINT_FIELD_HEX_ARRAY(prefix_, where_, field_); \
	} while (0)

int
ptp_ioctl(struct tcb *const tcp, const unsigned int code,
	  const kernel_ulong_t arg)
{
	if (!verbose(tcp))
		return RVAL_DECODED;

	switch (code) {
	case PTP_EXTTS_REQUEST: {
		struct strace_ptp_extts_request extts;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &extts))
			break;

		PRINT_FIELD_U("{", extts, index);
		PRINT_FIELD_FLAGS(", ", extts, flags, ptp_flags_options,
				  "PTP_???");
		PRINT_FIELD_RSV(", ", extts, rsv);
		tprints("}");
		break;
	}

	case PTP_PEROUT_REQUEST: {
		struct strace_ptp_perout_request perout;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &perout))
			break;

		PRINT_FIELD_PTP_CLOCK_TIME("{", perout, start, true);
		PRINT_FIELD_PTP_CLOCK_TIME(", ", perout, period, false);
		PRINT_FIELD_D(", ", perout, index);
		PRINT_FIELD_FLAGS(", ", perout, flags, ptp_flags_options,
				  "PTP_???");
		PRINT_FIELD_RSV(", ", perout, rsv);
		tprints("}");
		break;
	}

	case PTP_ENABLE_PPS:
		tprintf(", %" PRI_klu, arg);
		break;

	case PTP_SYS_OFFSET: {
		struct strace_ptp_sys_offset sysoff;

		if (entering(tcp)) {
			tprints(", ");
			if (umove_or_printaddr(tcp, arg, &sysoff))
				break;

			PRINT_FIELD_U("{", sysoff, n_samples);
			return 0;
		}

		if (syserror(tcp)) {
			tprints("}");
			break;
		}

		if (umove(tcp, arg, &sysoff) < 0) {
			tprints(", ???}");
			break;
		}

		PRINT_FIELD_RSV(", ", sysoff, rsv);

		tprints(", ts=[");
		unsigned int n_samples = sysoff.n_samples > PTP_MAX_SAMPLES ?
			PTP_MAX_SAMPLES : sysoff.n_samples;
		for (unsigned int i = 0; i < 2 * n_samples + 1; ++i) {
			print_ptp_clock_time(i ? ", " : "",
					     sysoff.ts + i, true);
		}
		if (sysoff.n_samples > PTP_MAX_SAMPLES)
			tprints(", ...");
		tprints("]}");
		break;
	}

	case PTP_SYS_OFFSET_PRECISE: {
		struct strace_ptp_sys_offset_precise sysoff;

		if (entering(tcp)) {
			tprints(", ");
			return 0;
		}

		if (syserror(tcp)) {
			tprints("}");
			break;
		}

		if (umove_or_printaddr(tcp, arg, &sysoff))
			break;

		PRINT_FIELD_PTP_CLOCK_TIME("{", sysoff, device, true);
		PRINT_FIELD_PTP_CLOCK_TIME(", ", sysoff, sys_realtime, true);
		PRINT_FIELD_PTP_CLOCK_TIME(", ", sysoff, sys_monoraw, true);
		PRINT_FIELD_RSV(", ", sysoff, rsv);
		tprints("}");
		break;
	}
	case PTP_CLOCK_GETCAPS: {
		struct strace_ptp_clock_caps caps;

		if (entering(tcp)) {
			tprints(", ");
			return 0;
		}

		if (umove_or_printaddr(tcp, arg, &caps))
			break;

		PRINT_FIELD_D("{", caps, max_adj);
		PRINT_FIELD_D(", ", caps, n_alarm);
		PRINT_FIELD_D(", ", caps, n_ext_ts);
		PRINT_FIELD_D(", ", caps, n_per_out);
		PRINT_FIELD_D(", ", caps, pps);
		PRINT_FIELD_D(", ", caps, n_pins);
		PRINT_FIELD_D(", ", caps, cross_timestamping);
		PRINT_FIELD_RSV(", ", caps, rsv);
		tprints("}");
		break;
	}

	case PTP_PIN_GETFUNC:
	case PTP_PIN_SETFUNC: {
		struct strace_ptp_pin_desc pinfunc;

		if (entering(tcp)) {
			tprints(", ");

			if (umove_or_printaddr(tcp, arg, &pinfunc))
				break;

			PRINT_FIELD_U("{", pinfunc, index);

			if (code == PTP_PIN_GETFUNC)
				return 0;
		} else /* getter syscall exit */ {
			if (syserror(tcp)) {
				tprints("}");
				break;
			}

			if (umove(tcp, arg, &pinfunc) < 0) {
				tprints(", ???}");
				break;
			}
		}

		/* setter syscall enter or getter syscall exit */
		PRINT_FIELD_CSTRING(", ", pinfunc, name);
		PRINT_FIELD_XVAL(", ", pinfunc, func, ptp_pin_funcs,
				 "PTP_PF_???");
		PRINT_FIELD_U(", ", pinfunc, chan);
		PRINT_FIELD_RSV(", ", pinfunc, rsv);
		tprints("}");
		break;
	}

	default:
		return RVAL_DECODED;
	}

	return RVAL_IOCTL_DECODED;
}
