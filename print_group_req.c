/*
 * Copyright (c) 2015-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <netinet/in.h>

#ifdef MCAST_JOIN_GROUP

# include DEF_MPERS_TYPE(struct_group_req)
typedef struct group_req struct_group_req;

#endif /* MCAST_JOIN_GROUP */

#include MPERS_DEFS

#ifdef MCAST_JOIN_GROUP

# include "print_fields.h"

MPERS_PRINTER_DECL(void, print_group_req, struct tcb *const tcp,
		   const kernel_ulong_t addr, const int len)
{
	struct_group_req greq;

	if (len < (int) sizeof(greq)) {
		printaddr(addr);
	} else if (!umove_or_printaddr(tcp, addr, &greq)) {
		PRINT_FIELD_IFINDEX("{", greq, gr_interface);
		PRINT_FIELD_SOCKADDR(", ", greq, gr_group, tcp);
		tprints("}");
	}
}

#endif /* MCAST_JOIN_GROUP */
