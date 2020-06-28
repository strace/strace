/*
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "netlink_route.h"
#include "nlattr.h"
#include "print_fields.h"

#include "netlink.h"
#include <linux/rtnetlink.h>
#ifdef HAVE_LINUX_NEIGHBOUR_H
# include <linux/neighbour.h>
#endif

#include "xlat/neighbor_cache_entry_flags.h"
#include "xlat/neighbor_cache_entry_states.h"
#include "xlat/rtnl_neigh_attrs.h"

static bool
decode_neigh_addr(struct tcb *const tcp,
		  const kernel_ulong_t addr,
		  const unsigned int len,
		  const void *const opaque_data)
{
	const struct ndmsg *const ndmsg = opaque_data;

	decode_inet_addr(tcp, addr, len, ndmsg->ndm_family, NULL);

	return true;
}

static bool
decode_nda_cacheinfo(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const unsigned int len,
		     const void *const opaque_data)
{
	struct nda_cacheinfo ci;

	if (len < sizeof(ci))
		return false;
	else if (!umove_or_printaddr(tcp, addr, &ci)) {
		PRINT_FIELD_U("{", ci, ndm_confirmed);
		PRINT_FIELD_U(", ", ci, ndm_used);
		PRINT_FIELD_U(", ", ci, ndm_updated);
		PRINT_FIELD_U(", ", ci, ndm_refcnt);
		tprints("}");
	}

	return true;
}

static const nla_decoder_t ndmsg_nla_decoders[] = {
	[NDA_DST]		= decode_neigh_addr,
	[NDA_LLADDR]		= decode_nla_hwaddr_nofamily,
	[NDA_CACHEINFO]		= decode_nda_cacheinfo,
	[NDA_PROBES]		= decode_nla_u32,
	[NDA_VLAN]		= decode_nla_u16,
	[NDA_PORT]		= decode_nla_be16,
	[NDA_VNI]		= decode_nla_u32,
	[NDA_IFINDEX]		= decode_nla_ifindex,
	[NDA_MASTER]		= decode_nla_ifindex,
	[NDA_LINK_NETNSID]	= decode_nla_u32,
	[NDA_SRC_VNI]		= NULL,
	[NDA_PROTOCOL]		= decode_nla_u8,
	[NDA_NH_ID]		= decode_nla_u32,
};

DECL_NETLINK_ROUTE_DECODER(decode_ndmsg)
{
	struct ndmsg ndmsg = { .ndm_family = family };
	size_t offset = sizeof(ndmsg.ndm_family);
	bool decode_nla = false;

	PRINT_FIELD_XVAL("{", ndmsg, ndm_family, addrfams, "AF_???");

	tprints(", ");
	if (len >= sizeof(ndmsg)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(ndmsg) - offset,
					 (char *) &ndmsg + offset)) {
			PRINT_FIELD_IFINDEX("", ndmsg, ndm_ifindex);
			PRINT_FIELD_FLAGS(", ", ndmsg, ndm_state,
					  neighbor_cache_entry_states,
					  "NUD_???");
			PRINT_FIELD_FLAGS(", ", ndmsg, ndm_flags,
					  neighbor_cache_entry_flags,
					  "NTF_???");
			PRINT_FIELD_XVAL(", ", ndmsg, ndm_type,
					 routing_types, "RTN_???");
			decode_nla = true;
		}
	} else
		tprints("...");
	tprints("}");

	offset = NLMSG_ALIGN(sizeof(ndmsg));
	if (decode_nla && len > offset) {
		tprints(", ");
		decode_nlattr(tcp, addr + offset, len - offset,
			      rtnl_neigh_attrs, "NDA_???",
			      ndmsg_nla_decoders,
			      ARRAY_SIZE(ndmsg_nla_decoders), &ndmsg);
	}
}

DECL_NETLINK_ROUTE_DECODER(decode_rtm_getneigh)
{
	if (family == AF_BRIDGE)
		decode_ifinfomsg(tcp, nlmsghdr, family, addr, len);
	else
		decode_ndmsg(tcp, nlmsghdr, family, addr, len);
}
