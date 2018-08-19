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

#ifdef HAVE_STRUCT_NETCONFMSG

# include "netlink_route.h"
# include "nlattr.h"
# include "print_fields.h"

# include <linux/netconf.h>
# include "netlink.h"

# include "xlat/rtnl_netconf_attrs.h"

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

	PRINT_FIELD_XVAL("{", ncm, ncm_family, addrfams, "AF_???");
	tprints("}");

	const size_t offset = NLMSG_ALIGN(sizeof(ncm));
	if (len > offset) {
		tprints(", ");
		decode_nlattr(tcp, addr + offset, len - offset,
			      rtnl_netconf_attrs, "NETCONFA_???",
			      netconfmsg_nla_decoders,
			      ARRAY_SIZE(netconfmsg_nla_decoders), NULL);
	}
}

#endif
