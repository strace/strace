/*
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2016-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "netlink_route.h"
#include "nlattr.h"

#include <linux/ip.h>
#include <linux/rtnetlink.h>

#include "xlat/ip_type_of_services.h"
#include "xlat/lwtunnel_encap_types.h"
#include "xlat/route_nexthop_flags.h"
#include "xlat/routing_flags.h"
#include "xlat/routing_protocols.h"
#include "xlat/routing_table_ids.h"
#include "xlat/routing_types.h"
#include "xlat/rtnl_route_attrs.h"
#include "xlat/rtnl_rta_metrics_attrs.h"

DECL_NLA(rt_class)
{
	uint32_t num;

	if (len < sizeof(num))
		return false;
	if (!umove_or_printaddr(tcp, addr, &num))
		printxval(routing_table_ids, num, NULL);
	return true;
}

DECL_NLA(rt_proto)
{
	uint8_t num;

	if (len < sizeof(num))
		return false;
	if (!umove_or_printaddr(tcp, addr, &num))
		printxval(routing_protocols, num, "RTPROT_???");
	return true;
}

static bool
decode_route_addr(struct tcb *const tcp,
		  const kernel_ulong_t addr,
		  const unsigned int len,
		  const void *const opaque_data)
{
	const struct rtmsg *const rtmsg = opaque_data;

	decode_inet_addr(tcp, addr, len, rtmsg->rtm_family, NULL);

	return true;
}

static const nla_decoder_t rta_metrics_nla_decoders[] = {
	[RTAX_LOCK]		= decode_nla_u32,
	[RTAX_MTU]		= decode_nla_u32,
	[RTAX_WINDOW]		= decode_nla_u32,
	[RTAX_RTT]		= decode_nla_u32,
	[RTAX_RTTVAR]		= decode_nla_u32,
	[RTAX_SSTHRESH]		= decode_nla_u32,
	[RTAX_CWND]		= decode_nla_u32,
	[RTAX_ADVMSS]		= decode_nla_u32,
	[RTAX_REORDERING]	= decode_nla_u32,
	[RTAX_HOPLIMIT]		= decode_nla_u32,
	[RTAX_INITCWND]		= decode_nla_u32,
	[RTAX_FEATURES]		= decode_nla_u32,
	[RTAX_RTO_MIN]		= decode_nla_u32,
	[RTAX_INITRWND]		= decode_nla_u32,
	[RTAX_QUICKACK]		= decode_nla_u32,
	[RTAX_CC_ALGO]		= decode_nla_str
};

static bool
decode_rta_metrics(struct tcb *const tcp,
		   const kernel_ulong_t addr,
		   const unsigned int len,
		   const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, rtnl_rta_metrics_attrs,
		      "RTAX_???", rta_metrics_nla_decoders,
		      ARRAY_SIZE(rta_metrics_nla_decoders), opaque_data);

	return true;
}

static bool
decode_rta_multipath(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const unsigned int len,
		     const void *const opaque_data);

static bool
decode_rta_cacheinfo(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const unsigned int len,
		     const void *const opaque_data)
{
	struct rta_cacheinfo ci;

	if (len < sizeof(ci))
		return false;
	else if (!umove_or_printaddr(tcp, addr, &ci)) {
		tprint_struct_begin();
		PRINT_FIELD_U(ci, rta_clntref);
		tprint_struct_next();
		PRINT_FIELD_U(ci, rta_lastuse);
		tprint_struct_next();
		PRINT_FIELD_U(ci, rta_expires);
		tprint_struct_next();
		PRINT_FIELD_U(ci, rta_error);
		tprint_struct_next();
		PRINT_FIELD_U(ci, rta_used);
		tprint_struct_next();
		PRINT_FIELD_X(ci, rta_id);
		tprint_struct_next();
		PRINT_FIELD_U(ci, rta_ts);
		tprint_struct_next();
		PRINT_FIELD_U(ci, rta_tsage);
		tprint_struct_end();
	}

	return true;
}

static bool
decode_rta_mfc_stats(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const unsigned int len,
		     const void *const opaque_data)
{
	struct rta_mfc_stats mfcs;

	if (len < sizeof(mfcs))
		return false;
	else if (!umove_or_printaddr(tcp, addr, &mfcs)) {
		tprint_struct_begin();
		PRINT_FIELD_U(mfcs, mfcs_packets);
		tprint_struct_next();
		PRINT_FIELD_U(mfcs, mfcs_bytes);
		tprint_struct_next();
		PRINT_FIELD_U(mfcs, mfcs_wrong_if);
		tprint_struct_end();
	}

	return true;
}

static bool
decode_rtvia(struct tcb *const tcp,
	     const kernel_ulong_t addr,
	     const unsigned int len,
	     const void *const opaque_data)
{
	struct rtvia via;

	if (len < sizeof(via))
		return false;
	else if (!umove_or_printaddr(tcp, addr, &via)) {
		tprint_struct_begin();
		PRINT_FIELD_XVAL(via, rtvia_family, addrfams, "AF_???");

		const unsigned int offset = offsetof(struct rtvia, rtvia_addr);

		if (len > offset) {
			tprint_struct_next();
			decode_inet_addr(tcp, addr + offset, len - offset,
					 via.rtvia_family, "rtvia_addr");
		}
		tprint_struct_end();
	}

	return true;
}

DECL_NLA(lwt_encap_type)
{
	uint16_t type;

	if (len < sizeof(type))
		return false;
	else if (!umove_or_printaddr(tcp, addr, &type))
		printxval(lwtunnel_encap_types, type, "LWTUNNEL_ENCAP_???");

	return true;
}

static const nla_decoder_t rtmsg_nla_decoders[] = {
	[RTA_DST]		= decode_route_addr,
	[RTA_SRC]		= decode_route_addr,
	[RTA_IIF]		= decode_nla_ifindex,
	[RTA_OIF]		= decode_nla_ifindex,
	[RTA_GATEWAY]		= decode_route_addr,
	[RTA_PRIORITY]		= decode_nla_u32,
	[RTA_PREFSRC]		= decode_route_addr,
	[RTA_METRICS]		= decode_rta_metrics,
	[RTA_MULTIPATH]		= decode_rta_multipath,
	[RTA_PROTOINFO]		= decode_nla_u32,
	[RTA_FLOW]		= decode_nla_u32,
	[RTA_CACHEINFO]		= decode_rta_cacheinfo,
	[RTA_SESSION]		= NULL, /* unused */
	[RTA_MP_ALGO]		= decode_nla_u32,
	[RTA_TABLE]		= decode_nla_rt_class,
	[RTA_MARK]		= decode_nla_u32,
	[RTA_MFC_STATS]		= decode_rta_mfc_stats,
	[RTA_VIA]		= decode_rtvia,
	[RTA_NEWDST]		= decode_route_addr,
	[RTA_PREF]		= decode_nla_u8,
	[RTA_ENCAP_TYPE]	= decode_nla_lwt_encap_type,
	[RTA_ENCAP]		= NULL, /* unimplemented */
	[RTA_EXPIRES]		= decode_nla_u64,
	[RTA_PAD]		= NULL,
	[RTA_UID]		= decode_nla_u32,
	[RTA_TTL_PROPAGATE]	= decode_nla_u8,
	[RTA_IP_PROTO]		= decode_nla_u8,
	[RTA_SPORT]		= decode_nla_u16,
	[RTA_DPORT]		= decode_nla_u16
};

