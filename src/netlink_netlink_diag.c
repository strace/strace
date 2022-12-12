/*
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "netlink.h"
#include "netlink_sock_diag.h"
#include "nlattr.h"

#include <linux/sock_diag.h>
#include <linux/netlink_diag.h>

#include "xlat/netlink_diag_attrs.h"
#include "xlat/netlink_diag_show.h"
#include "xlat/netlink_socket_flags.h"
#include "xlat/netlink_states.h"

DECL_NETLINK_DIAG_DECODER(decode_netlink_diag_req)
{
	struct netlink_diag_req req = { .sdiag_family = family };
	const size_t offset = sizeof(req.sdiag_family);

	tprint_struct_begin();
	PRINT_FIELD_XVAL(req, sdiag_family, addrfams, "AF_???");
	tprint_struct_next();

	if (len >= sizeof(req)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(req) - offset,
					 (char *) &req + offset)) {
			if (NDIAG_PROTO_ALL == req.sdiag_protocol) {
				tprints_field_name("sdiag_protocol");
				print_xlat(NDIAG_PROTO_ALL);
			} else {
				PRINT_FIELD_XVAL(req, sdiag_protocol,
						 netlink_protocols,
						 "NETLINK_???");
			}
			tprint_struct_next();
			PRINT_FIELD_U(req, ndiag_ino);
			tprint_struct_next();
			PRINT_FIELD_FLAGS(req, ndiag_show,
					  netlink_diag_show, "NDIAG_SHOW_???");
			tprint_struct_next();
			PRINT_FIELD_COOKIE(req, ndiag_cookie);
		}
	} else
		tprint_more_data_follows();
	tprint_struct_end();
}

static bool
print_group(struct tcb *const tcp,
	    void *const elem_buf,
	    const size_t elem_size,
	    void *const opaque_data)
{
	if (elem_size < sizeof(kernel_ulong_t)) {
		unsigned int val = *(unsigned int *) elem_buf;
		PRINT_VAL_0X(val);
	} else {
		kernel_ulong_t val = *(kernel_ulong_t *) elem_buf;
		PRINT_VAL_0X(val);
	}

	return true;
}

static bool
decode_netlink_diag_groups(struct tcb *const tcp,
			   const kernel_ulong_t addr,
			   const unsigned int len,
			   const void *const opaque_data)
{
	kernel_ulong_t buf;
	const size_t nmemb = len / current_wordsize;

	if (!nmemb)
		return false;

	print_array(tcp, addr, nmemb, &buf, current_wordsize,
		    tfetch_mem, print_group, 0);

	return true;
}

static bool
decode_netlink_diag_ring(struct tcb *const tcp,
			 const kernel_ulong_t addr,
			 const unsigned int len,
			 const void *const opaque_data)
{
	struct netlink_diag_ring ndr;

	if (len < sizeof(ndr))
		return false;
	if (umove_or_printaddr(tcp, addr, &ndr))
		return true;

	tprint_struct_begin();
	PRINT_FIELD_U(ndr, ndr_block_size);
	tprint_struct_next();
	PRINT_FIELD_U(ndr, ndr_block_nr);
	tprint_struct_next();
	PRINT_FIELD_U(ndr, ndr_frame_size);
	tprint_struct_next();
	PRINT_FIELD_U(ndr, ndr_frame_nr);
	tprint_struct_end();

	return true;
}

static bool
decode_netlink_diag_flags(struct tcb *const tcp,
			  const kernel_ulong_t addr,
			  const unsigned int len,
			  const void *const opaque_data)
{
	uint32_t flags;

	if (len < sizeof(flags))
		return false;
	if (umove_or_printaddr(tcp, addr, &flags))
		return true;

	printflags(netlink_socket_flags, flags, "NDIAG_FLAG_???");

	return true;
}

static const nla_decoder_t netlink_diag_msg_nla_decoders[] = {
	[NETLINK_DIAG_MEMINFO]	= decode_nla_meminfo,
	[NETLINK_DIAG_GROUPS]	= decode_netlink_diag_groups,
	[NETLINK_DIAG_RX_RING]	= decode_netlink_diag_ring,
	[NETLINK_DIAG_TX_RING]	= decode_netlink_diag_ring,
	[NETLINK_DIAG_FLAGS]	= decode_netlink_diag_flags
};

DECL_NETLINK_DIAG_DECODER(decode_netlink_diag_msg)
{
	struct netlink_diag_msg msg = { .ndiag_family = family };
	size_t offset = sizeof(msg.ndiag_family);
	bool decode_nla = false;

	tprint_struct_begin();
	PRINT_FIELD_XVAL(msg, ndiag_family, addrfams, "AF_???");
	tprint_struct_next();

	if (len >= sizeof(msg)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(msg) - offset,
					 (char *) &msg + offset)) {
			PRINT_FIELD_XVAL(msg, ndiag_type,
					 socktypes, "SOCK_???");
			tprint_struct_next();
			PRINT_FIELD_XVAL(msg, ndiag_protocol,
					 netlink_protocols, "NETLINK_???");
			tprint_struct_next();
			PRINT_FIELD_XVAL(msg, ndiag_state,
					 netlink_states, "NETLINK_???");
			tprint_struct_next();
			PRINT_FIELD_U(msg, ndiag_portid);
			tprint_struct_next();
			PRINT_FIELD_U(msg, ndiag_dst_portid);
			tprint_struct_next();
			PRINT_FIELD_U(msg, ndiag_dst_group);
			tprint_struct_next();
			PRINT_FIELD_U(msg, ndiag_ino);
			tprint_struct_next();
			PRINT_FIELD_COOKIE(msg, ndiag_cookie);
			decode_nla = true;
		}
	} else
		tprint_more_data_follows();
	tprint_struct_end();

	offset = NLMSG_ALIGN(sizeof(msg));
	if (decode_nla && len > offset) {
		tprint_array_next();
		decode_nlattr(tcp, addr + offset, len - offset,
			      netlink_diag_attrs, "NETLINK_DIAG_???",
			      netlink_diag_msg_nla_decoders,
			      ARRAY_SIZE(netlink_diag_msg_nla_decoders), NULL);
	}
}
