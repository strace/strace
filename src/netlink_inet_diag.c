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

#include <arpa/inet.h>

#include <linux/sock_diag.h>
#include <linux/inet_diag.h>
#include <linux/mptcp.h>
#include <linux/tcp.h>
#include <linux/tls.h>

#include "xlat/inet_diag_attrs.h"
#include "xlat/inet_diag_bpf_storage_attrs.h"
#include "xlat/inet_diag_bpf_storages_attrs.h"
#include "xlat/inet_diag_bytecodes.h"
#include "xlat/inet_diag_extended_flags.h"
#include "xlat/inet_diag_req_attrs.h"
#include "xlat/inet_diag_shutdown_flags.h"
#include "xlat/inet_diag_ulp_info_attrs.h"
#include "xlat/inet_diag_ulp_info_mptcp_attrs.h"
#include "xlat/inet_diag_ulp_info_tls_attrs.h"

#include "xlat/mptcp_subflow_flags.h"

#include "xlat/tcp_states.h"
#include "xlat/tcp_state_flags.h"

#include "xlat/tls_info_ciphers.h"
#include "xlat/tls_info_configs.h"
#include "xlat/tls_info_versions.h"


void
print_inet_diag_sockid(const struct inet_diag_sockid *id, const uint8_t family)
{
	tprint_struct_begin();
	PRINT_FIELD_NET_PORT(*id, idiag_sport);
	tprint_struct_next();
	PRINT_FIELD_NET_PORT(*id, idiag_dport);
	tprint_struct_next();
	PRINT_FIELD_INET_ADDR(*id, idiag_src, family);
	tprint_struct_next();
	PRINT_FIELD_INET_ADDR(*id, idiag_dst, family);
	tprint_struct_next();
	PRINT_FIELD_IFINDEX(*id, idiag_if);
	tprint_struct_next();
	PRINT_FIELD_COOKIE(*id, idiag_cookie);
	tprint_struct_end();
}

static void
decode_inet_diag_hostcond(struct tcb *const tcp,
			  const kernel_ulong_t addr,
			  const unsigned int len)
{
	struct inet_diag_hostcond cond;

	if (len < sizeof(cond)) {
		printstr_ex(tcp, addr, len, QUOTE_FORCE_HEX);
		return;
	}
	if (umove_or_printaddr(tcp, addr, &cond))
		return;

	tprint_struct_begin();
	PRINT_FIELD_XVAL(cond, family, addrfams, "AF_???");
	tprint_struct_next();
	PRINT_FIELD_U(cond, prefix_len);
	tprint_struct_next();
	PRINT_FIELD_U(cond, port);

	if (len > sizeof(cond)) {
		tprint_struct_next();
		decode_inet_addr(tcp, addr + sizeof(cond),
				 len - sizeof(cond), cond.family, "addr");
	}
	tprint_struct_end();
}

static void
print_inet_diag_bc_op(const struct inet_diag_bc_op *const op)
{
	tprint_struct_begin();
	PRINT_FIELD_XVAL(*op, code, inet_diag_bytecodes, "INET_DIAG_BC_???");
	tprint_struct_next();
	PRINT_FIELD_U(*op, yes);
	tprint_struct_next();
	PRINT_FIELD_U(*op, no);
	tprint_struct_end();
}

static void
decode_inet_diag_markcond(struct tcb *const tcp,
			  const kernel_ulong_t addr,
			  const unsigned int len)
{
	struct inet_diag_markcond markcond;

	if (len < sizeof(markcond))
		printstr_ex(tcp, addr, len, QUOTE_FORCE_HEX);
	else if (!umove_or_printaddr(tcp, addr, &markcond)) {
		tprint_struct_begin();
		PRINT_FIELD_U(markcond, mark);
		tprint_struct_next();
		PRINT_FIELD_U(markcond, mask);
		tprint_struct_end();
	}
}