/*
 * RTA_MULTIPATH payload is a list of struct rtnexthop-headed RTA_* netlink
 * attributes:
 *
 * {RTA_MULTIPATH nlattr hdr} [ [{struct rtnexthop}, {RTA_* nlattr}],
 *                              [{struct rtnexthop}, {RTA_* nlattr}], ... ]
 */
static bool
decode_rta_multipath(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const unsigned int len,
		     const void *const opaque_data)
{
	bool is_array = false;
	struct rtnexthop nh;
	kernel_ulong_t cur = addr;
	kernel_ulong_t left = len;

	if (len < sizeof(nh))
		return false;

	while (!umove_or_printaddr(tcp, cur, &nh)) {
		static const size_t offset = RTNH_ALIGN(sizeof(nh));
		const unsigned int rtnh_len = MIN(left, nh.rtnh_len);

		if (cur == addr && nh.rtnh_len < len) {
			tprint_array_begin();
			is_array = true;
		}

		if (cur > addr)
			tprint_array_next();

		if (rtnh_len > offset)
			tprint_array_begin();

		/* print the whole structure regardless of its rtnh_len */
		tprint_struct_begin();
		PRINT_FIELD_U(nh, rtnh_len);
		tprint_struct_next();
		PRINT_FIELD_FLAGS(nh, rtnh_flags,
				  route_nexthop_flags, "RTNH_F_???");
		tprint_struct_next();
		PRINT_FIELD_U(nh, rtnh_hops);
		tprint_struct_next();
		PRINT_FIELD_IFINDEX(nh, rtnh_ifindex);
		tprint_struct_end();

		if (rtnh_len > offset) {
			tprint_array_next();
			decode_nlattr(tcp, cur + offset, rtnh_len - offset,
				      rtnl_route_attrs, "RTA_???",
				      rtmsg_nla_decoders,
				      ARRAY_SIZE(rtmsg_nla_decoders),
				      opaque_data);
		}

		if (rtnh_len > offset)
			tprint_array_end();

		if (RTNH_ALIGN(rtnh_len) >= left)
			break;

		cur += RTNH_ALIGN(rtnh_len);
		left -= RTNH_ALIGN(rtnh_len);
	}

	if (is_array)
		tprint_array_end();

	return true;
}

