/*
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#ifdef HAVE_STRUCT_IFADDRLBLMSG

# include "netlink_route.h"
# include "nlattr.h"
# include "print_fields.h"

# include <linux/if_addrlabel.h>
# include "netlink.h"

# include "xlat/rtnl_addrlabel_attrs.h"

static bool
decode_ifal_address(struct tcb *const tcp,
		    const kernel_ulong_t addr,
		    const unsigned int len,
		    const void *const opaque_data)
{
	const struct ifaddrlblmsg *const ifal = opaque_data;

	decode_inet_addr(tcp, addr, len, ifal->ifal_family, NULL);

	return true;
}

static const nla_decoder_t ifaddrlblmsg_nla_decoders[] = {
	[IFAL_ADDRESS]	= decode_ifal_address,
	[IFAL_LABEL]	= decode_nla_u32
};

DECL_NETLINK_ROUTE_DECODER(decode_ifaddrlblmsg)
{
	struct ifaddrlblmsg ifal = { .ifal_family = family };
	size_t offset = sizeof(ifal.ifal_family);
	bool decode_nla = false;

	PRINT_FIELD_XVAL("{", ifal, ifal_family, addrfams, "AF_???");

	tprints(", ");
	if (len >= sizeof(ifal)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(ifal) - offset,
					 (char *) &ifal + offset)) {
			PRINT_FIELD_U("", ifal, ifal_prefixlen);
			PRINT_FIELD_U(", ", ifal, ifal_flags);
			PRINT_FIELD_IFINDEX(", ", ifal, ifal_index);
			PRINT_FIELD_U(", ", ifal, ifal_seq);
			decode_nla = true;
		}
	} else
		tprints("...");
	tprints("}");

	offset = NLMSG_ALIGN(sizeof(ifal));
	if (decode_nla && len > offset) {
		tprints(", ");
		decode_nlattr(tcp, addr + offset, len - offset,
			      rtnl_addrlabel_attrs, "IFAL_???",
			      ifaddrlblmsg_nla_decoders,
			      ARRAY_SIZE(ifaddrlblmsg_nla_decoders), &ifal);
	}
}

#endif
