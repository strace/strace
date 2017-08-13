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
#include "print_fields.h"

#include "netlink.h"
#include <linux/rtnetlink.h>
#ifdef HAVE_LINUX_FIB_RULES_H
# include <linux/fib_rules.h>
#endif

#include "xlat/fib_rule_actions.h"
#include "xlat/fib_rule_flags.h"

DECL_NETLINK_ROUTE_DECODER(decode_fib_rule_hdr)
{
	/*
	 * struct rtmsg and struct fib_rule_hdr are essentially
	 * the same structure, use struct rtmsg but treat it as
	 * struct fib_rule_hdr.
	 */
	struct rtmsg msg = { .rtm_family = family };
	const size_t offset = sizeof(msg.rtm_family);

	tprints("{family=");
	printxval(addrfams, msg.rtm_family, "AF_???");

	tprints(", ");
	if (len >= sizeof(msg)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(msg) - offset,
					 (void *) &msg + offset)) {
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
		}
	} else
		tprints("...");
	tprints("}");
}
