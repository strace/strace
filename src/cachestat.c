/*
 * Copyright (c) 2023 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "cachestat.h"

SYS_FUNC(cachestat)
{
	const int fd = tcp->u_arg[0];
	const kernel_ulong_t range_addr = tcp->u_arg[1];
	const kernel_ulong_t stat_addr = tcp->u_arg[2];
	const unsigned int flags = tcp->u_arg[3];

	if (entering(tcp)) {
		printfd(tcp, fd);
		tprint_arg_next();

		struct cachestat_range crange;
		if (!umove_or_printaddr(tcp, range_addr, &crange)) {
			tprint_struct_begin();
			PRINT_FIELD_X(crange, off);
			tprint_struct_next();
			PRINT_FIELD_U(crange, len);
			tprint_struct_end();
		}
		tprint_arg_next();
	} else {
		struct cachestat cstat;
		if (!umove_or_printaddr(tcp, stat_addr, &cstat)) {
			tprint_struct_begin();
			PRINT_FIELD_U(cstat, nr_cache);
			tprint_struct_next();
			PRINT_FIELD_U(cstat, nr_dirty);
			tprint_struct_next();
			PRINT_FIELD_U(cstat, nr_writeback);
			tprint_struct_next();
			PRINT_FIELD_U(cstat, nr_evicted);
			tprint_struct_next();
			PRINT_FIELD_U(cstat, nr_recently_evicted);
			tprint_struct_end();
		}
		tprint_arg_next();

		PRINT_VAL_X(flags);
	}

	return 0;
}
