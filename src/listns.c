/*
 * Copyright (c) 2024-2026 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/nsfs.h>
#include "xlat/ns_type.h"
#include "xlat/listns_user_ns_id.h"

static void
print_ns_id_req(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct ns_id_req req = { 0 };

	static_assert(offsetof(struct ns_id_req, size) == 0,
		      "offsetof(struct ns_id_req, size) > 0");
	if (umove_or_printaddr(tcp, addr, &req.size))
		return;

	tprint_struct_begin();
	PRINT_FIELD_U(req, size);

	if (req.size < NS_ID_REQ_SIZE_VER0) {
		tprint_struct_end();
		return;
	}

	if (umoven(tcp, addr, MIN(sizeof(req), req.size), &req)) {
		tprint_struct_next();
		tprint_unavailable();
		tprint_struct_end();
		return;
	}

	if (req.spare) {
		tprint_struct_next();
		PRINT_FIELD_X(req, spare);
	}

	tprint_struct_next();
	PRINT_FIELD_X(req, ns_id);

	tprint_struct_next();
	PRINT_FIELD_FLAGS(req, ns_type, ns_type, "???_NS");

	if (req.spare2) {
		tprint_struct_next();
		PRINT_FIELD_X(req, spare2);
	}

	tprint_struct_next();
	PRINT_FIELD_XVAL(req, user_ns_id, listns_user_ns_id, NULL);

	if (req.size > NS_ID_REQ_SIZE_VER0) {
		print_nonzero_bytes(tcp, tprint_struct_next, addr,
				    NS_ID_REQ_SIZE_VER0,
				    MIN(req.size, get_pagesize()),
				    QUOTE_FORCE_HEX);
	}

	tprint_struct_end();
}

SYS_FUNC(listns)
{
	const kernel_ulong_t req = tcp->u_arg[0];
	const kernel_ulong_t ns_ids = tcp->u_arg[1];
	const kernel_ulong_t nr_ns_ids = tcp->u_arg[2];
	const unsigned int flags = tcp->u_arg[3];
	uint64_t elem;

	if (entering(tcp)) {
		tprints_arg_name("req");
		print_ns_id_req(tcp, req);
		return 0;
	}

	tprints_arg_next_name("ns_ids");
	print_array(tcp, ns_ids, MIN(nr_ns_ids, (kernel_ulong_t) tcp->u_rval),
		    &elem, sizeof(elem), tfetch_mem, print_xint_array_member, 0);

	tprints_arg_next_name("nr_ns_ids");
	PRINT_VAL_U(nr_ns_ids);

	tprints_arg_next_name("flags");
	PRINT_VAL_X(flags);

	return 0;
}
