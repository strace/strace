/*
 * Copyright (c) 2018-2020 The strace developers.
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
		tprints(", ");

		if (umove_or_printaddr(tcp, arg, &info))
			return RVAL_IOCTL_DECODED;

		tprintf("{ids_len=%u, ", info);

		return 0;
	}

	if (syserror(tcp) ||
	    umove(tcp, arg + offsetof(struct perf_event_query_bpf, prog_cnt),
		  &info)) {
		tprints("...}");

		return RVAL_IOCTL_DECODED;
	}

	tprintf("prog_cnt=%u, ids=", info);

	print_array(tcp, arg + offsetof(struct perf_event_query_bpf, ids), info,
		    &info, sizeof(info),
		    tfetch_mem, print_uint32_array_member, NULL);

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
perf_ioctl_modify_attributes(struct tcb *const tcp, const kernel_ulong_t arg)
{
	tprints(", ");
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
		tprints(", ");
		printflags(perf_ioctl_flags, arg, "PERF_IOC_FLAG_???");

		return RVAL_IOCTL_DECODED;

	case PERF_EVENT_IOC_REFRESH:
		tprintf(", %d", (int) arg);

		return RVAL_IOCTL_DECODED;

	case PERF_EVENT_IOC_PERIOD:
		tprints(", ");
		printnum_int64(tcp, arg, "%" PRIu64);

		return RVAL_IOCTL_DECODED;

	case PERF_EVENT_IOC_SET_OUTPUT:
	case PERF_EVENT_IOC_SET_BPF:
		tprints(", ");
		printfd(tcp, (int) arg);

		return RVAL_IOCTL_DECODED;

	case PERF_EVENT_IOC_PAUSE_OUTPUT:
		tprintf(", %" PRI_klu, arg);

		return RVAL_IOCTL_DECODED;

	/*
	 * The following ioctl requests are personality-specific
	 * due to the pointer size.
	 */
	case PERF_EVENT_IOC_SET_FILTER:
		tprints(", ");
		printstr_ex(tcp, arg, get_pagesize(), QUOTE_0_TERMINATED);

		return RVAL_IOCTL_DECODED;

	case PERF_EVENT_IOC_ID:
		if (entering(tcp)) {
			tprints(", ");

			return 0;
		}

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
