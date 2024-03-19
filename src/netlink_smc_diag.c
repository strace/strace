/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <sys/socket.h>

#ifndef AF_SMC
# define XLAT_MACROS_ONLY
# include "xlat/addrfams.h"
# undef XLAT_MACROS_ONLY
#endif

#include "netlink.h"
#include "netlink_sock_diag.h"
#include "nlattr.h"

#include <arpa/inet.h>
#include <linux/smc_diag.h>

#include "xlat/smc_decl_codes.h"
#include "xlat/smc_diag_attrs.h"
#include "xlat/smc_diag_extended_flags.h"
#include "xlat/smc_diag_mode.h"
#include "xlat/smc_link_group_roles.h"
#include "xlat/smc_states.h"
#include "xlat/sock_shutdown_flags.h"

DECL_NETLINK_DIAG_DECODER(decode_smc_diag_req)
{
	struct smc_diag_req req = { .diag_family = family };
	const size_t offset = sizeof(req.diag_family);

	tprint_struct_begin();
	PRINT_FIELD_XVAL(req, diag_family, addrfams, "AF_???");
	tprint_struct_next();

	if (len >= sizeof(req)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(req) - offset,
					 (void *) &req + offset)) {
			PRINT_FIELD_FLAGS(req, diag_ext,
					  smc_diag_extended_flags,
					  "1<<SMC_DIAG_\?\?\?-1");
			/*
			 * AF_SMC protocol family socket handler
			 * keeping the AF_INET sock address.
			 */
			tprint_struct_next();
			PRINT_FIELD_INET_DIAG_SOCKID(req, id, AF_INET);
		}
	} else
		tprint_more_data_follows();
	tprint_struct_end();
}

static void
print_smc_diag_cursor(const struct smc_diag_cursor *const cursor)
{
	tprint_struct_begin();
	PRINT_FIELD_U(*cursor, reserved);
	tprint_struct_next();
	PRINT_FIELD_U(*cursor, wrap);
	tprint_struct_next();
	PRINT_FIELD_U(*cursor, count);
	tprint_struct_end();
}

static bool
decode_smc_diag_conninfo(struct tcb *const tcp,
			 const kernel_ulong_t addr,
			 const unsigned int len,
			 const void *const opaque_data)
{
	struct smc_diag_conninfo cinfo;

	if (len < sizeof(cinfo))
		return false;
	if (umove_or_printaddr(tcp, addr, &cinfo))
		return true;

	tprint_struct_begin();
	PRINT_FIELD_U(cinfo, token);
	tprint_struct_next();
	PRINT_FIELD_U(cinfo, sndbuf_size);
	tprint_struct_next();
	PRINT_FIELD_U(cinfo, rmbe_size);
	tprint_struct_next();
	PRINT_FIELD_U(cinfo, peer_rmbe_size);
	tprint_struct_next();
	PRINT_FIELD_OBJ_PTR(cinfo, rx_prod, print_smc_diag_cursor);
	tprint_struct_next();
	PRINT_FIELD_OBJ_PTR(cinfo, rx_cons, print_smc_diag_cursor);
	tprint_struct_next();
	PRINT_FIELD_OBJ_PTR(cinfo, tx_prod, print_smc_diag_cursor);
	tprint_struct_next();
	PRINT_FIELD_OBJ_PTR(cinfo, tx_cons, print_smc_diag_cursor);
	tprint_struct_next();
	PRINT_FIELD_0X(cinfo, rx_prod_flags);
	tprint_struct_next();
	PRINT_FIELD_0X(cinfo, rx_conn_state_flags);
	tprint_struct_next();
	PRINT_FIELD_0X(cinfo, tx_prod_flags);
	tprint_struct_next();
	PRINT_FIELD_0X(cinfo, tx_conn_state_flags);
	tprint_struct_next();
	PRINT_FIELD_OBJ_PTR(cinfo, tx_prep, print_smc_diag_cursor);
	tprint_struct_next();
	PRINT_FIELD_OBJ_PTR(cinfo, tx_sent, print_smc_diag_cursor);
	tprint_struct_next();
	PRINT_FIELD_OBJ_PTR(cinfo, tx_fin, print_smc_diag_cursor);
	tprint_struct_end();

	return true;
}

static bool
print_smc_diag_linkinfo_array_member(struct tcb *tcp, void *elem_buf,
				     size_t elem_size, void *data)
{
	const struct smc_diag_linkinfo *const p = elem_buf;
	tprint_struct_begin();
	PRINT_FIELD_U(*p, link_id);
	tprint_struct_next();
	PRINT_FIELD_CSTRING(*p, ibname);
	tprint_struct_next();
	PRINT_FIELD_U(*p, ibport);
	tprint_struct_next();
	PRINT_FIELD_CSTRING(*p, gid);
	tprint_struct_next();
	PRINT_FIELD_CSTRING(*p, peer_gid);
	tprint_struct_end();
	return true;
}

