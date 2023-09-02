/*
 * Copyright (c) 2014 Stefan Sørensen <stefan.sorensen@spectralink.com>
 * Copyright (c) 2014-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2014-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <linux/ioctl.h>
#include <linux/ptp_clock.h>

#include "xlat/ptp_extts_flags.h"
#include "xlat/ptp_perout_flags.h"
#include "xlat/ptp_pin_funcs.h"

static void
print_ptp_clock_time(const struct ptp_clock_time *const p, bool rtc)
{
	tprint_struct_begin();
	PRINT_FIELD_D(*p, sec);
	tprint_struct_next();
	PRINT_FIELD_U(*p, nsec);
	if (p->reserved) {
		tprint_struct_next();
		PRINT_FIELD_X(*p, reserved);
	}
	tprint_struct_end();

	if (rtc && xlat_verbose(xlat_verbosity) != XLAT_STYLE_RAW)
		tprints_comment(sprinttime_nsec(p->sec, p->nsec));
}

static bool
print_ptp_clock_time_am(struct tcb *tcp, void *elem_buf, size_t elem_size,
			void *data)
{
	print_ptp_clock_time(elem_buf, true);
	return true;
}

static bool
print_ptp_clock_time3_am(struct tcb *tcp, void *elem_buf, size_t elem_size,
			 void *data)
{
	const struct ptp_clock_time *const p = elem_buf;

	tprint_array_begin();
	print_ptp_clock_time(p, true);
	tprint_array_next();
	print_ptp_clock_time(p + 1, true);
	tprint_array_next();
	print_ptp_clock_time(p + 2, true);
	tprint_array_end();

	return true;
}

#define PRINT_RSV(where_, field_)					\
	do {								\
		if (!IS_ARRAY_ZERO(where_.field_)) {			\
			tprint_struct_next();				\
			PRINT_FIELD_ARRAY(where_, field_, tcp,		\
					  print_xint_array_member);	\
		}							\
	} while (0)

int
ptp_ioctl(struct tcb *const tcp, const unsigned int code,
	  const kernel_ulong_t arg)
{
	if (!verbose(tcp))
		return RVAL_DECODED;

	switch (code) {
	case PTP_CLOCK_GETCAPS:
	case PTP_CLOCK_GETCAPS2: {
		struct ptp_clock_caps caps;
		CHECK_TYPE_SIZE(caps.rsv, sizeof(unsigned int) * 11);
		CHECK_IOCTL_SIZE(PTP_CLOCK_GETCAPS, 80);
		CHECK_IOCTL_SIZE(PTP_CLOCK_GETCAPS2, 80);

		if (entering(tcp)) {
			tprint_arg_next();
			return 0;
		}

		if (umove_or_printaddr(tcp, arg, &caps))
			break;

		tprint_struct_begin();
		PRINT_FIELD_D(caps, max_adj);
		tprint_struct_next();
		PRINT_FIELD_D(caps, n_alarm);
		tprint_struct_next();
		PRINT_FIELD_D(caps, n_ext_ts);
		tprint_struct_next();
		PRINT_FIELD_D(caps, n_per_out);
		tprint_struct_next();
		PRINT_FIELD_D(caps, pps);
		tprint_struct_next();
		PRINT_FIELD_D(caps, n_pins);
		tprint_struct_next();
		PRINT_FIELD_D(caps, cross_timestamping);
		tprint_struct_next();
		PRINT_FIELD_D(caps, adjust_phase);
		tprint_struct_next();
		PRINT_FIELD_TICKS_D(caps, max_phase_adj, 1000000000, 9);
		PRINT_RSV(caps, rsv);
		tprint_struct_end();
		break;
	}

	case PTP_EXTTS_REQUEST:
	case PTP_EXTTS_REQUEST2: {
		struct ptp_extts_request extts;
		CHECK_TYPE_SIZE(extts.rsv, sizeof(unsigned int) * 2);
		CHECK_IOCTL_SIZE(PTP_EXTTS_REQUEST, 16);
		CHECK_IOCTL_SIZE(PTP_EXTTS_REQUEST2, 16);

		tprint_arg_next();
		if (umove_or_printaddr(tcp, arg, &extts))
			break;

		tprint_struct_begin();
		PRINT_FIELD_U(extts, index);
		tprint_struct_next();
		PRINT_FIELD_FLAGS(extts, flags, ptp_extts_flags, "PTP_???");
		if (code == PTP_EXTTS_REQUEST2)
			PRINT_RSV(extts, rsv);
		tprint_struct_end();
		break;
	}

	case PTP_PEROUT_REQUEST:
	case PTP_PEROUT_REQUEST2: {
		struct ptp_perout_request perout;
		CHECK_TYPE_SIZE(perout.rsv, sizeof(unsigned int) * 4);
		CHECK_IOCTL_SIZE(PTP_PEROUT_REQUEST, 56);
		CHECK_IOCTL_SIZE(PTP_PEROUT_REQUEST2, 56);

		tprint_arg_next();
		if (umove_or_printaddr(tcp, arg, &perout))
			break;

		tprint_struct_begin();
		if (perout.flags & PTP_PEROUT_PHASE) {
			PRINT_FIELD_OBJ_PTR(perout, phase, print_ptp_clock_time,
					    false);
		} else {
			PRINT_FIELD_OBJ_PTR(perout, start, print_ptp_clock_time,
					    true);
		}
		tprint_struct_next();
		PRINT_FIELD_OBJ_PTR(perout, period, print_ptp_clock_time,
				    false);
		tprint_struct_next();
		PRINT_FIELD_U(perout, index);
		tprint_struct_next();
		PRINT_FIELD_FLAGS(perout, flags, ptp_perout_flags,
				  "PTP_PEROUT_???");
		if (perout.flags & PTP_PEROUT_DUTY_CYCLE) {
			tprint_struct_next();
			PRINT_FIELD_OBJ_PTR(perout, on, print_ptp_clock_time,
					    false);
		} else if (code == PTP_PEROUT_REQUEST2) {
			PRINT_RSV(perout, rsv);
		}
		tprint_struct_end();
		break;
	}

	case PTP_ENABLE_PPS:
	case PTP_ENABLE_PPS2:
		tprint_arg_next();
		PRINT_VAL_X(arg);
		break;

	case PTP_SYS_OFFSET:
	case PTP_SYS_OFFSET2: {
		struct ptp_sys_offset sysoff;
		CHECK_TYPE_SIZE(sysoff.rsv, sizeof(unsigned int) * 3);
		CHECK_IOCTL_SIZE(PTP_SYS_OFFSET, 832);
		CHECK_IOCTL_SIZE(PTP_SYS_OFFSET2, 832);

		if (entering(tcp)) {
			tprint_arg_next();
			if (umove_or_printaddr(tcp, arg, &sysoff))
				break;

			tprint_struct_begin();
			PRINT_FIELD_U(sysoff, n_samples);
			PRINT_RSV(sysoff, rsv);
			return 0;
		} else {
			if (syserror(tcp)) {
				/* ... */
			} else if (!umove(tcp, arg, &sysoff)) {
				unsigned int n_samples =
					MIN(sysoff.n_samples, PTP_MAX_SAMPLES);
				tprint_struct_next();
				PRINT_FIELD_ARRAY_UPTO(sysoff, ts,
						       2 * n_samples + 1, tcp,
						       print_ptp_clock_time_am);
			} else {
				tprint_struct_next();
				tprint_unavailable();
			}
			tprint_struct_end();
			break;
		}
	}

	case PTP_PIN_GETFUNC:
	case PTP_PIN_GETFUNC2:
	case PTP_PIN_SETFUNC:
	case PTP_PIN_SETFUNC2: {
		struct ptp_pin_desc pinfunc;
		CHECK_TYPE_SIZE(pinfunc.rsv, sizeof(unsigned int) * 5);
		CHECK_IOCTL_SIZE(PTP_PIN_GETFUNC, 96);
		CHECK_IOCTL_SIZE(PTP_PIN_GETFUNC2, 96);
		CHECK_IOCTL_SIZE(PTP_PIN_SETFUNC, 96);
		CHECK_IOCTL_SIZE(PTP_PIN_SETFUNC2, 96);

		if (entering(tcp)) {
			tprint_arg_next();

			if (umove_or_printaddr(tcp, arg, &pinfunc))
				break;

			tprint_struct_begin();
			PRINT_FIELD_U(pinfunc, index);

			switch (code) {
			case PTP_PIN_GETFUNC2:
				PRINT_RSV(pinfunc, rsv);
				ATTRIBUTE_FALLTHROUGH;
			case PTP_PIN_GETFUNC:
				return 0;
			}
		} else /* getter syscall exit */ {
			if (syserror(tcp)) {
				tprint_struct_end();
				break;
			}

			if (umove(tcp, arg, &pinfunc) < 0) {
				tprint_arg_next();
				tprint_unavailable();
				tprint_struct_end();
				break;
			}
		}

		/* setter syscall enter or getter syscall exit */
		switch (code) {
		case PTP_PIN_GETFUNC:
		case PTP_PIN_GETFUNC2:
			tprint_struct_next();
			PRINT_FIELD_CSTRING(pinfunc, name);
		}
		tprint_struct_next();
		PRINT_FIELD_XVAL(pinfunc, func, ptp_pin_funcs, "PTP_PF_???");
		tprint_struct_next();
		PRINT_FIELD_U(pinfunc, chan);
		if (code == PTP_PIN_SETFUNC2) {
			PRINT_RSV(pinfunc, rsv);
		}
		tprint_struct_end();
		break;
	}

	case PTP_SYS_OFFSET_PRECISE:
	case PTP_SYS_OFFSET_PRECISE2: {
		struct ptp_sys_offset_precise sysoff;
		CHECK_TYPE_SIZE(sysoff.rsv, sizeof(unsigned int) * 4);
		CHECK_IOCTL_SIZE(PTP_SYS_OFFSET_PRECISE, 64);
		CHECK_IOCTL_SIZE(PTP_SYS_OFFSET_PRECISE2, 64);

		if (entering(tcp)) {
			tprint_arg_next();
			return 0;
		}

		if (umove_or_printaddr(tcp, arg, &sysoff))
			break;

		tprint_struct_begin();
		PRINT_FIELD_OBJ_PTR(sysoff, device, print_ptp_clock_time, true);
		tprint_struct_next();
		PRINT_FIELD_OBJ_PTR(sysoff, sys_realtime, print_ptp_clock_time,
				    true);
		tprint_struct_next();
		PRINT_FIELD_OBJ_PTR(sysoff, sys_monoraw, print_ptp_clock_time,
				    false);
		PRINT_RSV(sysoff, rsv);
		tprint_struct_end();
		break;
	}

	case PTP_SYS_OFFSET_EXTENDED:
	case PTP_SYS_OFFSET_EXTENDED2: {
		struct ptp_sys_offset_extended sysoff;
		CHECK_TYPE_SIZE(sysoff.rsv, sizeof(unsigned int) * 3);
		CHECK_IOCTL_SIZE(PTP_SYS_OFFSET_EXTENDED, 1216);
		CHECK_IOCTL_SIZE(PTP_SYS_OFFSET_EXTENDED2, 1216);

		if (entering(tcp)) {
			tprint_arg_next();
			if (umove_or_printaddr(tcp, arg, &sysoff))
				break;

			tprint_struct_begin();
			PRINT_FIELD_U(sysoff, n_samples);
			PRINT_RSV(sysoff, rsv);
			return 0;
		}

		if (syserror(tcp)) {
			/* ... */
		} else if (!umove(tcp, arg, &sysoff)) {
			unsigned int n_samples =
				MIN(sysoff.n_samples, PTP_MAX_SAMPLES);
			tprint_struct_next();
			PRINT_FIELD_ARRAY_UPTO(sysoff, ts,
					       n_samples, tcp,
					       print_ptp_clock_time3_am);
		} else {
			tprint_struct_next();
			tprint_unavailable();
		}
		tprint_struct_end();
		break;
	}

	default:
		return RVAL_DECODED;
	}

	return RVAL_IOCTL_DECODED;
}