DECL_NETLINK_ROUTE_DECODER(decode_rtmsg)
{
	struct rtmsg rtmsg = { .rtm_family = family };
	size_t offset = sizeof(rtmsg.rtm_family);
	bool decode_nla = false;

	tprint_struct_begin();
	PRINT_FIELD_XVAL(rtmsg, rtm_family, addrfams, "AF_???");
	tprint_struct_next();

	if (len >= sizeof(rtmsg)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(rtmsg) - offset,
					 (char *) &rtmsg + offset)) {
			PRINT_FIELD_U(rtmsg, rtm_dst_len);
			tprint_struct_next();
			PRINT_FIELD_U(rtmsg, rtm_src_len);
			tprint_struct_next();
			PRINT_FIELD_FLAGS(rtmsg, rtm_tos,
					  ip_type_of_services, "IPTOS_TOS_???");
			tprint_struct_next();
			PRINT_FIELD_XVAL(rtmsg, rtm_table,
					 routing_table_ids, NULL);
			tprint_struct_next();
			PRINT_FIELD_XVAL(rtmsg, rtm_protocol,
					 routing_protocols, "RTPROT_???");
			tprint_struct_next();
			PRINT_FIELD_XVAL(rtmsg, rtm_scope,
					 routing_scopes, NULL);
			tprint_struct_next();
			PRINT_FIELD_XVAL(rtmsg, rtm_type,
					 routing_types, "RTN_???");
			tprint_struct_next();
			PRINT_FIELD_FLAGS(rtmsg, rtm_flags,
					  routing_flags, "RTM_F_???");
			decode_nla = true;
		}
	} else
		tprint_more_data_follows();
	tprint_struct_end();

	offset = NLMSG_ALIGN(sizeof(rtmsg));
	if (decode_nla && len > offset) {
		tprint_array_next();
		decode_nlattr(tcp, addr + offset, len - offset,
			      rtnl_route_attrs, "RTA_???",
			      rtmsg_nla_decoders,
			      ARRAY_SIZE(rtmsg_nla_decoders), &rtmsg);
	}
}
