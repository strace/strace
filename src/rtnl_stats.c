/*
 * Copyright (c) 2018-2021 Eugene Syromyatnikov <evgsyr@gmail.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "netlink_route.h"
#include "nlattr.h"
#include "print_fields.h"

#include "netlink.h"

#include <netinet/in.h>

#include <linux/if_bonding.h>
#include <linux/if_bridge.h>
#include <linux/if_link.h>
#include <linux/mpls.h>
#include <linux/rtnetlink.h>

#include "xlat/ifstats_af_spec_mpls_attrs.h"
#include "xlat/ifstats_attrs.h"
#include "xlat/ifstats_attr_flags.h"
#include "xlat/ifstats_offload_attrs.h"
#include "xlat/ifstats_xstats_bond_attrs.h"
#include "xlat/ifstats_xstats_bond_3ad_attrs.h"
#include "xlat/ifstats_xstats_bridge_attrs.h"
#include "xlat/ifstats_xstats_bridge_mcast_indices.h"
#include "xlat/ifstats_xstats_type_attrs.h"
#include "xlat/nl_bridge_vlan_flags.h"

#define XLAT_MACROS_ONLY
# include "xlat/addrfams.h" /* AF_MPLS */
#undef XLAT_MACROS_ONLY

static bool
decode_ifstats_link_xstats_bridge_vlan(struct tcb *const tcp,
				       const kernel_ulong_t addr,
				       const unsigned int len,
				       const void *const opaque_data)
{
	struct bridge_vlan_xstats st;

	if (len < sizeof(st))
		return false;

	if (umove_or_printaddr(tcp, addr, &st))
		return true;

	tprint_struct_begin();
	PRINT_FIELD_U(st, rx_bytes);
	tprint_struct_next();
	PRINT_FIELD_U(st, rx_packets);
	tprint_struct_next();
	PRINT_FIELD_U(st, tx_bytes);
	tprint_struct_next();
	PRINT_FIELD_U(st, tx_packets);
	tprint_struct_next();
	PRINT_FIELD_U(st, vid);
	tprint_struct_next();
	PRINT_FIELD_FLAGS(st, flags, nl_bridge_vlan_flags,
			  "BRIDGE_VLAN_INFO_???");
	if (st.pad2) {
		tprint_struct_next();
		PRINT_FIELD_X(st, pad2);
	}
	tprint_struct_end();

	if (len > sizeof(st)) {
		tprint_array_next();
		printstr_ex(tcp, addr + sizeof(st), len - sizeof(st),
			    QUOTE_FORCE_HEX);
	}

	return true;
}