static void
decode_bytecode_data(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const unsigned int len,
		     const unsigned char code)
{
	switch (code) {
	case INET_DIAG_BC_S_COND:
	case INET_DIAG_BC_D_COND:
		decode_inet_diag_hostcond(tcp, addr, len);
		break;
	case INET_DIAG_BC_DEV_COND: {
		uint32_t ifindex;

		if (len < sizeof(ifindex))
			printstr_ex(tcp, addr, len, QUOTE_FORCE_HEX);
		else if (!umove_or_printaddr(tcp, addr, &ifindex))
			print_ifindex(ifindex);
		break;
	}
	case INET_DIAG_BC_S_GE:
	case INET_DIAG_BC_S_LE:
	case INET_DIAG_BC_D_GE:
	case INET_DIAG_BC_D_LE: {
		struct inet_diag_bc_op op;

		if (len < sizeof(op))
			printstr_ex(tcp, addr, len, QUOTE_FORCE_HEX);
		else if (!umove_or_printaddr(tcp, addr, &op))
			print_inet_diag_bc_op(&op);
		break;
	}
	case INET_DIAG_BC_MARK_COND:
		decode_inet_diag_markcond(tcp, addr, len);
		break;
	case INET_DIAG_BC_AUTO:
	case INET_DIAG_BC_JMP:
	case INET_DIAG_BC_NOP:
	default:
		printstr_ex(tcp, addr, len, QUOTE_FORCE_HEX);
		break;
	}
}

static bool
decode_inet_diag_bc_op(struct tcb *const tcp,
		       const kernel_ulong_t addr,
		       const unsigned int len,
		       const void *const opaque_data)
{
	struct inet_diag_bc_op op;

	if (len < sizeof(op))
		return false;
	if (umove_or_printaddr(tcp, addr, &op))
		return true;

	print_inet_diag_bc_op(&op);

	if (len > sizeof(op)) {
		tprint_array_next();
		decode_bytecode_data(tcp, addr + sizeof(op),
				     len - sizeof(op), op.code);
	}

	return true;
}

static const nla_decoder_t inet_diag_req_nla_decoders[] = {
	[INET_DIAG_REQ_NONE]		= NULL,
	[INET_DIAG_REQ_BYTECODE]	= decode_inet_diag_bc_op,
	[INET_DIAG_REQ_SK_BPF_STORAGES]	= NULL, /* no payload */
	[INET_DIAG_REQ_PROTOCOL]	= decode_nla_ip_proto,
};

static void
decode_inet_diag_req_compat(struct tcb *const tcp,
			    const struct nlmsghdr *const nlmsghdr,
			    const uint8_t family,
			    const kernel_ulong_t addr,
			    const unsigned int len)
{
	struct inet_diag_req req = { .idiag_family = family };
	size_t offset = sizeof(req.idiag_family);
	bool decode_nla = false;

	tprint_struct_begin();
	PRINT_FIELD_XVAL(req, idiag_family, addrfams, "AF_???");
	tprint_struct_next();

	if (len >= sizeof(req)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(req) - offset,
					 (char *) &req + offset)) {
			PRINT_FIELD_U(req, idiag_src_len);
			tprint_struct_next();
			PRINT_FIELD_U(req, idiag_dst_len);
			tprint_struct_next();
			PRINT_FIELD_FLAGS(req, idiag_ext,
					  inet_diag_extended_flags,
					  "1<<INET_DIAG_\?\?\?-1");
			tprint_struct_next();
			PRINT_FIELD_INET_DIAG_SOCKID(req, id,
						     req.idiag_family);
			tprint_struct_next();
			PRINT_FIELD_FLAGS(req, idiag_states,
					  tcp_state_flags, "1<<TCP_???");
			tprint_struct_next();
			PRINT_FIELD_U(req, idiag_dbs);
			decode_nla = true;
		}
	} else
		tprint_more_data_follows();
	tprint_struct_end();

	offset = NLMSG_ALIGN(sizeof(req));
	if (decode_nla && len > offset) {
		tprint_array_next();
		decode_nlattr(tcp, addr + offset, len - offset,
			      inet_diag_req_attrs, "INET_DIAG_REQ_???",
			      inet_diag_req_nla_decoders,
			      ARRAY_SIZE(inet_diag_req_nla_decoders), NULL);
	}
}

