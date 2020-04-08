/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2018 The strace developers.
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
#include "print_fields.h"

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

	PRINT_FIELD_XVAL("{", req, diag_family, addrfams, "AF_???");
	tprints(", ");
	if (len >= sizeof(req)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(req) - offset,
					 (void *) &req + offset)) {
			PRINT_FIELD_FLAGS("", req, diag_ext,
					  smc_diag_extended_flags,
					  "1<<SMC_DIAG_\?\?\?-1");
			/*
			 * AF_SMC protocol family socket handler
			 * keeping the AF_INET sock address.
			 */
			PRINT_FIELD_INET_DIAG_SOCKID(", ", req, id, AF_INET);
		}
	} else
		tprints("...");
	tprints("}");
}

static void
print_smc_diag_cursor(const struct smc_diag_cursor *const cursor)
{
	PRINT_FIELD_U("{", *cursor, reserved);
	PRINT_FIELD_U(", ", *cursor, wrap);
	PRINT_FIELD_U(", ", *cursor, count);
	tprints("}");
}

#define PRINT_FIELD_SMC_DIAG_CURSOR(prefix_, where_, field_)		\
	do {								\
		tprintf("%s%s=", (prefix_), #field_);			\
		print_smc_diag_cursor(&(where_).field_);		\
	} while (0)

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

	PRINT_FIELD_U("{", cinfo, token);
	PRINT_FIELD_U(", ", cinfo, sndbuf_size);
	PRINT_FIELD_U(", ", cinfo, rmbe_size);
	PRINT_FIELD_U(", ", cinfo, peer_rmbe_size);
	PRINT_FIELD_SMC_DIAG_CURSOR(", ", cinfo, rx_prod);
	PRINT_FIELD_SMC_DIAG_CURSOR(", ", cinfo, rx_cons);
	PRINT_FIELD_SMC_DIAG_CURSOR(", ", cinfo, tx_prod);
	PRINT_FIELD_SMC_DIAG_CURSOR(", ", cinfo, tx_cons);
	PRINT_FIELD_0X(", ", cinfo, rx_prod_flags);
	PRINT_FIELD_0X(", ", cinfo, rx_conn_state_flags);
	PRINT_FIELD_0X(", ", cinfo, tx_prod_flags);
	PRINT_FIELD_0X(", ", cinfo, tx_conn_state_flags);
	PRINT_FIELD_SMC_DIAG_CURSOR(", ", cinfo, tx_prep);
	PRINT_FIELD_SMC_DIAG_CURSOR(", ", cinfo, tx_sent);
	PRINT_FIELD_SMC_DIAG_CURSOR(", ", cinfo, tx_fin);
	tprints("}");

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

	tprints("{lnk[0]={");
	PRINT_FIELD_U("", linfo.lnk[0], link_id);
	PRINT_FIELD_CSTRING(", ", linfo.lnk[0], ibname);
	PRINT_FIELD_U(", ", linfo.lnk[0], ibport);
	PRINT_FIELD_CSTRING(", ", linfo.lnk[0], gid);
	PRINT_FIELD_CSTRING(", ", linfo.lnk[0], peer_gid);
	PRINT_FIELD_XVAL("}, ", linfo, role, smc_link_group_roles, "SMC_???");
	tprints("}");

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
	struct smcd_diag_dmbinfo dinfo;

	if (len < sizeof(dinfo))
		return false;
	if (umove_or_printaddr(tcp, addr, &dinfo))
		return true;

	PRINT_FIELD_U("{", dinfo, linkid);
	PRINT_FIELD_X(", ", dinfo, peer_gid);
	PRINT_FIELD_X(", ", dinfo, my_gid);
	PRINT_FIELD_X(", ", dinfo, token);
	PRINT_FIELD_X(", ", dinfo, peer_token);
	tprints("}");

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
	tprints("{reason=");
	printxval_ex(smc_decl_codes, fb.reason, "SMC_CLC_DECL_???",
		     XLAT_STYLE_VERBOSE);
	tprints(", peer_diagnosis=");
	printxval_ex(smc_decl_codes, fb.peer_diagnosis, "SMC_CLC_DECL_???",
		     XLAT_STYLE_VERBOSE);
	tprints("}");

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

	PRINT_FIELD_XVAL("{", msg, diag_family, addrfams, "AF_???");
	tprints(", ");
	if (len >= sizeof(msg)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(msg) - offset,
					 (void *) &msg + offset)) {
			PRINT_FIELD_XVAL("", msg, diag_state,
					 smc_states, "SMC_???");
			PRINT_FIELD_XVAL(", ", msg, diag_fallback,
					 smc_diag_mode, "SMC_DIAG_MODE_???");
			PRINT_FIELD_U(", ", msg, diag_shutdown);
			/*
			 * AF_SMC protocol family socket handler
			 * keeping the AF_INET sock address.
			 */
			PRINT_FIELD_INET_DIAG_SOCKID(", ", msg, id, AF_INET);
			PRINT_FIELD_U(", ", msg, diag_uid);
			PRINT_FIELD_U(", ", msg, diag_inode);
			decode_nla = true;
		}
	} else
		tprints("...");
	tprints("}");

	offset = NLMSG_ALIGN(sizeof(msg));
	if (decode_nla && len > offset) {
		tprints(", ");
		decode_nlattr(tcp, addr + offset, len - offset,
			      smc_diag_attrs, "SMC_DIAG_???",
			      smc_diag_msg_nla_decoders,
			      ARRAY_SIZE(smc_diag_msg_nla_decoders), NULL);
	}
}
