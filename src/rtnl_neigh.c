/*
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2016-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "netlink_route.h"
#include "nlattr.h"

#include "netlink.h"
#include <linux/neighbour.h>

#include "xlat/fdb_notify_flags.h"
#include "xlat/neighbor_cache_entry_ext_flags.h"
#include "xlat/neighbor_cache_entry_flags.h"
#include "xlat/neighbor_cache_entry_states.h"
#include "xlat/rtnl_neigh_attrs.h"
#include "xlat/rtnl_neigh_fdb_ext_attrs.h"

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
		tprint_struct_begin();
		PRINT_FIELD_U(ci, ndm_confirmed);
		tprint_struct_next();
		PRINT_FIELD_U(ci, ndm_used);
		tprint_struct_next();
		PRINT_FIELD_U(ci, ndm_updated);
		tprint_struct_next();
		PRINT_FIELD_U(ci, ndm_refcnt);
		tprint_struct_end();
	}

	return true;
}

static bool
decode_fdb_notify_flags(struct tcb *const tcp,
		        const kernel_ulong_t addr,
		        const unsigned int len,
		        const void *const opaque_data)
{
	static const struct decode_nla_xlat_opts opts = {
		fdb_notify_flags, "FDB_NOTIFY_???",
		.size = 1,
	};

	return decode_nla_flags(tcp, addr, len, &opts);
}

static const nla_decoder_t nda_fdb_ext_attrs_nla_decoders[] = {
	[NFEA_UNSPEC]		= NULL,
	[NFEA_ACTIVITY_NOTIFY]	= decode_fdb_notify_flags,
	[NFEA_DONT_REFRESH]	= NULL, /* flag attr, no payload is expected */
};

static bool
decode_nda_fdb_ext_attrs(struct tcb *const tcp,
			 const kernel_ulong_t addr,
			 const unsigned int len,
			 const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, rtnl_neigh_fdb_ext_attrs,
		      "NFEA_???", ARRSZ_PAIR(nda_fdb_ext_attrs_nla_decoders),
		      opaque_data);

	return true;
}

static bool
decode_nda_ext_flags(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const unsigned int len,
		     const void *const opaque_data)
{
	static const struct decode_nla_xlat_opts opts = {
		neighbor_cache_entry_ext_flags, "NTF_EXT_???",
		.size = 4,
	};

	return decode_nla_flags(tcp, addr, len, &opts);
}

static bool
decode_nda_ndm_states(struct tcb *const tcp,
		      const kernel_ulong_t addr,
		      const unsigned int len,
		      const void *const opaque_data)
{
	static const struct decode_nla_xlat_opts opts = {
		neighbor_cache_entry_states, "NUD_???",
		.size = 2,
	};

	return decode_nla_flags(tcp, addr, len, &opts);
}

static bool
decode_nda_ndm_flags(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const unsigned int len,
		     const void *const opaque_data)
{
	static const struct decode_nla_xlat_opts opts = {
		neighbor_cache_entry_flags, "NTF_???",
		.size = 1,
	};

	return decode_nla_flags(tcp, addr, len, &opts);
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
	[NDA_SRC_VNI]		= decode_nla_u32,
	[NDA_PROTOCOL]		= decode_nla_ip_proto,
	[NDA_NH_ID]		= decode_nla_u32,
	[NDA_FDB_EXT_ATTRS]	= decode_nda_fdb_ext_attrs,
	[NDA_FLAGS_EXT]		= decode_nda_ext_flags,
	[NDA_NDM_STATE_MASK]	= decode_nda_ndm_states,
	[NDA_NDM_FLAGS_MASK]	= decode_nda_ndm_flags,
};

DECL_NETLINK_ROUTE_DECODER(decode_ndmsg)
{
	struct ndmsg ndmsg = { .ndm_family = family };
	size_t offset = sizeof(ndmsg.ndm_family);
	bool decode_nla = false;

	tprint_struct_begin();
	PRINT_FIELD_XVAL(ndmsg, ndm_family, addrfams, "AF_???");
	tprint_struct_next();

	if (len >= sizeof(ndmsg)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(ndmsg) - offset,
					 (char *) &ndmsg + offset)) {
			PRINT_FIELD_IFINDEX(ndmsg, ndm_ifindex);
			tprint_struct_next();
			PRINT_FIELD_FLAGS(ndmsg, ndm_state,
					  neighbor_cache_entry_states,
					  "NUD_???");
			tprint_struct_next();
			PRINT_FIELD_FLAGS(ndmsg, ndm_flags,
					  neighbor_cache_entry_flags,
					  "NTF_???");
			tprint_struct_next();
			PRINT_FIELD_XVAL(ndmsg, ndm_type,
					 routing_types, "RTN_???");
			decode_nla = true;
		}
	} else
		tprint_more_data_follows();
	tprint_struct_end();

	offset = NLMSG_ALIGN(sizeof(ndmsg));
	if (decode_nla && len > offset) {
		tprint_array_next();
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