static void
decode_inet_diag_req_v2(struct tcb *const tcp,
			const struct nlmsghdr *const nlmsghdr,
			const uint8_t family,
			const kernel_ulong_t addr,
			const unsigned int len)
{
	struct inet_diag_req_v2 req = { .sdiag_family = family };
	size_t offset = sizeof(req.sdiag_family);
	bool decode_nla = false;

	tprint_struct_begin();
	PRINT_FIELD_XVAL(req, sdiag_family, addrfams, "AF_???");
	tprint_struct_next();

	if (len >= sizeof(req)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(req) - offset,
					 (char *) &req + offset)) {
			PRINT_FIELD_XVAL(req, sdiag_protocol,
					 inet_protocols, "IPPROTO_???");
			tprint_struct_next();
			PRINT_FIELD_FLAGS(req, idiag_ext,
					  inet_diag_extended_flags,
					  "1<<INET_DIAG_\?\?\?-1");
			tprint_struct_next();
			PRINT_FIELD_FLAGS(req, idiag_states,
					  tcp_state_flags, "1<<TCP_???");
			tprint_struct_next();
			PRINT_FIELD_INET_DIAG_SOCKID(req, id,
						     req.sdiag_family);
			decode_nla = true;
		}
	} else
		tprint_more_data_follows();
	tprint_struct_end();

	offset = NLMSG_ALIGN(sizeof(req));
	if (decode_nla && len > offset) {
		tprint_array_next();
		decode_nlattr(tcp, addr + offset, len - offset,
			      inet_diag_req_attrs, "INET_DIAG_REQ_???",
			      inet_diag_req_nla_decoders,
			      ARRAY_SIZE(inet_diag_req_nla_decoders), NULL);
	}
}

DECL_NETLINK_DIAG_DECODER(decode_inet_diag_req)
{
	if (nlmsghdr->nlmsg_type == TCPDIAG_GETSOCK
	    || nlmsghdr->nlmsg_type == DCCPDIAG_GETSOCK)
		decode_inet_diag_req_compat(tcp, nlmsghdr, family, addr, len);
	else
		decode_inet_diag_req_v2(tcp, nlmsghdr, family, addr, len);
}

static bool
decode_inet_diag_meminfo(struct tcb *const tcp,
			 const kernel_ulong_t addr,
			 const unsigned int len,
			 const void *const opaque_data)
{
	struct inet_diag_meminfo minfo;

	if (len < sizeof(minfo))
		return false;
	if (umove_or_printaddr(tcp, addr, &minfo))
		return true;

	tprint_struct_begin();
	PRINT_FIELD_U(minfo, idiag_rmem);
	tprint_struct_next();
	PRINT_FIELD_U(minfo, idiag_wmem);
	tprint_struct_next();
	PRINT_FIELD_U(minfo, idiag_fmem);
	tprint_struct_next();
	PRINT_FIELD_U(minfo, idiag_tmem);
	tprint_struct_end();

	return true;
}

void
print_tcpvegas_info(struct tcb *tcp, const struct tcpvegas_info *const vegas,
		    const unsigned int len)
{
	MAYBE_PRINT_FIELD_LEN(tprint_struct_begin(),
			      *vegas, tcpv_enabled, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      *vegas, tcpv_rttcnt, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      *vegas, tcpv_rtt, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      *vegas, tcpv_minrtt, len, PRINT_FIELD_U);
	tprint_struct_end();
}

static bool
decode_tcpvegas_info(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const unsigned int len,
		     const void *const opaque_data)
{
	struct tcpvegas_info vegas;

	if (len < sizeof(vegas))
		return false;
	if (umove_or_printaddr(tcp, addr, &vegas))
		return true;

	print_tcpvegas_info(tcp, &vegas, len);

	return true;
}

static bool
decode_diag_shutdown(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const unsigned int len,
		     const void *const opaque_data)
{
	struct decode_nla_xlat_opts opts = {
		.xlat  = inet_diag_shutdown_flags,
		.dflt  = "???_SHUTDOWN",
		/*
		 * While these values are exposed to the user space all over
		 * the place, the associated RCV_SHUTDOWN/SEND_SHUTDOWN
		 * constants are not part of UAPI for whatever reason,
		 * hence we cannot print only the symbolic names.
		 */
		.style = xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW
			 ? XLAT_STYLE_RAW : XLAT_STYLE_VERBOSE,
		.size  = 1,
	};

	return decode_nla_flags(tcp, addr, len, &opts);
}

