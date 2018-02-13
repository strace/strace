/*
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2016-2018 The strace developers.
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

#ifdef HAVE_STRUCT_BR_PORT_MSG

# include "netlink_route.h"
# include "nlattr.h"
# include "print_fields.h"

# include <netinet/in.h>
# include <linux/if_bridge.h>
# include "netlink.h"

# ifdef HAVE_STRUCT_BR_MDB_ENTRY_FLAGS
#  include "xlat/mdb_flags.h"
# endif
# include "xlat/mdb_states.h"
# include "xlat/multicast_router_types.h"
# include "xlat/rtnl_mdb_attrs.h"
# include "xlat/rtnl_mdba_mdb_attrs.h"
# include "xlat/rtnl_mdba_mdb_eattr_attrs.h"
# include "xlat/rtnl_mdba_mdb_entry_attrs.h"
# include "xlat/rtnl_mdba_router_attrs.h"
# include "xlat/rtnl_mdba_router_pattr_attrs.h"

static const nla_decoder_t mdba_mdb_eattr_nla_decoders[] = {
	[MDBA_MDB_EATTR_TIMER]	= decode_nla_u32
};

static bool
decode_mdba_mdb_entry_info(struct tcb *const tcp,
			   const kernel_ulong_t addr,
			   const unsigned int len,
			   const void *const opaque_data)
{
# ifdef HAVE_STRUCT_BR_MDB_ENTRY
	struct br_mdb_entry entry;

	if (len < sizeof(entry))
		return false;
	else if (!umove_or_printaddr(tcp, addr, &entry)) {
		PRINT_FIELD_IFINDEX("{", entry, ifindex);
		PRINT_FIELD_XVAL(", ", entry, state, mdb_states, "MDB_???");
#  ifdef HAVE_STRUCT_BR_MDB_ENTRY_FLAGS
		PRINT_FIELD_FLAGS(", ", entry, flags,
				  mdb_flags, "MDB_FLAGS_???");
#  endif
#  ifdef HAVE_STRUCT_BR_MDB_ENTRY_VID
		PRINT_FIELD_U(", ", entry, vid);
#  endif

		const int proto = ntohs(entry.addr.proto);

		tprints(", addr={");
		print_inet_addr(proto, &entry.addr.u,
				sizeof(entry.addr.u), "u");
		tprints(", proto=htons(");
		printxval(addrfams, proto, "AF_???");
		tprints(")}}");
	}

	const size_t offset = NLMSG_ALIGN(sizeof(entry));
	if (len > offset) {
		tprints(", ");
		decode_nlattr(tcp, addr + offset, len - offset,
			      rtnl_mdba_mdb_eattr_attrs, "MDBA_MDB_EATTR_???",
			      mdba_mdb_eattr_nla_decoders,
			      ARRAY_SIZE(mdba_mdb_eattr_nla_decoders), NULL);
	}

	return true;
# else
	return false;
# endif /* HAVE_STRUCT_BR_MDB_ENTRY */
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
	[MDBA_ROUTER_PATTR_TIMER]	= decode_nla_u32,
	[MDBA_ROUTER_PATTR_TYPE]	= decode_multicast_router_type
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
		tprints(", ");
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

	PRINT_FIELD_XVAL("{", bpm, family, addrfams, "AF_???");

	tprints(", ");
	if (len >= sizeof(bpm)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(bpm) - offset,
					 (char *) &bpm + offset)) {
			PRINT_FIELD_IFINDEX("", bpm, ifindex);
			decode_nla = true;
		}
	} else
		tprints("...");
	tprints("}");

	offset = NLMSG_ALIGN(sizeof(bpm));
	if (decode_nla && len > offset) {
		tprints(", ");
		decode_nlattr(tcp, addr + offset, len - offset,
			      rtnl_mdb_attrs, "MDBA_???",
			      br_port_msg_nla_decoders,
			      ARRAY_SIZE(br_port_msg_nla_decoders), NULL);
	}
}

#endif
