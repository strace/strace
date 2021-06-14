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

#include <linux/netconf.h>
#include "netlink.h"

#include "xlat/rtnl_netconf_attrs.h"

static const nla_decoder_t netconfmsg_nla_decoders[] = {
	[NETCONFA_IFINDEX]			= decode_nla_ifindex,
	[NETCONFA_FORWARDING]			= decode_nla_s32,
	[NETCONFA_RP_FILTER]			= decode_nla_s32,
	[NETCONFA_MC_FORWARDING]		= decode_nla_s32,
	[NETCONFA_PROXY_NEIGH]			= decode_nla_s32,
	[NETCONFA_IGNORE_ROUTES_WITH_LINKDOWN]	= decode_nla_s32,
	[NETCONFA_INPUT]			= decode_nla_s32,
	[NETCONFA_BC_FORWARDING]		= decode_nla_s32,
};

DECL_NETLINK_ROUTE_DECODER(decode_netconfmsg)
{
	struct netconfmsg ncm = { .ncm_family = family };

	tprint_struct_begin();
	PRINT_FIELD_XVAL(ncm, ncm_family, addrfams, "AF_???");
	tprint_struct_end();

	const size_t offset = NLMSG_ALIGN(sizeof(ncm));
	if (len > offset) {
		tprint_array_next();
		decode_nlattr(tcp, addr + offset, len - offset,
			      rtnl_netconf_attrs, "NETCONFA_???",
			      netconfmsg_nla_decoders,
			      ARRAY_SIZE(netconfmsg_nla_decoders), NULL);
	}
}
