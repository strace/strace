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

#include <linux/ip.h>
#include "netlink.h"
#include <linux/rtnetlink.h>

#include "xlat/ip_type_of_services.h"
#include "xlat/routing_flags.h"
#include "xlat/routing_protocols.h"
#include "xlat/routing_table_ids.h"
#include "xlat/routing_types.h"
#include "xlat/rtnl_route_attrs.h"

DECL_NETLINK_ROUTE_DECODER(decode_rtmsg)
{
	struct rtmsg rtmsg = { .rtm_family = family };
	size_t offset = sizeof(rtmsg.rtm_family);
	bool decode_nla = false;

	PRINT_FIELD_XVAL("{", rtmsg, rtm_family, addrfams, "AF_???");

	tprints(", ");
	if (len >= sizeof(rtmsg)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(rtmsg) - offset,
					 (void *) &rtmsg + offset)) {
			PRINT_FIELD_U("", rtmsg, rtm_dst_len);
			PRINT_FIELD_U(", ", rtmsg, rtm_src_len);
			PRINT_FIELD_FLAGS(", ", rtmsg, rtm_tos,
					  ip_type_of_services, "IPTOS_TOS_???");
			PRINT_FIELD_XVAL(", ", rtmsg, rtm_table,
					 routing_table_ids, NULL);
			PRINT_FIELD_XVAL(", ", rtmsg, rtm_protocol,
					 routing_protocols, "RTPROT_???");
			PRINT_FIELD_XVAL(", ", rtmsg, rtm_scope,
					 routing_scopes, NULL);
			PRINT_FIELD_XVAL(", ", rtmsg, rtm_type,
					 routing_types, "RTN_???");
			PRINT_FIELD_FLAGS(", ", rtmsg, rtm_flags,
					  routing_flags, "RTM_F_???");
			decode_nla = true;
		}
	} else
		tprints("...");
	tprints("}");

	offset = NLMSG_ALIGN(sizeof(rtmsg));
	if (decode_nla && len > offset) {
		tprints(", ");
		decode_nlattr(tcp, addr + offset, len - offset,
			      rtnl_route_attrs, "RTA_???", NULL, 0, NULL);
	}
}