void
print_tcp_dctcp_info(struct tcb *tcp, const struct tcp_dctcp_info *const dctcp,
		     const unsigned int len)
{
	MAYBE_PRINT_FIELD_LEN(tprint_struct_begin(),
			      *dctcp, dctcp_enabled, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      *dctcp, dctcp_ce_state, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      *dctcp, dctcp_alpha, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      *dctcp, dctcp_ab_ecn, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      *dctcp, dctcp_ab_tot, len, PRINT_FIELD_U);
	tprint_struct_end();
}

static bool
decode_tcp_dctcp_info(struct tcb *const tcp,
		      const kernel_ulong_t addr,
		      const unsigned int len,
		      const void *const opaque_data)
{
	struct tcp_dctcp_info dctcp;

	if (len < sizeof(dctcp))
		return false;
	if (umove_or_printaddr(tcp, addr, &dctcp))
		return true;

	print_tcp_dctcp_info(tcp, &dctcp, len);

	return true;
}

static bool
print_sockaddr_array_member(struct tcb *const tcp, void *const elem_buf,
			    const size_t elem_size, void *const data)
{
	print_sockaddr(tcp, elem_buf, elem_size);

	return true;
}

static bool
decode_nla_sockaddrs(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const unsigned int len,
		     const void *const opaque_data)
{
	struct sockaddr_storage sas;
	const size_t nmemb = len / sizeof(sas);

	if (!nmemb)
		return false;

	print_array(tcp, addr, nmemb, &sas, sizeof(sas),
		    tfetch_mem, print_sockaddr_array_member, 0);

	return true;
}

void
print_tcp_bbr_info(struct tcb *tcp, const struct tcp_bbr_info *const bbr,
		   const unsigned int len)
{
	MAYBE_PRINT_FIELD_LEN(tprint_struct_begin(),
			      *bbr, bbr_bw_lo, len, PRINT_FIELD_X);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      *bbr, bbr_bw_hi, len, PRINT_FIELD_X);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      *bbr, bbr_min_rtt, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      *bbr, bbr_pacing_gain, len, PRINT_FIELD_U);
	MAYBE_PRINT_FIELD_LEN(tprint_struct_next(),
			      *bbr, bbr_cwnd_gain, len, PRINT_FIELD_U);
	tprint_struct_end();
}

static bool
decode_tcp_bbr_info(struct tcb *const tcp,
		    const kernel_ulong_t addr,
		    const unsigned int len,
		    const void *const opaque_data)
{
	struct tcp_bbr_info bbr;

	if (len < sizeof(bbr))
		return false;
	if (umove_or_printaddr(tcp, addr, &bbr))
		return true;

	print_tcp_bbr_info(tcp, &bbr, len);

	return true;
}

static bool
print_tcp_md5sig(struct tcb *const tcp, void *const elem_buf,
		 const size_t elem_size, void *const data)
{
	const struct tcp_diag_md5sig *const sig =
			(const struct tcp_diag_md5sig *) elem_buf;

	tprint_struct_begin();
	PRINT_FIELD_XVAL(*sig, tcpm_family, addrfams, "AF_???");
	tprint_struct_next();
	PRINT_FIELD_U(*sig, tcpm_prefixlen);
	tprint_struct_next();
	PRINT_FIELD_U(*sig, tcpm_keylen);
	tprint_struct_next();
	PRINT_FIELD_INET_ADDR(*sig, tcpm_addr, sig->tcpm_family);
	tprint_struct_next();
	PRINT_FIELD_HEX_ARRAY_UPTO(*sig, tcpm_key,
				   MIN(sizeof(sig->tcpm_key),
				       sig->tcpm_keylen));
	tprint_struct_end();

	return true;
}

static bool
decode_tcp_md5sig(struct tcb *const tcp,
		  const kernel_ulong_t addr,
		  const unsigned int len,
		  const void *const opaque_data)
{
	struct tcp_diag_md5sig sig;
	const size_t nmemb = len / sizeof(sig);

	if (!nmemb)
		return false;

	print_array(tcp, addr, nmemb, &sig, sizeof(sig),
		    tfetch_mem, print_tcp_md5sig, 0);

	return true;
}

static bool
decode_tls_version(struct tcb *const tcp,
		   const kernel_ulong_t addr,
		   const unsigned int len,
		   const void *const opaque_data)
{
	static const struct decode_nla_xlat_opts opts = {
		.xlat  = tls_info_versions,
		.dflt  = "TLS_???_VERSION",
		.size  = 2,
	};

	return decode_nla_xval(tcp, addr, len, &opts);
}

