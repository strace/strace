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

#include "netlink_route.h"
#include "nlattr.h"
#include "print_fields.h"

#include "netlink.h"
#include <linux/rtnetlink.h>
#ifdef HAVE_LINUX_FIB_RULES_H
# include <linux/fib_rules.h>
#endif

#include "xlat/fib_rule_actions.h"
#include "xlat/fib_rule_flags.h"
#include "xlat/rtnl_rule_attrs.h"

static bool
decode_rule_addr(struct tcb *const tcp,
		 const kernel_ulong_t addr,
		 const unsigned int len,
		 const void *const opaque_data)
{
	const struct rtmsg *const rtmsg = opaque_data;

	decode_inet_addr(tcp, addr, len, rtmsg->rtm_family, NULL);

	return true;
}

static bool
decode_fib_rule_uid_range(struct tcb *const tcp,
			  const kernel_ulong_t addr,
			  const unsigned int len,
			  const void *const opaque_data)
{
#ifdef HAVE_STRUCT_FIB_RULE_UID_RANGE
	struct fib_rule_uid_range range;

	if (len < sizeof(range))
		return false;
	else if (!umove_or_printaddr(tcp, addr, &range)) {
		PRINT_FIELD_U("{", range, start);
		PRINT_FIELD_U(", ", range, end);
		tprints("}");
	}

	return true;
#else
	return false;
#endif
}

static bool
decode_rule_port_range(struct tcb *const tcp,
		       const kernel_ulong_t addr,
		       const unsigned int len,
		       const void *const opaque_data)
{
	struct /* fib_rule_port_range */ {
		uint16_t start;
		uint16_t end;
	} range;

	if (len < sizeof(range))
		return false;
	else if (!umove_or_printaddr(tcp, addr, &range)) {
		PRINT_FIELD_U("{", range, start);
		PRINT_FIELD_U(", ", range, end);
		tprints("}");
	}

	return true;
}

static const nla_decoder_t fib_rule_hdr_nla_decoders[] = {
	[FRA_DST]			= decode_rule_addr,
	[FRA_SRC]			= decode_rule_addr,
	[FRA_IIFNAME]			= decode_nla_str,
	[FRA_GOTO]			= decode_nla_u32,
	[FRA_PRIORITY]			= decode_nla_u32,
	[FRA_FWMARK]			= decode_nla_u32,
	[FRA_FLOW]			= decode_nla_u32,
	[FRA_TUN_ID]			= decode_nla_be64,
	[FRA_SUPPRESS_IFGROUP]		= decode_nla_u32,
	[FRA_SUPPRESS_PREFIXLEN]	= decode_nla_u32,
	[FRA_TABLE]			= decode_nla_rt_class,
	[FRA_FWMASK]			= decode_nla_u32,
	[FRA_OIFNAME]			= decode_nla_str,
	[FRA_PAD]			= NULL,
	[FRA_L3MDEV]			= decode_nla_u8,
	[FRA_UID_RANGE]			= decode_fib_rule_uid_range,
	[FRA_PROTOCOL]			= decode_nla_rt_proto,
	[FRA_IP_PROTO]			= decode_nla_ip_proto,
	[FRA_SPORT_RANGE]		= decode_rule_port_range,
	[FRA_DPORT_RANGE]		= decode_rule_port_range,
};

DECL_NETLINK_ROUTE_DECODER(decode_fib_rule_hdr)
{
	/*
	 * struct rtmsg and struct fib_rule_hdr are essentially
	 * the same structure, use struct rtmsg but treat it as
	 * struct fib_rule_hdr.
	 */
	struct rtmsg msg = { .rtm_family = family };
	size_t offset = sizeof(msg.rtm_family);
	bool decode_nla = false;

	tprints("{family=");
	printxval(addrfams, msg.rtm_family, "AF_???");

	tprints(", ");
	if (len >= sizeof(msg)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(msg) - offset,
					 (char *) &msg + offset)) {
			tprintf("dst_len=%u, src_len=%u",
				msg.rtm_dst_len, msg.rtm_src_len);
			tprints(", tos=");
			printflags(ip_type_of_services, msg.rtm_tos,
				   "IPTOS_TOS_???");
			tprints(", table=");
			printxval(routing_table_ids, msg.rtm_table, NULL);
			tprints(", action=");
			printxval(fib_rule_actions, msg.rtm_type, "FR_ACT_???");
			tprints(", flags=");
			printflags(fib_rule_flags, msg.rtm_flags,
				   "FIB_RULE_???");
			decode_nla = true;
		}
	} else
		tprints("...");
	tprints("}");

	offset = NLMSG_ALIGN(sizeof(msg));
	if (decode_nla && len > offset) {
		tprints(", ");
		decode_nlattr(tcp, addr + offset, len - offset,
			      rtnl_rule_attrs, "FRA_???",
			      fib_rule_hdr_nla_decoders,
			      ARRAY_SIZE(fib_rule_hdr_nla_decoders), &msg);
	}
}
