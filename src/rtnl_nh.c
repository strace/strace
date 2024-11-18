/*
 * Copyright (c) 2021 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2021-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "netlink_route.h"
#include "nlattr.h"
#include "print_fields.h"

#include <linux/rtnetlink.h>
#include <linux/nexthop.h>

#include "xlat/rtnl_nexthop_attrs.h"
#include "xlat/rtnl_nexthop_grp_types.h"
#include "xlat/rtnl_nha_res_bucket_attrs.h"
#include "xlat/rtnl_nha_res_group_attrs.h"

static bool
print_nh_grp(struct tcb *const tcp, void *const elem_buf,
	     const size_t elem_size, void *const opaque_data)
{
	struct nexthop_grp *grp = (struct nexthop_grp *) elem_buf;

	tprint_struct_begin();
	PRINT_FIELD_U(*grp, id);
	tprint_struct_next();
	PRINT_FIELD_U(*grp, weight);
	tprint_struct_next();
	PRINT_FIELD_U(*grp, weight_high);
	if (grp->resvd2) {
		tprint_struct_next();
		PRINT_FIELD_X(*grp, resvd2);
	}
	tprint_struct_end();

	return true;
}

static bool
decode_nha_nh_grp(struct tcb *const tcp,
		  const kernel_ulong_t addr,
		  const unsigned int len,
		  const void *const opaque_data)
{
	struct nexthop_grp elem;
	const size_t nmemb = len / sizeof(elem);

	if (!nmemb)
		return false;

	print_array(tcp, addr, nmemb, &elem, sizeof(elem),
		    tfetch_mem, print_nh_grp, NULL);

	return true;
}

static bool
decode_nha_nh_grp_type(struct tcb *const tcp,
		       const kernel_ulong_t addr,
		       const unsigned int len,
		       const void *const opaque_data)
{
	static const struct decode_nla_xlat_opts opts = {
		.xlat = rtnl_nexthop_grp_types,
		.dflt = "NEXTHOP_GRP_TYPE_???",
		.size = 2,
	};

	return decode_nla_xval(tcp, addr, len, &opts);
}

static bool
decode_nha_addr(struct tcb *const tcp,
		const kernel_ulong_t addr,
		const unsigned int len,
		const void *const opaque_data)
{
	const struct nhmsg *const nhmsg = opaque_data;

	decode_inet_addr(tcp, addr, len, nhmsg->nh_family, NULL);

	return true;
}

static const nla_decoder_t nha_res_group_nla_decoders[] = {
	[NHA_RES_GROUP_PAD]			= NULL,
	[NHA_RES_GROUP_BUCKETS]			= decode_nla_u16,
	[NHA_RES_GROUP_IDLE_TIMER]		= decode_nla_clock_t,
	[NHA_RES_GROUP_UNBALANCED_TIMER]	= decode_nla_clock_t,
	[NHA_RES_GROUP_UNBALANCED_TIME]		= decode_nla_clock_t,
};

static bool
decode_nha_res_group(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const unsigned int len,
		     const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, rtnl_nha_res_group_attrs,
		      "NHA_RES_GROUP_???",
		      ARRSZ_PAIR(nha_res_group_nla_decoders), opaque_data);

	return true;
}

static const nla_decoder_t nha_res_bucket_nla_decoders[] = {
	[NHA_RES_BUCKET_PAD]		= NULL,
	[NHA_RES_BUCKET_INDEX]		= decode_nla_u16,
	[NHA_RES_BUCKET_IDLE_TIME]	= decode_nla_clock_t,
	[NHA_RES_BUCKET_NH_ID]		= decode_nla_u32,
};

static bool
decode_nha_res_bucket(struct tcb *const tcp,
		      const kernel_ulong_t addr,
		      const unsigned int len,
		      const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, rtnl_nha_res_bucket_attrs,
		      "NHA_RES_BUCKET_???",
		      ARRSZ_PAIR(nha_res_bucket_nla_decoders), opaque_data);

	return true;
}

static const nla_decoder_t nhmsg_nla_decoders[] = {
	[NHA_UNSPEC]		= NULL,
	[NHA_ID]		= decode_nla_u32,
	[NHA_GROUP]		= decode_nha_nh_grp,
	[NHA_GROUP_TYPE]	= decode_nha_nh_grp_type,
	[NHA_BLACKHOLE]		= decode_nla_u32,
	[NHA_OIF]		= decode_nla_ifindex,
	[NHA_GATEWAY]		= decode_nha_addr,
	[NHA_ENCAP_TYPE]	= decode_nla_lwt_encap_type,
	[NHA_ENCAP]		= NULL, /* unimplemented */
	[NHA_GROUPS]		= decode_nla_u32,
	[NHA_MASTER]		= decode_nla_ifindex,
	[NHA_FDB]		= decode_nla_u32,
	[NHA_RES_GROUP]		= decode_nha_res_group,
	[NHA_RES_BUCKET]	= decode_nha_res_bucket,
};

DECL_NETLINK_ROUTE_DECODER(decode_nhmsg)
{
	struct nhmsg nhmsg = { .nh_family = family };
	size_t offset = sizeof(nhmsg.nh_family);
	bool decode_nla = false;

	tprint_struct_begin();
	PRINT_FIELD_XVAL(nhmsg, nh_family, addrfams, "AF_???");
	tprint_struct_next();

	if (len >= sizeof(nhmsg)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(nhmsg) - offset,
					 (char *) &nhmsg + offset)) {
			PRINT_FIELD_XVAL(nhmsg, nh_scope,
					 routing_scopes, NULL);
			tprint_struct_next();
			PRINT_FIELD_XVAL(nhmsg, nh_protocol,
					 routing_protocols, "RTPROT_???");
			if (nhmsg.resvd) {
				tprint_struct_next();
				PRINT_FIELD_X(nhmsg, resvd);
			}
			tprint_struct_next();
			PRINT_FIELD_FLAGS(nhmsg, nh_flags,
					  route_nexthop_flags, "RTNH_F_???");
			decode_nla = true;
		}
	} else {
		tprint_more_data_follows();
	}
	tprint_struct_end();

	offset = NLMSG_ALIGN(sizeof(nhmsg));
	if (decode_nla && len > offset) {
		tprint_array_next();
		decode_nlattr(tcp, addr + offset, len - offset,
			      rtnl_nexthop_attrs, "NHA_???",
			      ARRSZ_PAIR(nhmsg_nla_decoders), &nhmsg);
	}
}