static bool
decode_tls_cipher(struct tcb *const tcp,
		  const kernel_ulong_t addr,
		  const unsigned int len,
		  const void *const opaque_data)
{
	static const struct decode_nla_xlat_opts opts = {
		.xlat  = tls_info_ciphers,
		.dflt  = "TLS_CIPHER_???",
		.size  = 2,
	};

	return decode_nla_xval(tcp, addr, len, &opts);
}

static bool
decode_tls_config(struct tcb *const tcp,
		  const kernel_ulong_t addr,
		  const unsigned int len,
		  const void *const opaque_data)
{
	static const struct decode_nla_xlat_opts opts = {
		.xlat  = tls_info_configs,
		.dflt  = "TLS_CONF_???",
		.size  = 2,
	};

	return decode_nla_xval(tcp, addr, len, &opts);
}

static const nla_decoder_t diag_ulp_info_tls_nla_decoders[] = {
	[TLS_INFO_UNSPEC]	= NULL,
	[TLS_INFO_VERSION]	= decode_tls_version,
	[TLS_INFO_CIPHER]	= decode_tls_cipher,
	[TLS_INFO_TXCONF]	= decode_tls_config,
	[TLS_INFO_RXCONF]	= decode_tls_config,
	[TLS_INFO_ZC_RO_TX]	= NULL, /* flag nlattr, no payload */
	[TLS_INFO_RX_NO_PAD]	= NULL, /* flag nlattr, no payload */
};

static bool
decode_diag_ulp_info_tls(struct tcb *const tcp,
			 const kernel_ulong_t addr,
			 const unsigned int len,
			 const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, inet_diag_ulp_info_tls_attrs,
		      "TLS_INFO_???",
		      ARRSZ_PAIR(diag_ulp_info_tls_nla_decoders), NULL);

	return true;
}

static bool
decode_mptcp_subflow_flags(struct tcb *const tcp,
			   const kernel_ulong_t addr,
			   const unsigned int len,
			   const void *const opaque_data)
{
	static const struct decode_nla_xlat_opts opts = {
		.xlat  = mptcp_subflow_flags,
		.dflt  = "MPTCP_SUBFLOW_FLAG_???",
		.size  = 4,
	};

	return decode_nla_flags(tcp, addr, len, &opts);
}

static const nla_decoder_t diag_ulp_info_mptcp_nla_decoders[] = {
	[MPTCP_SUBFLOW_ATTR_UNSPEC]		= NULL,
	[MPTCP_SUBFLOW_ATTR_TOKEN_REM]		= decode_nla_x32,
	[MPTCP_SUBFLOW_ATTR_TOKEN_LOC]		= decode_nla_x32,
	[MPTCP_SUBFLOW_ATTR_RELWRITE_SEQ]	= decode_nla_u32,
	[MPTCP_SUBFLOW_ATTR_MAP_SEQ]		= decode_nla_u64,
	[MPTCP_SUBFLOW_ATTR_MAP_SFSEQ]		= decode_nla_u32,
	[MPTCP_SUBFLOW_ATTR_SSN_OFFSET]		= decode_nla_u32,
	[MPTCP_SUBFLOW_ATTR_MAP_DATALEN]	= decode_nla_u16,
	[MPTCP_SUBFLOW_ATTR_FLAGS]		= decode_mptcp_subflow_flags,
	[MPTCP_SUBFLOW_ATTR_ID_REM]		= decode_nla_u8,
	[MPTCP_SUBFLOW_ATTR_ID_LOC]		= decode_nla_u8,
	[MPTCP_SUBFLOW_ATTR_PAD]		= NULL,
};

static bool
decode_diag_ulp_info_mptcp(struct tcb *const tcp,
			   const kernel_ulong_t addr,
			   const unsigned int len,
			   const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, inet_diag_ulp_info_mptcp_attrs,
		      "MPTCP_SUBFLOW_ATTR_???",
		      ARRSZ_PAIR(diag_ulp_info_mptcp_nla_decoders), NULL);

	return true;
}

