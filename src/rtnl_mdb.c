/*
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include "netlink_route.h"
#include "nlattr.h"
#include "netlink.h"
#include <netinet/in.h>
#include <linux/if_bridge.h>

#include "xlat/mdb_flags.h"
#include "xlat/mdb_states.h"
#include "xlat/multicast_router_types.h"
#include "xlat/rtnl_mdb_attrs.h"
#include "xlat/rtnl_mdba_mdb_attrs.h"
#include "xlat/rtnl_mdba_mdb_eattr_attrs.h"
#include "xlat/rtnl_mdba_mdb_entry_attrs.h"
#include "xlat/rtnl_mdba_router_attrs.h"
#include "xlat/rtnl_mdba_router_pattr_attrs.h"

static const nla_decoder_t mdba_mdb_eattr_nla_decoders[] = {
	[MDBA_MDB_EATTR_TIMER]	= decode_nla_clock_t,
};

static bool
decode_mdba_mdb_entry_info(struct tcb *const tcp,
			   const kernel_ulong_t addr,
			   const unsigned int len,
			   const void *const opaque_data)
{
	struct br_mdb_entry entry;

	if (len < sizeof(entry))
		return false;
	else if (!umove_or_printaddr(tcp, addr, &entry)) {
		tprint_struct_begin();
		PRINT_FIELD_IFINDEX(entry, ifindex);
		tprint_struct_next();
		PRINT_FIELD_XVAL(entry, state, mdb_states, "MDB_???");

		/*
		 * Note that it's impossible to derive if flags/vid fields
		 * are present on all architectures except m68k; as a side note,
		 * v4.3-rc1~96^2~365 has introduced an ABI breakage on m68k.
		 */
		tprint_struct_next();
		PRINT_FIELD_FLAGS(entry, flags,
				  mdb_flags, "MDB_FLAGS_???");
		tprint_struct_next();
		PRINT_FIELD_U(entry, vid);

		const int proto = ntohs(entry.addr.proto);

		tprint_struct_next();
		tprints_field_name("addr");
		tprint_struct_begin();
		print_inet_addr(proto, &entry.addr.u,
				sizeof(entry.addr.u), "u");
		tprint_struct_next();
		tprints_field_name("proto");
		tprints_arg_begin("htons");
		printxval(addrfams, proto, "AF_???");
		tprint_arg_end();
		tprint_struct_end();
		tprint_struct_end();
	}

	const size_t offset = NLMSG_ALIGN(sizeof(entry));
	if (len > offset) {
		tprint_array_next();
		decode_nlattr(tcp, addr + offset, len - offset,
			      rtnl_mdba_mdb_eattr_attrs, "MDBA_MDB_EATTR_???",
			      mdba_mdb_eattr_nla_decoders,
			      ARRAY_SIZE(mdba_mdb_eattr_nla_decoders), NULL);
	}

	return true;
}

static const nla_decoder_t mdba_mdb_entry_nla_decoders[] = {
	[MDBA_MDB_ENTRY_INFO]	= decode_mdba_mdb_entry_info
};

static bool
decode_mdba_mdb_entry(struct tcb *const tcp,
		      const kernel_ulong_t addr,
		      const unsigned int len,
		      const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, rtnl_mdba_mdb_entry_attrs,
		      "MDBA_MDB_ENTRY_???", mdba_mdb_entry_nla_decoders,
		      ARRAY_SIZE(mdba_mdb_entry_nla_decoders), NULL);

	return true;
}

static const nla_decoder_t mdba_mdb_nla_decoders[] = {
	[MDBA_MDB_ENTRY]	= decode_mdba_mdb_entry
};

static bool
decode_mdba_mdb(struct tcb *const tcp,
		const kernel_ulong_t addr,
		const unsigned int len,
		const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, rtnl_mdba_mdb_attrs, "MDBA_MDB_???",
		      mdba_mdb_nla_decoders,
		      ARRAY_SIZE(mdba_mdb_nla_decoders), NULL);

	return true;
}

static bool
decode_multicast_router_type(struct tcb *const tcp,
			     const kernel_ulong_t addr,
			     const unsigned int len,
			     const void *const opaque_data)
{
	uint8_t type;

	if (!umove_or_printaddr(tcp, addr, &type))
		printxval(multicast_router_types, type, "MDB_RTR_TYPE_???");

	return true;
}

static const nla_decoder_t mdba_router_pattr_nla_decoders[] = {
	[MDBA_ROUTER_PATTR_TIMER]	= decode_nla_clock_t,
	[MDBA_ROUTER_PATTR_TYPE]	= decode_multicast_router_type,
	[MDBA_ROUTER_PATTR_INET_TIMER]	= decode_nla_clock_t,
	[MDBA_ROUTER_PATTR_INET6_TIMER]	= decode_nla_clock_t,
};

static bool
decode_mdba_router_port(struct tcb *const tcp,
			const kernel_ulong_t addr,
			const unsigned int len,
			const void *const opaque_data)
{
	uint32_t ifindex;

	if (len < sizeof(ifindex))
		return false;
	else if (!umove_or_printaddr(tcp, addr, &ifindex))
		print_ifindex(ifindex);

	const size_t offset = NLMSG_ALIGN(sizeof(ifindex));
	if (len > offset) {
		tprint_array_next();
		decode_nlattr(tcp, addr + offset, len - offset,
			      rtnl_mdba_router_pattr_attrs,
			      "MDBA_ROUTER_PATTR_???",
			      mdba_router_pattr_nla_decoders,
			      ARRAY_SIZE(mdba_router_pattr_nla_decoders), NULL);
	}

	return true;
}

static const nla_decoder_t mdba_router_nla_decoders[] = {
	[MDBA_ROUTER_PORT]	= decode_mdba_router_port
};

static bool
decode_mdba_router(struct tcb *const tcp,
		   const kernel_ulong_t addr,
		   const unsigned int len,
		   const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, rtnl_mdba_router_attrs, "MDBA_ROUTER_???",
		      mdba_router_nla_decoders,
		      ARRAY_SIZE(mdba_router_nla_decoders), NULL);

	return true;
}

static const nla_decoder_t br_port_msg_nla_decoders[] = {
	[MDBA_MDB]	= decode_mdba_mdb,
	[MDBA_ROUTER]	= decode_mdba_router
};

DECL_NETLINK_ROUTE_DECODER(decode_br_port_msg)
{
	struct br_port_msg bpm = { .family = family };
	size_t offset = sizeof(bpm.family);
	bool decode_nla = false;

	tprint_struct_begin();
	PRINT_FIELD_XVAL(bpm, family, addrfams, "AF_???");
	tprint_struct_next();

	if (len >= sizeof(bpm)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(bpm) - offset,
					 (char *) &bpm + offset)) {
			PRINT_FIELD_IFINDEX(bpm, ifindex);
			decode_nla = true;
		}
	} else
		tprint_more_data_follows();
	tprint_struct_end();

	offset = NLMSG_ALIGN(sizeof(bpm));
	if (decode_nla && len > offset) {
		tprint_array_next();
		decode_nlattr(tcp, addr + offset, len - offset,
			      rtnl_mdb_attrs, "MDBA_???",
			      br_port_msg_nla_decoders,
			      ARRAY_SIZE(br_port_msg_nla_decoders), NULL);
	}
}
