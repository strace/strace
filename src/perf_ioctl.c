/*
 * Copyright (c) 2018-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <linux/ioctl.h>

#include "perf_event_struct.h"

#define XLAT_MACROS_ONLY
#include "xlat/perf_ioctl_cmds.h"
#undef XLAT_MACROS_ONLY

#include "xlat/perf_ioctl_flags.h"

#include MPERS_DEFS

static int
perf_ioctl_query_bpf(struct tcb *const tcp, const kernel_ulong_t arg)
{
	uint32_t info;

	if (entering(tcp)) {
		tprint_arg_next();

		if (umove_or_printaddr(tcp, arg, &info))
			return RVAL_IOCTL_DECODED;

		tprint_struct_begin();
		tprints_field_name("ids_len");
		PRINT_VAL_U(info);
		tprint_struct_next();

		return 0;
	}

	if (syserror(tcp) ||
	    umove(tcp, arg + offsetof(struct perf_event_query_bpf, prog_cnt),
		  &info)) {
		tprint_more_data_follows();
		tprint_struct_end();

		return RVAL_IOCTL_DECODED;
	}

	tprints_field_name("prog_cnt");
	PRINT_VAL_U(info);

	tprint_struct_next();
	tprints_field_name("ids");
	print_array(tcp, arg + offsetof(struct perf_event_query_bpf, ids), info,
		    &info, sizeof(info),
		    tfetch_mem, print_uint_array_member, NULL);

	tprint_struct_end();

	return RVAL_IOCTL_DECODED;
}

static int
perf_ioctl_modify_attributes(struct tcb *const tcp, const kernel_ulong_t arg)
{
	tprint_arg_next();
	if (!fetch_perf_event_attr(tcp, arg))
		print_perf_event_attr(tcp, arg);

	return RVAL_IOCTL_DECODED;
}

MPERS_PRINTER_DECL(int, perf_ioctl,
		   struct tcb *const tcp, const unsigned int code,
		   const kernel_ulong_t arg)
{
	switch (code) {
	case PERF_EVENT_IOC_ENABLE:
	case PERF_EVENT_IOC_DISABLE:
	case PERF_EVENT_IOC_RESET:
		tprint_arg_next();
		printflags(perf_ioctl_flags, arg, "PERF_IOC_FLAG_???");

		return RVAL_IOCTL_DECODED;

	case PERF_EVENT_IOC_REFRESH:
		tprint_arg_next();
		PRINT_VAL_D((int) arg);

		return RVAL_IOCTL_DECODED;

	case PERF_EVENT_IOC_PERIOD:
		tprint_arg_next();
		printnum_int64(tcp, arg, "%" PRIu64);

		return RVAL_IOCTL_DECODED;

	case PERF_EVENT_IOC_SET_OUTPUT:
	case PERF_EVENT_IOC_SET_BPF:
		tprint_arg_next();
		printfd(tcp, (int) arg);

		return RVAL_IOCTL_DECODED;

	case PERF_EVENT_IOC_PAUSE_OUTPUT:
		tprint_arg_next();
		PRINT_VAL_U(arg);

		return RVAL_IOCTL_DECODED;

	/*
	 * The following ioctl requests are personality-specific
	 * due to the pointer size.
	 */
	case PERF_EVENT_IOC_SET_FILTER:
		tprint_arg_next();
		printstr_ex(tcp, arg, get_pagesize(), QUOTE_0_TERMINATED);

		return RVAL_IOCTL_DECODED;

	case PERF_EVENT_IOC_ID:
		if (entering(tcp))
			return 0;

		tprint_arg_next();
		printnum_int64(tcp, arg, "%" PRIu64);

		return RVAL_IOCTL_DECODED;

	case PERF_EVENT_IOC_QUERY_BPF:
		return perf_ioctl_query_bpf(tcp, arg);

	case PERF_EVENT_IOC_MODIFY_ATTRIBUTES:
		return perf_ioctl_modify_attributes(tcp, arg);

	default:
		return RVAL_DECODED;
	}
}
