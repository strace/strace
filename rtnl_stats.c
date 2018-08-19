/*
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
#include "netlink_route.h"
#include "nlattr.h"
#include "print_fields.h"

#include "netlink.h"

#include <netinet/in.h>

#ifdef HAVE_LINUX_IF_LINK_H
# include <linux/if_link.h>
#endif
#include <linux/rtnetlink.h>

#include "xlat/ifstats_attr_mask.h"
#include "xlat/ifstats_attr.h"
#include "xlat/ifstats_offload_attr.h"
#include "xlat/ifstats_xstats_type_attr.h"
#include "xlat/nl_bridge_vlan_flags.h"

#ifndef HAVE_STRUCT_IF_STATS_MSG
struct if_stats_msg {
	uint8_t	 family;
	uint8_t  pad1;
	uint16_t pad2;
	uint32_t ifindex;
	uint32_t filter_mask;
};
#endif

static bool
decode_nla_xstats_bridge(struct tcb *const tcp,
			 const kernel_ulong_t addr,
			 const unsigned int len,
			 const void *const opaque_data)
{
	struct strace_bridge_vlan_xstats {
		uint64_t rx_bytes;
		uint64_t rx_packets;
		uint64_t tx_bytes;
		uint64_t tx_packets;
		uint16_t vid;
		uint16_t flags;
		uint32_t pad2;
	} st;

	if (len < sizeof(struct strace_bridge_vlan_xstats))
		return false;

	if (umove_or_printaddr(tcp, addr, &st))
		return true;

	PRINT_FIELD_U("{", st, rx_bytes);
	PRINT_FIELD_U(", ", st, rx_packets);
	PRINT_FIELD_U(", ", st, tx_bytes);
	PRINT_FIELD_U(", ", st, tx_packets);
	PRINT_FIELD_U(", ", st, vid);
	PRINT_FIELD_FLAGS(", ", st, flags, nl_bridge_vlan_flags,
			  "BRIDGE_VLAN_INFO_???");
	tprints("}");

	return true;
}

static const nla_decoder_t ifstats_xstats_decoders[] = {
	[LINK_XSTATS_TYPE_UNSPEC]	= NULL,
	[LINK_XSTATS_TYPE_BRIDGE]	= decode_nla_xstats_bridge,
};

static bool
decode_nla_link_xstats(struct tcb *const tcp,
		       const kernel_ulong_t addr,
		       const unsigned int len,
		       const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, ifstats_xstats_type_attr,
		      "LINK_XSTATS_TYPE_???",
		      ARRSZ_PAIR(ifstats_xstats_decoders),
		      opaque_data);

	return true;
}

static const nla_decoder_t ifstats_offload_xstats_decoders[] = {
	[LINK_XSTATS_TYPE_UNSPEC]	= NULL,
	[LINK_XSTATS_TYPE_BRIDGE]	= decode_nla_rtnl_link_stats64,
};

static bool
decode_nla_link_offload_xstats(struct tcb *const tcp,
			       const kernel_ulong_t addr,
			       const unsigned int len,
			       const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, ifstats_offload_attr,
		      "IFLA_OFFLOAD_XSTATS_???",
		      ARRSZ_PAIR(ifstats_offload_xstats_decoders),
		      opaque_data);

	return true;
}

static const nla_decoder_t ifstatsmsg_nla_decoders[] = {
	[IFLA_STATS_UNSPEC]			= NULL,
	[IFLA_STATS_LINK_64]			= decode_nla_rtnl_link_stats64,
	[IFLA_STATS_LINK_XSTATS]		= decode_nla_link_xstats,
	[IFLA_STATS_LINK_XSTATS_SLAVE]		= decode_nla_link_xstats,
	[IFLA_STATS_LINK_OFFLOAD_XSTATS]	= decode_nla_link_offload_xstats,
	[IFLA_STATS_AF_SPEC]			= decode_nla_ifla_af_spec,
};

DECL_NETLINK_ROUTE_DECODER(decode_ifstatsmsg)
{
	struct if_stats_msg ifstats = { .family = family };
	size_t offset = sizeof(ifstats.family);
	bool decode_nla = false;

	PRINT_FIELD_XVAL("{", ifstats, family, addrfams, "AF_???");

	tprints(", ");
	if (len >= sizeof(ifstats)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(ifstats) - offset,
					 (char *) &ifstats + offset)) {
			if (ifstats.pad1)
				PRINT_FIELD_X("", ifstats, pad1);
			if (ifstats.pad2)
				PRINT_FIELD_X("", ifstats, pad2);
			PRINT_FIELD_IFINDEX(", ", ifstats, ifindex);
			PRINT_FIELD_FLAGS(", ", ifstats, filter_mask,
					  ifstats_attr_mask ,
					  "IFLA_STATS_???");
			decode_nla = true;
		}
	} else
		tprints("...");
	tprints("}");

	offset = NLMSG_ALIGN(sizeof(ifstats));
	if (decode_nla && len > offset) {
		tprints(", ");
		decode_nlattr(tcp, addr + offset, len - offset,
			      ifstats_attr, "IFLA_STATS_???",
			      ARRSZ_PAIR(ifstatsmsg_nla_decoders), NULL);
	}
}