static bool
decode_ifstats_link_xstats_bridge_mcast(struct tcb *const tcp,
					const kernel_ulong_t addr,
					const unsigned int len,
					const void *const opaque_data)
{
	struct br_mcast_stats st;

	if (len < sizeof(st))
		return false;

	if (umove_or_printaddr(tcp, addr, &st))
		return true;

#define PRINT_FIELD_MCAST_ARRAY_(where_, field_)			\
	PRINT_FIELD_ARRAY_INDEXED(where_, field_,			\
				  tcp, print_uint_array_member,		\
				  ifstats_xstats_bridge_mcast_indices,	\
				  NULL);

	tprint_struct_begin();
	PRINT_FIELD_MCAST_ARRAY_(st, igmp_v1queries);
	tprint_struct_next();
	PRINT_FIELD_MCAST_ARRAY_(st, igmp_v2queries);
	tprint_struct_next();
	PRINT_FIELD_MCAST_ARRAY_(st, igmp_v3queries);
	tprint_struct_next();
	PRINT_FIELD_MCAST_ARRAY_(st, igmp_leaves);
	tprint_struct_next();
	PRINT_FIELD_MCAST_ARRAY_(st, igmp_v1reports);
	tprint_struct_next();
	PRINT_FIELD_MCAST_ARRAY_(st, igmp_v2reports);
	tprint_struct_next();
	PRINT_FIELD_MCAST_ARRAY_(st, igmp_v3reports);
	tprint_struct_next();
	PRINT_FIELD_U(st, igmp_parse_errors);
	tprint_struct_next();
	PRINT_FIELD_MCAST_ARRAY_(st, mld_v1queries);
	tprint_struct_next();
	PRINT_FIELD_MCAST_ARRAY_(st, mld_v2queries);
	tprint_struct_next();
	PRINT_FIELD_MCAST_ARRAY_(st, mld_leaves);
	tprint_struct_next();
	PRINT_FIELD_MCAST_ARRAY_(st, mld_v1reports);
	tprint_struct_next();
	PRINT_FIELD_MCAST_ARRAY_(st, mld_v2reports);
	tprint_struct_next();
	PRINT_FIELD_U(st, mld_parse_errors);
	tprint_struct_next();
	PRINT_FIELD_MCAST_ARRAY_(st, mcast_bytes);
	tprint_struct_next();
	PRINT_FIELD_MCAST_ARRAY_(st, mcast_packets);
	tprint_struct_end();

#undef PRINT_FIELD_MCAST_ARRAY_

	if (len > sizeof(st)) {
		tprint_array_next();
		printstr_ex(tcp, addr + sizeof(st), len - sizeof(st),
			    QUOTE_FORCE_HEX);
	}

	return true;
}

static bool
decode_ifstats_link_xstats_bridge_stp(struct tcb *const tcp,
				      const kernel_ulong_t addr,
				      const unsigned int len,
				      const void *const opaque_data)
{
	struct bridge_stp_xstats st;

	if (len < sizeof(st))
		return false;

	if (umove_or_printaddr(tcp, addr, &st))
		return true;

	tprint_struct_begin();
	PRINT_FIELD_U(st, transition_blk);
	tprint_struct_next();
	PRINT_FIELD_U(st, transition_fwd);
	tprint_struct_next();
	PRINT_FIELD_U(st, rx_bpdu);
	tprint_struct_next();
	PRINT_FIELD_U(st, tx_bpdu);
	tprint_struct_next();
	PRINT_FIELD_U(st, rx_tcn);
	tprint_struct_next();
	PRINT_FIELD_U(st, tx_tcn);
	tprint_struct_end();

	if (len > sizeof(st)) {
		tprint_array_next();
		printstr_ex(tcp, addr + sizeof(st), len - sizeof(st),
			    QUOTE_FORCE_HEX);
	}

	return true;
}

static const nla_decoder_t ifstats_xstats_bridge_decoders[] = {
	[BRIDGE_XSTATS_UNSPEC]	= NULL,
	[BRIDGE_XSTATS_VLAN]	= decode_ifstats_link_xstats_bridge_vlan,
	[BRIDGE_XSTATS_MCAST]	= decode_ifstats_link_xstats_bridge_mcast,
	[BRIDGE_XSTATS_PAD]	= NULL,
	[BRIDGE_XSTATS_STP]	= decode_ifstats_link_xstats_bridge_stp,
};

static bool
decode_ifstats_link_xstats_bridge(struct tcb *const tcp,
				  const kernel_ulong_t addr,
				  const unsigned int len,
				  const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, ifstats_xstats_bridge_attrs,
		      "BRIDGE_XSTATS_???",
		      ARRSZ_PAIR(ifstats_xstats_bridge_decoders),
		      opaque_data);

	return true;
}