static bool
decode_smc_diag_lgrinfo(struct tcb *const tcp,
			const kernel_ulong_t addr,
			const unsigned int len,
			const void *const opaque_data)
{
	struct smc_diag_lgrinfo linfo;

	if (len < sizeof(linfo))
		return false;
	if (umove_or_printaddr(tcp, addr, &linfo))
		return true;

	tprint_struct_begin();
	PRINT_FIELD_ARRAY(linfo, lnk, tcp,
			  print_smc_diag_linkinfo_array_member);
	tprint_struct_next();
	PRINT_FIELD_XVAL(linfo, role, smc_link_group_roles, "SMC_???");
	tprint_struct_end();

	return true;
}

static bool
decode_smc_diag_shutdown(struct tcb *const tcp,
			 const kernel_ulong_t addr,
			 const unsigned int len,
			 const void *const opaque_data)
{
	const struct decode_nla_xlat_opts opts = {
		sock_shutdown_flags, "???_SHUTDOWN",
		.size = 1,
	};

	return decode_nla_flags(tcp, addr, len, &opts);
}

static bool
decode_smc_diag_dmbinfo(struct tcb *const tcp,
			const kernel_ulong_t addr,
			const unsigned int len,
			const void *const opaque_data)
{
	struct smcd_diag_dmbinfo dinfo = { 0 };
	const unsigned int min_size =
		offsetofend(struct smcd_diag_dmbinfo, peer_token);
	const unsigned int max_size =
		offsetofend(struct smcd_diag_dmbinfo, my_gid_ext);

	if (len < min_size)
		return false;
	if (umoven_or_printaddr(tcp, addr, MIN(len, max_size), &dinfo))
		return true;

	tprint_struct_begin();
	PRINT_FIELD_U(dinfo, linkid);
	tprint_struct_next();
	PRINT_FIELD_X(dinfo, peer_gid);
	tprint_struct_next();
	PRINT_FIELD_X(dinfo, my_gid);
	tprint_struct_next();
	PRINT_FIELD_X(dinfo, token);
	tprint_struct_next();
	PRINT_FIELD_X(dinfo, peer_token);
	if (len > min_size) {
		tprint_struct_next();
		PRINT_FIELD_X(dinfo, peer_gid_ext);
		tprint_struct_next();
		PRINT_FIELD_X(dinfo, my_gid_ext);
	}
	tprint_struct_end();

	return true;
}

static bool
decode_smc_diag_fallback(struct tcb *const tcp,
			 const kernel_ulong_t addr,
			 const unsigned int len,
			 const void *const opaque_data)
{
	struct smc_diag_fallback fb;

	if (len < sizeof(fb))
		return false;
	if (umove_or_printaddr(tcp, addr, &fb))
		return true;

	/*
	 * We print them verbose since they are defined in a non-UAPI header,
	 * net/smc/smc_clc.h
	 */
	tprint_struct_begin();
	PRINT_FIELD_XVAL_VERBOSE(fb, reason, smc_decl_codes,
				 "SMC_CLC_DECL_???");
	tprint_struct_next();
	PRINT_FIELD_XVAL_VERBOSE(fb, peer_diagnosis, smc_decl_codes,
				 "SMC_CLC_DECL_???");
	tprint_struct_end();

	return true;
}

static const nla_decoder_t smc_diag_msg_nla_decoders[] = {
	[SMC_DIAG_CONNINFO]	= decode_smc_diag_conninfo,
	[SMC_DIAG_LGRINFO]	= decode_smc_diag_lgrinfo,
	[SMC_DIAG_SHUTDOWN]	= decode_smc_diag_shutdown,
	[SMC_DIAG_DMBINFO]      = decode_smc_diag_dmbinfo,
	[SMC_DIAG_FALLBACK]	= decode_smc_diag_fallback,
};

DECL_NETLINK_DIAG_DECODER(decode_smc_diag_msg)
{
	struct smc_diag_msg msg = { .diag_family = family };
	size_t offset = sizeof(msg.diag_family);
	bool decode_nla = false;

	tprint_struct_begin();
	PRINT_FIELD_XVAL(msg, diag_family, addrfams, "AF_???");
	tprint_struct_next();

	if (len >= sizeof(msg)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(msg) - offset,
					 (void *) &msg + offset)) {
			PRINT_FIELD_XVAL(msg, diag_state,
					 smc_states, "SMC_???");
			tprint_struct_next();
			PRINT_FIELD_XVAL(msg, diag_fallback,
					 smc_diag_mode, "SMC_DIAG_MODE_???");
			tprint_struct_next();
			PRINT_FIELD_U(msg, diag_shutdown);
			/*
			 * AF_SMC protocol family socket handler
			 * keeping the AF_INET sock address.
			 */
			tprint_struct_next();
			PRINT_FIELD_INET_DIAG_SOCKID(msg, id, AF_INET);
			tprint_struct_next();
			PRINT_FIELD_U(msg, diag_uid);
			tprint_struct_next();
			PRINT_FIELD_U(msg, diag_inode);
			decode_nla = true;
		}
	} else
		tprint_more_data_follows();
	tprint_struct_end();

	offset = NLMSG_ALIGN(sizeof(msg));
	if (decode_nla && len > offset) {
		tprint_array_next();
		decode_nlattr(tcp, addr + offset, len - offset,
			      smc_diag_attrs, "SMC_DIAG_???",
			      smc_diag_msg_nla_decoders,
			      ARRAY_SIZE(smc_diag_msg_nla_decoders), NULL);
	}
}
