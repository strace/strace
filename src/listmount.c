/*
 * Copyright (c) 2024 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/mount.h>
#include "xlat/listmount_mnt_id.h"
#include "xlat/listmount_flags.h"

static void
print_mnt_id_req(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct mnt_id_req req = { 0 };

	static_assert(offsetof(struct mnt_id_req, size) == 0,
		      "offsetof(struct mnt_id_req, size) > 0");
	if (umove_or_printaddr(tcp, addr, &req.size))
		return;

	tprint_struct_begin();
	PRINT_FIELD_U(req, size);

	if (req.size < MNT_ID_REQ_SIZE_VER0) {
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
	PRINT_FIELD_XVAL(req, mnt_id, listmount_mnt_id, NULL);

	tprint_struct_next();
	PRINT_FIELD_X(req, param);

	if (req.size >= offsetofend(struct mnt_id_req, mnt_ns_id)) {
		tprint_struct_next();
		PRINT_FIELD_X(req, mnt_ns_id);
	}

	if (req.size > MNT_ID_REQ_SIZE_VER1) {
		print_nonzero_bytes(tcp, tprint_struct_next, addr,
				    MNT_ID_REQ_SIZE_VER1,
				    MIN(req.size, get_pagesize()),
				    QUOTE_FORCE_HEX);
	}

	tprint_struct_end();
}

SYS_FUNC(listmount)
{
	const kernel_ulong_t req = tcp->u_arg[0];
	const kernel_ulong_t mnt_ids = tcp->u_arg[1];
	const kernel_ulong_t nr_mnt_ids = tcp->u_arg[2];
	const unsigned int flags = tcp->u_arg[3];
	uint64_t elem;

	if (entering(tcp)) {
		print_mnt_id_req(tcp, req);
		tprint_arg_next();
		return 0;
	}

	print_array(tcp, mnt_ids, MIN(nr_mnt_ids, (kernel_ulong_t) tcp->u_rval),
		    &elem, sizeof(elem), tfetch_mem, print_xint_array_member, 0);
	tprint_arg_next();

	PRINT_VAL_U(nr_mnt_ids);
	tprint_arg_next();

	printflags(listmount_flags, flags, "LISTMOUNT_???");

	return 0;
}