static const nla_decoder_t diag_ulp_info_nla_decoders[] = {
	[INET_ULP_INFO_UNSPEC]	= NULL,
	[INET_ULP_INFO_NAME]	= decode_nla_str,
	/*
	 * In theory, the decoding of INTEL_ULP_* attributes depends
	 * on the value of INET_ULP_INFO_NAME attribute, but, luckily,
	 * all currently implemented ULPs that return information
	 * simply use their own attribute types.
	 */
	[INET_ULP_INFO_TLS]	= decode_diag_ulp_info_tls,
	[INET_ULP_INFO_MPTCP]	= decode_diag_ulp_info_mptcp,
};

static bool
decode_diag_ulp_info(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const unsigned int len,
		     const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, inet_diag_ulp_info_attrs,
		      "INET_ULP_INFO_???",
		      ARRSZ_PAIR(diag_ulp_info_nla_decoders), NULL);

	return true;
}

static const nla_decoder_t diag_bpf_storage_nla_decoders[] = {
	[SK_DIAG_BPF_STORAGE_NONE]	= NULL,
	[SK_DIAG_BPF_STORAGE_PAD]	= NULL,
	[SK_DIAG_BPF_STORAGE_MAP_ID]	= decode_nla_u32,
	[SK_DIAG_BPF_STORAGE_MAP_VALUE]	= decode_nla_xint,
};

static bool
decode_diag_bpf_storage(struct tcb *const tcp,
			const kernel_ulong_t addr,
			const unsigned int len,
			const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, inet_diag_bpf_storage_attrs,
		      "SK_DIAG_BPF_STORAGE_???",
		      ARRSZ_PAIR(diag_bpf_storage_nla_decoders), NULL);

	return true;
}

static const nla_decoder_t diag_bpf_storages_nla_decoders[] = {
	[SK_DIAG_BPF_STORAGE_REP_NONE]	= NULL,
	[SK_DIAG_BPF_STORAGE]		= decode_diag_bpf_storage,
};

static bool
decode_diag_bpf_storages(struct tcb *const tcp,
			 const kernel_ulong_t addr,
			 const unsigned int len,
			 const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, inet_diag_bpf_storages_attrs,
		      "SK_DIAG_BPF_STORAGE_???",
		      ARRSZ_PAIR(diag_bpf_storages_nla_decoders), NULL);

	return true;
}

static bool
decode_diag_sockopt(struct tcb *const tcp,
		    const kernel_ulong_t addr,
		    const unsigned int len,
		    const void *const opaque_data)
{
	/*
	 * The own version of struct inet_diag_sockopt here since
	 * there is no trivial way to detect new bit fields and, as a result,
	 * shrinkage of the "unused" field.
	 */
	struct inet_diag_sockopt {
		uint8_t	recverr			:1,
			is_icsk			:1,
			freebind		:1,
			hdrincl			:1,
			mc_loop			:1,
			transparent		:1,
			mc_all			:1,
			nodefrag		:1;
		uint8_t	bind_address_no_port	:1,
			recverr_rfc4884		:1,
			defer_connect		:1,
			unused			:5;
	} sockopt;

	if (len < sizeof(sockopt))
		return false;
	if (umove_or_printaddr(tcp, addr, &sockopt))
		return true;

	void (*delim_f)(void) = tprint_struct_begin;

#define OPT_PRINT_FIELD_U(where_, field_)				\
	do {								\
		if (!abbrev(tcp) || ((where_).field_)) {		\
			delim_f();					\
			delim_f = tprint_struct_next;			\
			PRINT_FIELD_U_CAST((where_), field_, uint8_t);	\
		}							\
	} while (0)

	OPT_PRINT_FIELD_U(sockopt, recverr);
	OPT_PRINT_FIELD_U(sockopt, is_icsk);
	OPT_PRINT_FIELD_U(sockopt, freebind);
	OPT_PRINT_FIELD_U(sockopt, hdrincl);
	OPT_PRINT_FIELD_U(sockopt, mc_loop);
	OPT_PRINT_FIELD_U(sockopt, transparent);
	OPT_PRINT_FIELD_U(sockopt, mc_all);
	OPT_PRINT_FIELD_U(sockopt, nodefrag);
	OPT_PRINT_FIELD_U(sockopt, bind_address_no_port);
	OPT_PRINT_FIELD_U(sockopt, recverr_rfc4884);
	OPT_PRINT_FIELD_U(sockopt, defer_connect);

	if (!abbrev(tcp) || sockopt.unused) {
		delim_f();
		delim_f = tprint_struct_next;
		PRINT_FIELD_X_CAST(sockopt, unused, uint8_t);
		tprints_comment("bits 3..8");
	}

	if (delim_f == tprint_struct_begin)
		tprint_struct_begin();

	tprint_struct_end();

#undef OPT_PRINT_FIELD_U

	print_nonzero_bytes(tcp, tprint_array_next, addr, sizeof(sockopt),
			    MIN(len, get_pagesize()), QUOTE_FORCE_HEX);

	return true;
}

