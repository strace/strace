/*
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

#include <linux/ioctl.h>

#include "perf_event_struct.h"

#define XLAT_MACROS_ONLY
# include "xlat/perf_ioctl_cmds.h"
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
		tprintf(", ");
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