static const nla_decoder_t ifstats_xstats_bond_3ad_decoders[] = {
	[BOND_3AD_STAT_LACPDU_RX]		= decode_nla_u64,
	[BOND_3AD_STAT_LACPDU_TX]		= decode_nla_u64,
	[BOND_3AD_STAT_LACPDU_UNKNOWN_RX]	= decode_nla_u64,
	[BOND_3AD_STAT_LACPDU_ILLEGAL_RX]	= decode_nla_u64,
	[BOND_3AD_STAT_MARKER_RX]		= decode_nla_u64,
	[BOND_3AD_STAT_MARKER_TX]		= decode_nla_u64,
	[BOND_3AD_STAT_MARKER_RESP_RX]		= decode_nla_u64,
	[BOND_3AD_STAT_MARKER_RESP_TX]		= decode_nla_u64,
	[BOND_3AD_STAT_MARKER_UNKNOWN_RX]	= decode_nla_u64,
	[BOND_3AD_STAT_PAD]			= NULL,
};

static bool
decode_ifstats_link_xstats_bond_3ad(struct tcb *const tcp,
				    const kernel_ulong_t addr,
				    const unsigned int len,
				    const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, ifstats_xstats_bond_3ad_attrs,
		      "BOND_XSTATS_???",
		      ARRSZ_PAIR(ifstats_xstats_bond_3ad_decoders),
		      opaque_data);

	return true;
}

static const nla_decoder_t ifstats_xstats_bond_decoders[] = {
	[BOND_XSTATS_UNSPEC]	= NULL,
	[BOND_XSTATS_3AD]	= decode_ifstats_link_xstats_bond_3ad,
};

static bool
decode_ifstats_link_xstats_bond(struct tcb *const tcp,
				const kernel_ulong_t addr,
				const unsigned int len,
				const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, ifstats_xstats_bond_attrs,
		      "BOND_XSTATS_???",
		      ARRSZ_PAIR(ifstats_xstats_bond_decoders),
		      opaque_data);

	return true;
}

static const nla_decoder_t ifstats_xstats_decoders[] = {
	[LINK_XSTATS_TYPE_UNSPEC]	= NULL,
	[LINK_XSTATS_TYPE_BRIDGE]	= decode_ifstats_link_xstats_bridge,
	[LINK_XSTATS_TYPE_BOND]		= decode_ifstats_link_xstats_bond,
};

static bool
decode_ifstats_link_xstats(struct tcb *const tcp,
			   const kernel_ulong_t addr,
			   const unsigned int len,
			   const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, ifstats_xstats_type_attrs,
		      "LINK_XSTATS_TYPE_???",
		      ARRSZ_PAIR(ifstats_xstats_decoders),
		      opaque_data);

	return true;
}

static const nla_decoder_t ifstats_offload_xstats_decoders[] = {
	[IFLA_OFFLOAD_XSTATS_UNSPEC]	= NULL,
	[IFLA_OFFLOAD_XSTATS_CPU_HIT]	= decode_nla_rtnl_link_stats64,
};

static bool
decode_ifstats_link_offload_xstats(struct tcb *const tcp,
				   const kernel_ulong_t addr,
				   const unsigned int len,
				   const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, ifstats_offload_attrs,
		      "IFLA_OFFLOAD_XSTATS_???",
		      ARRSZ_PAIR(ifstats_offload_xstats_decoders),
		      opaque_data);

	return true;
}

static bool
decode_ifstats_af_mpls_stats_link(struct tcb *const tcp,
				  const kernel_ulong_t addr,
				  const unsigned int len,
				  const void *const opaque_data)
{
	struct mpls_link_stats st;

	if (len < sizeof(st))
		return false;

	if (umove_or_printaddr(tcp, addr, &st))
		return true;

	tprint_struct_begin();
	PRINT_FIELD_U(st, rx_packets);
	tprint_struct_next();
	PRINT_FIELD_U(st, tx_packets);
	tprint_struct_next();
	PRINT_FIELD_U(st, rx_bytes);
	tprint_struct_next();
	PRINT_FIELD_U(st, tx_bytes);
	tprint_struct_next();
	PRINT_FIELD_U(st, rx_errors);
	tprint_struct_next();
	PRINT_FIELD_U(st, tx_errors);
	tprint_struct_next();
	PRINT_FIELD_U(st, rx_dropped);
	tprint_struct_next();
	PRINT_FIELD_U(st, tx_dropped);
	tprint_struct_next();
	PRINT_FIELD_U(st, rx_noroute);
	tprint_struct_end();

	if (len > sizeof(st)) {
		tprint_array_next();
		printstr_ex(tcp, addr + sizeof(st), len - sizeof(st),
			    QUOTE_FORCE_HEX);
	}

	return true;
}

