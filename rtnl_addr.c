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
#ifdef HAVE_LINUX_IF_ADDR_H
# include <linux/if_addr.h>
#endif

#include "xlat/ifaddrflags.h"
#include "xlat/routing_scopes.h"
#include "xlat/rtnl_addr_attrs.h"

static bool
decode_ifa_address(struct tcb *const tcp,
		   const kernel_ulong_t addr,
		   const unsigned int len,
		   const void *const opaque_data)
{
	const struct ifaddrmsg *const ifaddr = opaque_data;

	decode_inet_addr(tcp, addr, len, ifaddr->ifa_family, NULL);

	return true;
}

static bool
decode_ifa_cacheinfo(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const unsigned int len,
		     const void *const opaque_data)
{
	struct ifa_cacheinfo ci;

	if (len < sizeof(ci))
		return false;
	else if (!umove_or_printaddr(tcp, addr, &ci)) {
		PRINT_FIELD_U("{", ci, ifa_prefered);
		PRINT_FIELD_U(", ", ci, ifa_valid);
		PRINT_FIELD_U(", ", ci, cstamp);
		PRINT_FIELD_U(", ", ci, tstamp);
		tprintf("}");
	}

	return true;
}

static bool
decode_ifa_flags(struct tcb *const tcp,
		 const kernel_ulong_t addr,
		 const unsigned int len,
		 const void *const opaque_data)
{
	uint32_t ifa_flags;

	if (len < sizeof(ifa_flags))
		return false;
	else if (!umove_or_printaddr(tcp, addr, &ifa_flags))
		printflags(ifaddrflags, ifa_flags, "IFA_F_???");

	return true;
}

static const nla_decoder_t ifaddrmsg_nla_decoders[] = {
	[IFA_ADDRESS]	= decode_ifa_address,
	[IFA_LOCAL]	= decode_ifa_address,
	[IFA_LABEL]	= decode_nla_str,
	[IFA_BROADCAST]	= decode_ifa_address,
	[IFA_ANYCAST]	= decode_ifa_address,
	[IFA_CACHEINFO]	= decode_ifa_cacheinfo,
	[IFA_MULTICAST]	= decode_ifa_address,
	[IFA_FLAGS]	= decode_ifa_flags
};

DECL_NETLINK_ROUTE_DECODER(decode_ifaddrmsg)
{
	struct ifaddrmsg ifaddr = { .ifa_family = family };
	size_t offset = sizeof(ifaddr.ifa_family);
	bool decode_nla = false;

	PRINT_FIELD_XVAL("{", ifaddr, ifa_family, addrfams, "AF_???");

	tprints(", ");
	if (len >= sizeof(ifaddr)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(ifaddr) - offset,
					 (char *) &ifaddr + offset)) {
			PRINT_FIELD_U("", ifaddr, ifa_prefixlen);
			PRINT_FIELD_FLAGS(", ", ifaddr, ifa_flags,
					  ifaddrflags, "IFA_F_???");
			PRINT_FIELD_XVAL(", ", ifaddr, ifa_scope,
					 routing_scopes, NULL);
			PRINT_FIELD_IFINDEX(", ", ifaddr, ifa_index);
			decode_nla = true;
		}
	} else
		tprints("...");
	tprints("}");

	offset = NLMSG_ALIGN(sizeof(ifaddr));
	if (decode_nla && len > offset) {
		tprints(", ");
		decode_nlattr(tcp, addr + offset, len - offset,
			      rtnl_addr_attrs, "IFA_???",
			      ifaddrmsg_nla_decoders,
			      ARRAY_SIZE(ifaddrmsg_nla_decoders), &ifaddr);
	}
}
