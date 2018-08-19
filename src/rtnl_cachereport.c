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
#include <linux/mroute.h>
#include <linux/mroute6.h>
#include <linux/rtnetlink.h>

#include "xlat/rtnl_ip6mra_creport_attr.h"
#include "xlat/rtnl_ip6mra_msg_types.h"
#include "xlat/rtnl_ipmra_creport_attr.h"
#include "xlat/rtnl_ipmra_msg_types.h"
#include "xlat/rtnl_family.h"

static bool
decode_nla_ipmra_msg_type(struct tcb *const tcp,
			  const kernel_ulong_t addr,
			  const unsigned int len,
			  const void *const opaque_data)
{
	const struct decode_nla_xlat_opts opts = {
		.xlat = rtnl_ipmra_msg_types,
		.dflt = "IGMPMSG_???",
		.size = 1,
	};

	return decode_nla_xval(tcp, addr, len, &opts);
}

static const nla_decoder_t rtnl_creport_ipmra_decoders[] = {
	[IPMRA_CREPORT_UNSPEC]		= NULL,
	[IPMRA_CREPORT_MSGTYPE]		= decode_nla_ipmra_msg_type,
	[IPMRA_CREPORT_VIF_ID]		= decode_nla_u32,
	[IPMRA_CREPORT_SRC_ADDR]	= decode_nla_in_addr,
	[IPMRA_CREPORT_DST_ADDR]	= decode_nla_in_addr,
	[IPMRA_CREPORT_PKT]		= NULL, /* raw packet data */
	[IPMRA_CREPORT_TABLE]		= decode_nla_u32,
};

static bool
decode_nla_ip6mra_msg_type(struct tcb *const tcp,
			   const kernel_ulong_t addr,
			   const unsigned int len,
			   const void *const opaque_data)
{
	const struct decode_nla_xlat_opts opts = {
		.xlat = rtnl_ip6mra_msg_types,
		.dflt = "MRT6MSG_???",
		.size = 1,
	};

	return decode_nla_xval(tcp, addr, len, &opts);
}

static const nla_decoder_t rtnl_creport_ip6mra_decoders[] = {
	[IP6MRA_CREPORT_UNSPEC]		= NULL,
	[IP6MRA_CREPORT_MSGTYPE]	= decode_nla_ip6mra_msg_type,
	[IP6MRA_CREPORT_MIF_ID]		= decode_nla_u32,
	[IP6MRA_CREPORT_SRC_ADDR]	= decode_nla_in6_addr,
	[IP6MRA_CREPORT_DST_ADDR]	= decode_nla_in6_addr,
	[IP6MRA_CREPORT_PKT]		= NULL, /* raw packet data */
};

DECL_NETLINK_ROUTE_DECODER(decode_cachereport)
{
	static const struct af_spec_decoder_desc cachereport_descs[] = {
		{ RTNL_FAMILY_IPMR,
		  rtnl_ipmra_creport_attr, "IPMRA_CREPORT_???",
		  ARRSZ_PAIR(rtnl_creport_ipmra_decoders) },
		{ RTNL_FAMILY_IP6MR,
		  rtnl_ip6mra_creport_attr, "IP6MRA_CREPORT_???",
		  ARRSZ_PAIR(rtnl_creport_ip6mra_decoders) },
	};

	struct rtgenmsg rtgenmsg = { .rtgen_family = family };

	tprint_struct_begin();
	tprints_field_name("rtgen_family");
	printxvals_ex(rtgenmsg.rtgen_family, "RTNL_FAMILY_???",
		      XLAT_STYLE_DEFAULT, rtnl_family, addrfams, NULL);
	tprint_struct_end();

	const size_t offset = NLMSG_ALIGN(sizeof(rtgenmsg));

	if (len > offset) {
		tprint_array_next();
		decode_nla_af_spec(tcp, addr + offset, len - offset,
				   rtgenmsg.rtgen_family,
				   ARRSZ_PAIR(cachereport_descs));
	}
}
