/*
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2016-2017 The strace developers.
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
		ARRSZ_PAIR(rtnl_ipmra_msg_types), "IGMPMSG_???",
		.xt = XT_INDEXED,
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
};

static bool
decode_nla_ip6mra_msg_type(struct tcb *const tcp,
			   const kernel_ulong_t addr,
			   const unsigned int len,
			   const void *const opaque_data)
{
	const struct decode_nla_xlat_opts opts = {
		ARRSZ_PAIR(rtnl_ip6mra_msg_types), "MRT6MSG_???",
		.xt = XT_INDEXED,
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
	static const struct cachereport_decoder {
		uint8_t family;
		const struct xlat *attrs;
		const char *dflt;
		const nla_decoder_t *decoders;
		size_t decoders_size;
	} cachereport_decoders[] = {
		{ RTNL_FAMILY_IPMR,
		  rtnl_ipmra_creport_attr, "IPMRA_CREPORT_???",
		  ARRSZ_PAIR(rtnl_creport_ipmra_decoders) },
		{ RTNL_FAMILY_IP6MR,
		  rtnl_ip6mra_creport_attr, "IP6MRA_CREPORT_???",
		  ARRSZ_PAIR(rtnl_creport_ip6mra_decoders) },
		{ 0 }
	};

	struct rtgenmsg rtgenmsg = { .rtgen_family = family };

	tprints("{rtgen_family=");
	printxvals_ex(rtgenmsg.rtgen_family, "RTNL_FAMILY_???",
		      XLAT_STYLE_FMT_U, rtnl_family, addrfams, NULL);
	tprints("}");

	const size_t offset = NLMSG_ALIGN(sizeof(rtgenmsg));

	if (len > offset) {
		const struct cachereport_decoder *dec = cachereport_decoders;
		for (; dec->family; dec++)
			if (dec->family == rtgenmsg.rtgen_family)
				break;

		tprints(", ");
		decode_nlattr(tcp, addr + offset, len - offset,
			      dec->attrs, dec->dflt,
			      dec->decoders, dec->decoders_size, NULL);
	}
}
