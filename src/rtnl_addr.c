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
#include <linux/if_addr.h>

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
		tprint_struct_begin();
		PRINT_FIELD_U(ci, ifa_prefered);
		tprint_struct_next();
		PRINT_FIELD_U(ci, ifa_valid);
		tprint_struct_next();
		PRINT_FIELD_U(ci, cstamp);
		tprint_struct_next();
		PRINT_FIELD_U(ci, tstamp);
		tprint_struct_end();
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
	[IFA_ADDRESS]		= decode_ifa_address,
	[IFA_LOCAL]		= decode_ifa_address,
	[IFA_LABEL]		= decode_nla_str,
	[IFA_BROADCAST]		= decode_ifa_address,
	[IFA_ANYCAST]		= decode_ifa_address,
	[IFA_CACHEINFO]		= decode_ifa_cacheinfo,
	[IFA_MULTICAST]		= decode_ifa_address,
	[IFA_FLAGS]		= decode_ifa_flags,
	[IFA_RT_PRIORITY]	= decode_nla_u32,
	[IFA_TARGET_NETNSID]	= decode_nla_s32,
};

DECL_NETLINK_ROUTE_DECODER(decode_ifaddrmsg)
{
	struct ifaddrmsg ifaddr = { .ifa_family = family };
	size_t offset = sizeof(ifaddr.ifa_family);
	bool decode_nla = false;

	tprint_struct_begin();
	PRINT_FIELD_XVAL(ifaddr, ifa_family, addrfams, "AF_???");
	tprint_struct_next();

	if (len >= sizeof(ifaddr)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(ifaddr) - offset,
					 (char *) &ifaddr + offset)) {
			PRINT_FIELD_U(ifaddr, ifa_prefixlen);
			tprint_struct_next();
			PRINT_FIELD_FLAGS(ifaddr, ifa_flags,
					  ifaddrflags, "IFA_F_???");
			tprint_struct_next();
			PRINT_FIELD_XVAL(ifaddr, ifa_scope,
					 routing_scopes, NULL);
			tprint_struct_next();
			PRINT_FIELD_IFINDEX(ifaddr, ifa_index);
			decode_nla = true;
		}
	} else
		tprint_more_data_follows();
	tprint_struct_end();

	offset = NLMSG_ALIGN(sizeof(ifaddr));
	if (decode_nla && len > offset) {
		tprint_array_next();
		decode_nlattr(tcp, addr + offset, len - offset,
			      rtnl_addr_attrs, "IFA_???",
			      ifaddrmsg_nla_decoders,
			      ARRAY_SIZE(ifaddrmsg_nla_decoders), &ifaddr);
	}
}
