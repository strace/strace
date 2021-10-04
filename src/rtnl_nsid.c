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
#include <linux/rtnetlink.h>

#include "xlat/rtnl_nsid_attrs.h"

static const nla_decoder_t nsid_nla_decoders[] = {
	[NETNSA_NSID]		= decode_nla_s32,
	[NETNSA_PID]		= decode_nla_u32,
	[NETNSA_FD]		= decode_nla_fd,
	[NETNSA_TARGET_NSID]	= decode_nla_s32,
	[NETNSA_CURRENT_NSID]	= decode_nla_s32
};

DECL_NETLINK_ROUTE_DECODER(decode_nsid)
{
	struct rtgenmsg rtgenmsg = { .rtgen_family = family };

	tprint_struct_begin();
	PRINT_FIELD_XVAL(rtgenmsg, rtgen_family, addrfams, "AF_???");
	tprint_struct_end();

	const size_t offset = NLMSG_ALIGN(sizeof(rtgenmsg));
	if (len > offset) {
		tprint_array_next();
		decode_nlattr(tcp, addr + offset, len - offset,
			      rtnl_nsid_attrs, "NETNSA_???",
			      nsid_nla_decoders,
			      ARRAY_SIZE(nsid_nla_decoders), NULL);
	}
}