static const nla_decoder_t inet_diag_msg_nla_decoders[] = {
	[INET_DIAG_MEMINFO]		= decode_inet_diag_meminfo,
	[INET_DIAG_INFO]		= NULL,		/* unimplemented */
	[INET_DIAG_VEGASINFO]		= decode_tcpvegas_info,
	[INET_DIAG_CONG]		= decode_nla_str,
	[INET_DIAG_TOS]			= decode_nla_u8,
	[INET_DIAG_TCLASS]		= decode_nla_u8,
	[INET_DIAG_SKMEMINFO]		= decode_nla_meminfo,
	[INET_DIAG_SHUTDOWN]		= decode_diag_shutdown,
	[INET_DIAG_DCTCPINFO]		= decode_tcp_dctcp_info,
	[INET_DIAG_PROTOCOL]		= decode_nla_ip_proto,
	[INET_DIAG_SKV6ONLY]		= decode_nla_u8,
	[INET_DIAG_LOCALS]		= decode_nla_sockaddrs,
	[INET_DIAG_PEERS]		= decode_nla_sockaddrs,
	[INET_DIAG_PAD]			= NULL,
	[INET_DIAG_MARK]		= decode_nla_u32,
	[INET_DIAG_BBRINFO]		= decode_tcp_bbr_info,
	[INET_DIAG_CLASS_ID]		= decode_nla_u32,
	[INET_DIAG_MD5SIG]		= decode_tcp_md5sig,
	[INET_DIAG_ULP_INFO]		= decode_diag_ulp_info,
	[INET_DIAG_SK_BPF_STORAGES]	= decode_diag_bpf_storages,
	[INET_DIAG_CGROUP_ID]		= decode_nla_u64,
	[INET_DIAG_SOCKOPT]		= decode_diag_sockopt,
};

DECL_NETLINK_DIAG_DECODER(decode_inet_diag_msg)
{
	struct inet_diag_msg msg = { .idiag_family = family };
	size_t offset = sizeof(msg.idiag_family);
	bool decode_nla = false;

	tprint_struct_begin();
	PRINT_FIELD_XVAL(msg, idiag_family, addrfams, "AF_???");
	tprint_struct_next();

	if (len >= sizeof(msg)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(msg) - offset,
					 (char *) &msg + offset)) {
			PRINT_FIELD_XVAL(msg, idiag_state,
					 tcp_states, "TCP_???");
			tprint_struct_next();
			PRINT_FIELD_U(msg, idiag_timer);
			tprint_struct_next();
			PRINT_FIELD_U(msg, idiag_retrans);
			tprint_struct_next();
			PRINT_FIELD_INET_DIAG_SOCKID(msg, id,
						     msg.idiag_family);
			tprint_struct_next();
			PRINT_FIELD_U(msg, idiag_expires);
			tprint_struct_next();
			PRINT_FIELD_U(msg, idiag_rqueue);
			tprint_struct_next();
			PRINT_FIELD_U(msg, idiag_wqueue);
			tprint_struct_next();
			PRINT_FIELD_U(msg, idiag_uid);
			tprint_struct_next();
			PRINT_FIELD_U(msg, idiag_inode);
			decode_nla = true;
		}
	} else
		tprint_more_data_follows();
	tprint_struct_end();

	offset = NLMSG_ALIGN(sizeof(msg));
	if (decode_nla && len > offset) {
		tprint_array_next();
		decode_nlattr(tcp, addr + offset, len - offset,
			      inet_diag_attrs, "INET_DIAG_???",
			      inet_diag_msg_nla_decoders,
			      ARRAY_SIZE(inet_diag_msg_nla_decoders), NULL);
	}
}
