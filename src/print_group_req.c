/*
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <netinet/in.h>

# include DEF_MPERS_TYPE(struct_group_req)
typedef struct group_req struct_group_req;

#include MPERS_DEFS

MPERS_PRINTER_DECL(void, print_group_req, struct tcb *const tcp,
		   const kernel_ulong_t addr, const int len)
{
	struct_group_req greq;

	if (len < (int) sizeof(greq)) {
		printaddr(addr);
	} else if (!umove_or_printaddr(tcp, addr, &greq)) {
		tprint_struct_begin();
		PRINT_FIELD_IFINDEX(greq, gr_interface);
		tprint_struct_next();
		PRINT_FIELD_SOCKADDR(greq, gr_group, tcp);
		tprint_struct_end();
	}
}