static const nla_decoder_t ifla_stats_mpls_nla_decoders[] = {
	[MPLS_STATS_UNSPEC]	= NULL,
	[MPLS_STATS_LINK]	= decode_ifstats_af_mpls_stats_link,
};

static bool
decode_ifstats_af(struct tcb *const tcp,
		  const kernel_ulong_t addr,
		  const unsigned int len,
		  const void *const opaque_data)
{
	static const struct af_spec_decoder_desc protos[] = {
		{ AF_MPLS, ifstats_af_spec_mpls_attrs, "MPLS_STATS_???",
		  ARRSZ_PAIR(ifla_stats_mpls_nla_decoders) },
	};

	decode_nla_af_spec(tcp, addr, len,
			   (uintptr_t) opaque_data, ARRSZ_PAIR(protos));

	return true;
}

static bool
decode_ifstats_af_spec(struct tcb *const tcp,
		       const kernel_ulong_t addr,
		       const unsigned int len,
		       const void *const opaque_data)
{
	static const nla_decoder_t af_spec_decoder = &decode_ifstats_af;

	decode_nlattr(tcp, addr, len, addrfams, "AF_???",
		      &af_spec_decoder, 0, 0);

	return true;
}


static const nla_decoder_t ifstatsmsg_nla_decoders[] = {
	[IFLA_STATS_UNSPEC]			= NULL,
	[IFLA_STATS_LINK_64]			= decode_nla_rtnl_link_stats64,
	[IFLA_STATS_LINK_XSTATS]		= decode_ifstats_link_xstats,
	[IFLA_STATS_LINK_XSTATS_SLAVE]		= decode_ifstats_link_xstats,
	[IFLA_STATS_LINK_OFFLOAD_XSTATS]	= decode_ifstats_link_offload_xstats,
	[IFLA_STATS_AF_SPEC]			= decode_ifstats_af_spec,
};

DECL_NETLINK_ROUTE_DECODER(decode_ifstatsmsg)
{
	struct if_stats_msg ifstats = { .family = family };
	size_t offset = sizeof(ifstats.family);
	bool decode_nla = false;

	tprint_struct_begin();
	PRINT_FIELD_XVAL(ifstats, family, addrfams, "AF_???");
	tprint_struct_next();

	if (len >= sizeof(ifstats)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(ifstats) - offset,
					 (char *) &ifstats + offset)) {
			if (ifstats.pad1) {
				PRINT_FIELD_X(ifstats, pad1);
				tprint_struct_next();
			}
			if (ifstats.pad2) {
				PRINT_FIELD_X(ifstats, pad2);
				tprint_struct_next();
			}
			PRINT_FIELD_IFINDEX(ifstats, ifindex);
			tprint_struct_next();
			PRINT_FIELD_FLAGS(ifstats, filter_mask,
					  ifstats_attr_flags,
					  "1<<IFLA_STATS_???");
			decode_nla = true;
		}
	} else {
		tprint_more_data_follows();
	}
	tprint_struct_end();

	offset = NLMSG_ALIGN(sizeof(ifstats));
	if (decode_nla && len > offset) {
		tprint_array_next();
		decode_nlattr(tcp, addr + offset, len - offset,
			      ifstats_attrs, "IFLA_STATS_???",
			      ARRSZ_PAIR(ifstatsmsg_nla_decoders), &ifstats);
	}
}
