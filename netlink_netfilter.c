/*
 * Copyright (c) 2018 Chen Jingpiao <chenjingpiao@gmail.com>
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

#ifdef HAVE_LINUX_NETFILTER_NFNETLINK_H

# include "print_fields.h"
# include "nlattr.h"

# include <netinet/in.h>
# include <arpa/inet.h>
# include "netlink.h"
# include <linux/netfilter/nfnetlink.h>

# include "xlat/icmp_codes_redirect.h"
# include "xlat/icmp_codes_time_exceeded.h"
# include "xlat/icmp_codes_unreach.h"
# include "xlat/icmp_types.h"
# include "xlat/icmpv6_codes_parameter.h"
# include "xlat/icmpv6_codes_time_exceeded.h"
# include "xlat/icmpv6_codes_unreach.h"
# include "xlat/icmpv6_types.h"
# include "xlat/netfilter_versions.h"
# include "xlat/nf_acct_attr_names.h"
# include "xlat/nf_cthelper_attr_names.h"
# include "xlat/nf_ctnetlink_attr_names.h"
# include "xlat/nf_ctnetlink_exp_attr_names.h"
# include "xlat/nf_ctnetlink_to_attr_names.h"
# include "xlat/nf_ipset_attr_names.h"
# include "xlat/nf_nft_compat_attr_names.h"
# include "xlat/nf_osf_attr_names.h"
# include "xlat/nf_queue_attr_names.h"
# include "xlat/nf_ulog_attr_names.h"
# include "xlat/nfnl_ct_ip_attrs.h"
# include "xlat/nfnl_ct_proto_attrs.h"
# include "xlat/nfnl_ct_protoinfo_attrs.h"
# include "xlat/nfnl_ct_protoinfo_dccp_attrs.h"
# include "xlat/nfnl_ct_protoinfo_sctp_attrs.h"
# include "xlat/nfnl_ct_protoinfo_tcp_attrs.h"
# include "xlat/nfnl_ct_status.h"
# include "xlat/nfnl_ct_tcp_flags.h"
# include "xlat/nfnl_ct_tcp_states.h"
# include "xlat/nfnl_ct_tuple_attrs.h"
# include "xlat/nfnl_zones.h"
# include "xlat/nft_chain_attr_names.h"
# include "xlat/nft_flow_attr_names.h"
# include "xlat/nft_gen_attr_names.h"
# include "xlat/nft_obj_attr_names.h"
# include "xlat/nft_rule_attr_names.h"
# include "xlat/nft_set_attr_names.h"
# include "xlat/nft_setelem_attr_names.h"
# include "xlat/nft_table_attr_names.h"
# include "xlat/nft_trace_attr_names.h"
# include "xlat/nl_netfilter_msg_types.h"
# include "xlat/nl_netfilter_subsys_ids.h"


# define XLAT_MACROS_ONLY
#  include "xlat/nl_netfilter_subsys_ids.h"
#  include "xlat/nf_nftables_msg_types.h"
# undef XLAT_MACROS_ONLY

struct nfnl_decoder {
	const struct xlat *name_xlat;
	const char *dflt;

	const nla_decoder_t *decoders;
	size_t decoders_sz;

	const struct nfnl_decoder *subdecoder;
	size_t subdecoder_sz;
};

static const struct nfnl_decoder nft_subsystem_decoders[] = {
	[NFT_MSG_NEWTABLE] =
		{ nft_table_attr_names,		"NFTA_TABLE_???", },
	[NFT_MSG_GETTABLE] =
		{ nft_table_attr_names,		"NFTA_TABLE_???", },
	[NFT_MSG_DELTABLE] =
		{ nft_table_attr_names,		"NFTA_TABLE_???", },
	[NFT_MSG_NEWCHAIN] =
		{ nft_chain_attr_names,		"NFTA_CHAIN_???", },
	[NFT_MSG_GETCHAIN] =
		{ nft_chain_attr_names,		"NFTA_CHAIN_???", },
	[NFT_MSG_DELCHAIN] =
		{ nft_chain_attr_names,		"NFTA_CHAIN_???", },
	[NFT_MSG_NEWRULE] =
		{ nft_rule_attr_names,		"NFTA_RULE_???", },
	[NFT_MSG_GETRULE] =
		{ nft_rule_attr_names,		"NFTA_RULE_???", },
	[NFT_MSG_DELRULE] =
		{ nft_rule_attr_names,		"NFTA_RULE_???", },
	[NFT_MSG_NEWSET] =
		{ nft_set_attr_names,		"NFTA_SET_???", },
	[NFT_MSG_GETSET] =
		{ nft_set_attr_names,		"NFTA_SET_???", },
	[NFT_MSG_DELSET] =
		{ nft_set_attr_names,		"NFTA_SET_???", },
	[NFT_MSG_NEWSETELEM] =
		{ nft_setelem_attr_names,	"NFTA_SET_ELEM_???", },
	[NFT_MSG_GETSETELEM] =
		{ nft_setelem_attr_names,	"NFTA_SET_ELEM_???", },
	[NFT_MSG_DELSETELEM] =
		{ nft_setelem_attr_names,	"NFTA_SET_ELEM_???", },
	[NFT_MSG_NEWGEN] =
		{ nft_gen_attr_names,		"NFTA_GEN_???", },
	[NFT_MSG_GETGEN] =
		{ nft_gen_attr_names,		"NFTA_GEN_???", },
	[NFT_MSG_TRACE] =
		{ nft_trace_attr_names,		"NFTA_TRACE_???", },
	[NFT_MSG_NEWOBJ] =
		{ nft_obj_attr_names,		"NFTA_OBJ_???", },
	[NFT_MSG_GETOBJ] =
		{ nft_obj_attr_names,		"NFTA_OBJ_???", },
	[NFT_MSG_DELOBJ] =
		{ nft_obj_attr_names,		"NFTA_OBJ_???", },
	[NFT_MSG_GETOBJ_RESET] =
		{ nft_obj_attr_names,		"NFTA_OBJ_???", },
	[NFT_MSG_NEWFLOWTABLE] =
		{ nft_flow_attr_names,		"NFTA_FLOW_???", },
	[NFT_MSG_GETFLOWTABLE] =
		{ nft_flow_attr_names,		"NFTA_FLOW_???", },
	[NFT_MSG_DELFLOWTABLE] =
		{ nft_flow_attr_names,		"NFTA_FLOW_???", },
};

static const nla_decoder_t nfnl_ct_ip_decoders[] = {
	[CTA_IP_V4_SRC]		= decode_nla_in_addr,
	[CTA_IP_V4_DST]		= decode_nla_in_addr,
	[CTA_IP_V6_SRC]		= decode_nla_in6_addr,
	[CTA_IP_V6_DST]		= decode_nla_in6_addr,
};

static bool
decode_cta_ip(struct tcb *const tcp,
	      const kernel_ulong_t addr,
	      const unsigned int len,
	      const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, nfnl_ct_ip_attrs, "CTA_IP_???",
		      ARRSZ_PAIR(nfnl_ct_ip_decoders), opaque_data);

	return true;
}

struct cta_proto_ctx {
	uint8_t icmp_type_seen   :1,
		icmpv6_type_seen :1;

	uint8_t icmp_type;
	uint8_t icmpv6_type;
};

static bool
decode_cta_icmp_type(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const unsigned int len,
		     const void *const opaque_data)
{
	struct cta_proto_ctx *ctx = (void *) opaque_data;
	uint8_t type;

	if (len < sizeof(type))
		return false;
	if (!umove_or_printaddr(tcp, addr, &type))
		printxval(icmp_types, type, "ICMP_???");

	if (ctx) {
		ctx->icmp_type_seen = 1;
		ctx->icmp_type = type;
	}

	return true;
}

static bool
decode_cta_icmp_code(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const unsigned int len,
		     const void *const opaque_data)
{
	static const struct {
		uint16_t type;
		const struct xlat *xlat;
		const char *dflt;
	} codes[] = {
		{ ICMP_DEST_UNREACH,	icmp_codes_unreach,
			"ICMP_???" },
		{ ICMP_REDIRECT,	icmp_codes_redirect,
			"ICMP_REDIR_???" },
		{ ICMP_TIME_EXCEEDED,	icmp_codes_time_exceeded,
			"ICMP_EXC_???" },
	};

	struct cta_proto_ctx *ctx = (void *) opaque_data;
	uint8_t code;

	if (len < sizeof(code))
		return false;
	if (umove_or_printaddr(tcp, addr, &code))
		return true;

	if (!ctx || !ctx->icmp_type_seen)
		goto type_not_found;

	for (size_t i = 0; i < ARRAY_SIZE(codes); i++) {
		if (codes[i].type == ctx->icmp_type) {
			printxval(codes[i].xlat, code, codes[i].dflt);
			return true;
		}
	}

type_not_found:
	tprintf("%#hhx", code);

	return true;
}

static bool
decode_cta_icmpv6_type(struct tcb *const tcp,
		       const kernel_ulong_t addr,
		       const unsigned int len,
		       const void *const opaque_data)
{
	struct cta_proto_ctx *ctx = (void *) opaque_data;
	uint8_t type;

	if (len < sizeof(type))
		return false;
	if (!umove_or_printaddr(tcp, addr, &type))
		printxval(icmpv6_types, type, "ICMPV6_???");

	if (ctx) {
		ctx->icmpv6_type_seen = 1;
		ctx->icmpv6_type = type;
	}

	return true;
}

static bool
decode_cta_icmpv6_code(struct tcb *const tcp,
		       const kernel_ulong_t addr,
		       const unsigned int len,
		       const void *const opaque_data)
{
	static const struct code_data {
		const struct xlat *xlat;
		const char *dflt;
	} codes[] = {
		[ICMPV6_DEST_UNREACH] =
			{ icmpv6_codes_unreach,		"ICMPV6_???" },
		[ICMPV6_TIME_EXCEED] =
			{ icmpv6_codes_time_exceeded,	"ICMPV6_EXC_???" },
		[ICMPV6_PARAMPROB] =
			{ icmpv6_codes_parameter,	"ICMPV6_???" },
	};

	struct cta_proto_ctx *ctx = (void *) opaque_data;
	uint8_t code;

	if (len < sizeof(code))
		return false;
	if (umove_or_printaddr(tcp, addr, &code))
		return true;

	if (!ctx || !ctx->icmpv6_type_seen
	    || ctx->icmpv6_type >= ARRAY_SIZE(codes)
	    || !codes[ctx->icmpv6_type].xlat) {
		tprintf("%#hhx", code);
		return true;
	}

	const struct code_data *c = codes + ctx->icmpv6_type;
	printxval(c->xlat, code, c->dflt);

	return true;
}

static const nla_decoder_t nfnl_ct_proto_decoders[] = {
	[CTA_PROTO_NUM]		= decode_nla_ip_proto,
	[CTA_PROTO_SRC_PORT]	= decode_nla_be16,
	[CTA_PROTO_DST_PORT]	= decode_nla_be16,
	[CTA_PROTO_ICMP_ID]	= decode_nla_be16,
	[CTA_PROTO_ICMP_TYPE]	= decode_cta_icmp_type,
	[CTA_PROTO_ICMP_CODE]	= decode_cta_icmp_code,
	[CTA_PROTO_ICMPV6_ID]	= decode_nla_be16,
	[CTA_PROTO_ICMPV6_TYPE]	= decode_cta_icmpv6_type,
	[CTA_PROTO_ICMPV6_CODE]	= decode_cta_icmpv6_code,
};

static bool
decode_cta_proto(struct tcb *const tcp,
		 const kernel_ulong_t addr,
		 const unsigned int len,
		 const void *const opaque_data)
{
	struct cta_proto_ctx ctx = { 0 };

	decode_nlattr(tcp, addr, len, nfnl_ct_proto_attrs, "CTA_PROTO_???",
		      ARRSZ_PAIR(nfnl_ct_proto_decoders), &ctx);

	return true;
}


static const nla_decoder_t nfnl_ct_tuple_decoders[] = {
	[CTA_TUPLE_IP]		= decode_cta_ip,
	[CTA_TUPLE_PROTO]	= decode_cta_proto,
	[CTA_TUPLE_ZONE]	= decode_nla_be16,
};

static bool
decode_cta_tuple(struct tcb *const tcp,
		 const kernel_ulong_t addr,
		 const unsigned int len,
		 const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, nfnl_ct_tuple_attrs, "CTA_TUPLE_???",
		      ARRSZ_PAIR(nfnl_ct_tuple_decoders), opaque_data);

	return true;
}

static bool
decode_cta_status(struct tcb *const tcp,
		  const kernel_ulong_t addr,
		  const unsigned int len,
		  const void *const opaque_data)
{
	uint32_t status;

	if (len < sizeof(status))
		return false;
	if (umove_or_printaddr(tcp, addr, &status))
		return true;

	status = ntohs(status);
	tprints("htons(");
	printflags(nfnl_ct_status, status, "IPS_???");
	tprints(")");

	return true;
}

static bool
decode_cta_tcp_state(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const unsigned int len,
		     const void *const opaque_data)
{
	uint32_t state;

	if (len < sizeof(state))
		return false;
	if (!umove_or_printaddr(tcp, addr, &state))
		printxval(nfnl_ct_tcp_states, state, "TCP_CONNTRACK_???");

	return true;
}

static bool
decode_cta_tcp_flags(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const unsigned int len,
		     const void *const opaque_data)
{
	uint8_t flags[2];

	if (len < sizeof(flags))
		return false;
	if (umove_or_printaddr(tcp, addr, &flags))
		return true;

	tprints("{flags=");
	printflags(nfnl_ct_tcp_flags, flags[0], "IP_CT_???");
	tprints(", mask=");
	printflags(nfnl_ct_tcp_flags, flags[1], "IP_CT_???");
	tprints("}");

	return true;
}

static const nla_decoder_t nfnl_ct_protoinfo_tcp_decoders[] = {
	[CTA_PROTOINFO_TCP_STATE]		= decode_cta_tcp_state,
	[CTA_PROTOINFO_TCP_WSCALE_ORIGINAL]	= decode_nla_u8,
	[CTA_PROTOINFO_TCP_WSCALE_REPLY]	= decode_nla_u8,
	[CTA_PROTOINFO_TCP_FLAGS_ORIGINAL]	= decode_cta_tcp_flags,
	[CTA_PROTOINFO_TCP_FLAGS_REPLY]		= decode_cta_tcp_flags,
};

static bool
decode_cta_protoinfo_tcp(struct tcb *const tcp,
			 const kernel_ulong_t addr,
			 const unsigned int len,
			 const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, nfnl_ct_protoinfo_tcp_attrs,
		      "CTA_PROTOINFO_TCP_???",
		      ARRSZ_PAIR(nfnl_ct_protoinfo_tcp_decoders), opaque_data);

	return true;
}

static const nla_decoder_t nfnl_ct_protoinfo_dccp_decoders[] = {
};

static bool
decode_cta_protoinfo_dccp(struct tcb *const tcp,
			  const kernel_ulong_t addr,
			  const unsigned int len,
			  const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, nfnl_ct_protoinfo_dccp_attrs,
		      "CTA_PROTOINFO_DCCP_???",
		      ARRSZ_PAIR(nfnl_ct_protoinfo_dccp_decoders), opaque_data);

	return true;
}

static const nla_decoder_t nfnl_ct_protoinfo_sctp_decoders[] = {
};

static bool
decode_cta_protoinfo_sctp(struct tcb *const tcp,
			  const kernel_ulong_t addr,
			  const unsigned int len,
			  const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, nfnl_ct_protoinfo_sctp_attrs,
		      "CTA_PROTOINFO_SCTP_???",
		      ARRSZ_PAIR(nfnl_ct_protoinfo_sctp_decoders), opaque_data);

	return true;
}

static const nla_decoder_t nfnl_ct_protoinfo_decoders[] = {
	[CTA_PROTOINFO_TCP]	= decode_cta_protoinfo_tcp,
	[CTA_PROTOINFO_DCCP]	= decode_cta_protoinfo_dccp,
	[CTA_PROTOINFO_SCTP]	= decode_cta_protoinfo_sctp,
};

static bool
decode_cta_protoinfo(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const unsigned int len,
		     const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, nfnl_ct_protoinfo_attrs,
		      "CTA_PROTOINFO_???",
		      ARRSZ_PAIR(nfnl_ct_protoinfo_decoders), opaque_data);

	return true;
}

static const nla_decoder_t nf_ctnetlink_decoders[] = {
	[CTA_TUPLE_ORIG]	= decode_cta_tuple,
	[CTA_TUPLE_REPLY]	= decode_cta_tuple,
	[CTA_STATUS]		= decode_cta_status,
	[CTA_PROTOINFO]		= decode_cta_protoinfo,
	[CTA_HELP]		= decode_nla_u32,
	[CTA_NAT_SRC]		= decode_nla_u32,
	[CTA_TIMEOUT]		= decode_nla_u32,
	[CTA_MARK]		= decode_nla_u32,
	[CTA_COUNTERS_ORIG]	= decode_nla_u32,
	[CTA_COUNTERS_REPLY]	= decode_nla_u32,
	[CTA_USE]		= decode_nla_u32,
	[CTA_ID]		= decode_nla_u32,
	[CTA_NAT_DST]		= decode_nla_u32,
	[CTA_TUPLE_MASTER]	= decode_nla_u32,
	[CTA_SEQ_ADJ_ORIG]	= decode_nla_u32,
	[CTA_SEQ_ADJ_REPLY]	= decode_nla_u32,
	[CTA_SECMARK]		= decode_nla_u32,
	[CTA_ZONE]		= decode_nla_u32,
	[CTA_SECCTX]		= decode_nla_u32,
	[CTA_TIMESTAMP]		= decode_nla_u32,
	[CTA_MARK_MASK]		= decode_nla_u32,
	[CTA_LABELS]		= decode_nla_u32,
	[CTA_LABELS_MASK]	= decode_nla_u32,
	[CTA_SYNPROXY]		= decode_nla_u32,
};

static const struct nfnl_decoder nfnl_subsystems[] = {
	[NFNL_SUBSYS_CTNETLINK] =
		{ nf_ctnetlink_attr_names,	"CTA_???",
		  ARRSZ_PAIR(nf_ctnetlink_decoders) },
	[NFNL_SUBSYS_CTNETLINK_EXP] =
		{ nf_ctnetlink_exp_attr_names,	"CTA_EXPECT_???", },
	[NFNL_SUBSYS_QUEUE] =
		{ nf_queue_attr_names,		"NFQA_???", },
	[NFNL_SUBSYS_ULOG] =
		{ nf_ulog_attr_names,		"NFULA_???", },
	[NFNL_SUBSYS_OSF] =
		{ nf_osf_attr_names,		"OSF_???", },
	[NFNL_SUBSYS_IPSET] =
		{ nf_ipset_attr_names,		"IPSET_ATTR_???", },
	[NFNL_SUBSYS_ACCT] =
		{ nf_acct_attr_names,		"NFACCT_???", },
	[NFNL_SUBSYS_CTNETLINK_TIMEOUT] =
		{ nf_ctnetlink_to_attr_names,	"CTA_TIMEOUT_???", },
	[NFNL_SUBSYS_CTHELPER] =
		{ nf_cthelper_attr_names,	"NFCTH_???" },
	[NFNL_SUBSYS_NFTABLES] =
		{ NULL, "NFT_???", NULL, 0,
		  ARRSZ_PAIR(nft_subsystem_decoders) },
	[NFNL_SUBSYS_NFT_COMPAT] =
		{ nf_nft_compat_attr_names,	"NFTA_COMPAT_???" },
};

bool
decode_netlink_netfilter(struct tcb *const tcp,
			 const struct nlmsghdr *const nlmsghdr,
			 const kernel_ulong_t addr,
			 const unsigned int len)
{
	if (nlmsghdr->nlmsg_type == NLMSG_DONE)
		return false;

	struct nfgenmsg nfmsg;

	if (len < sizeof(nfmsg))
		printstr_ex(tcp, addr, len, QUOTE_FORCE_HEX);
	else if (!umove_or_printaddr(tcp, addr, &nfmsg)) {
		const uint8_t subsys_id = nlmsghdr->nlmsg_type >> 8;
		const uint8_t msg_type = nlmsghdr->nlmsg_type;
		uint16_t res_id = ntohs(nfmsg.res_id);

		PRINT_FIELD_XVAL("{", nfmsg, nfgen_family, addrfams, "AF_???");
		PRINT_FIELD_XVAL(", ", nfmsg, version, netfilter_versions,
				 "NFNETLINK_???");

		/*
		 * Work around wrong endianness in res_id field,
		 * see linux commit v4.3-rc1~28^2~47^2~1
		 */
		tprints(", res_id=");
		if (subsys_id == NFNL_SUBSYS_NFTABLES
		    && res_id == NFNL_SUBSYS_NFTABLES) {
			print_xlat_ex(nfmsg.res_id,
				      "htons(NFNL_SUBSYS_NFTABLES)",
				      XLAT_STYLE_DEFAULT);
		} else if (subsys_id == NFNL_SUBSYS_NFTABLES
			   && nfmsg.res_id == NFNL_SUBSYS_NFTABLES) {
			print_xlat_ex(nfmsg.res_id, "NFNL_SUBSYS_NFTABLES",
				      XLAT_STYLE_DEFAULT);
		} else {
			tprintf("htons(%d)", res_id);
		}

		const size_t offset = NLMSG_ALIGN(sizeof(nfmsg));
		if (len > offset) {
			tprints(", ");
			if ((nlmsghdr->nlmsg_type >= NFNL_MSG_BATCH_BEGIN
			     && nlmsghdr->nlmsg_type <= NFNL_MSG_BATCH_END)
			    || nlmsghdr->nlmsg_type < NLMSG_MIN_TYPE) {
				printstr_ex(tcp, addr + offset,
					    len - offset, QUOTE_FORCE_HEX);
			} else {
				static const struct nfnl_decoder def;
				const struct nfnl_decoder *subsys = &def;

				if (subsys_id < ARRAY_SIZE(nfnl_subsystems))
					subsys = nfnl_subsystems + subsys_id;
				if (subsys->subdecoder
				    && (msg_type < subsys->subdecoder_sz))
					subsys = subsys->subdecoder + msg_type;

				decode_nlattr(tcp, addr + offset, len - offset,
					      subsys->name_xlat, subsys->dflt,
					      subsys->decoders,
					      subsys->decoders_sz, NULL);
			}
		}
	}

	return true;
}

#endif /* HAVE_LINUX_NETFILTER_NFNETLINK_H */
