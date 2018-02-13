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
