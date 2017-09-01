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
#ifdef HAVE_LINUX_NEIGHBOUR_H
# include <linux/neighbour.h>
#endif

#include "xlat/rtnl_neightbl_attrs.h"
#include "xlat/rtnl_neightbl_parms_attrs.h"

static bool
decode_ndt_config(struct tcb *const tcp,
		  const kernel_ulong_t addr,
		  const unsigned int len,
		  const void *const opaque_data)
{
#ifdef HAVE_STRUCT_NDT_CONFIG
	struct ndt_config ndtc;

	if (len < sizeof(ndtc))
		return false;
	else if (!umove_or_printaddr(tcp, addr, &ndtc)) {
		PRINT_FIELD_U("{", ndtc, ndtc_key_len);
		PRINT_FIELD_U(", ", ndtc, ndtc_entry_size);
		PRINT_FIELD_U(", ", ndtc, ndtc_entries);
		PRINT_FIELD_U(", ", ndtc, ndtc_last_flush);
		PRINT_FIELD_U(", ", ndtc, ndtc_last_rand);
		PRINT_FIELD_U(", ", ndtc, ndtc_hash_rnd);
		PRINT_FIELD_0X(", ", ndtc, ndtc_hash_mask);
		PRINT_FIELD_U(", ", ndtc, ndtc_hash_chain_gc);
		PRINT_FIELD_U(", ", ndtc, ndtc_proxy_qlen);
		tprints("}");
	}

	return true;
#else
	return false;
#endif
}

static const nla_decoder_t ndt_parms_nla_decoders[] = {
	[NDTPA_IFINDEX]			= decode_nla_ifindex,
	[NDTPA_REFCNT]			= decode_nla_u32,
	[NDTPA_REACHABLE_TIME]		= decode_nla_u64,
	[NDTPA_BASE_REACHABLE_TIME]	= decode_nla_u64,
	[NDTPA_RETRANS_TIME]		= decode_nla_u64,
	[NDTPA_GC_STALETIME]		= decode_nla_u64,
	[NDTPA_DELAY_PROBE_TIME]	= decode_nla_u64,
	[NDTPA_QUEUE_LEN]		= decode_nla_u32,
	[NDTPA_APP_PROBES]		= decode_nla_u32,
	[NDTPA_UCAST_PROBES]		= decode_nla_u32,
	[NDTPA_MCAST_PROBES]		= decode_nla_u32,
	[NDTPA_ANYCAST_DELAY]		= decode_nla_u64,
	[NDTPA_PROXY_DELAY]		= decode_nla_u64,
	[NDTPA_PROXY_QLEN]		= decode_nla_u32,
	[NDTPA_LOCKTIME]		= decode_nla_u64,
	[NDTPA_QUEUE_LENBYTES]		= decode_nla_u32,
	[NDTPA_MCAST_REPROBES]		= decode_nla_u32,
	[NDTPA_PAD]			= NULL
};

static bool
decode_ndta_parms(struct tcb *const tcp,
		  const kernel_ulong_t addr,
		  const unsigned int len,
		  const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, rtnl_neightbl_parms_attrs, "NDTPA_???",
		      ndt_parms_nla_decoders,
		      ARRAY_SIZE(ndt_parms_nla_decoders), opaque_data);

	return true;
}

static bool
decode_ndt_stats(struct tcb *const tcp,
		 const kernel_ulong_t addr,
		 const unsigned int len,
		 const void *const opaque_data)
{
#ifdef HAVE_STRUCT_NDT_STATS
	struct ndt_stats ndtst;
	const unsigned int min_size =
		offsetofend(struct ndt_stats, ndts_forced_gc_runs);
	const unsigned int def_size = sizeof(ndtst);
	const unsigned int size =
		(len >= def_size) ? def_size :
				    ((len == min_size) ? min_size : 0);

	if (!size)
		return false;

	if (!umoven_or_printaddr(tcp, addr, size, &ndtst)) {
		PRINT_FIELD_U("{", ndtst, ndts_allocs);
		PRINT_FIELD_U(", ", ndtst, ndts_destroys);
		PRINT_FIELD_U(", ", ndtst, ndts_hash_grows);
		PRINT_FIELD_U(", ", ndtst, ndts_res_failed);
		PRINT_FIELD_U(", ", ndtst, ndts_lookups);
		PRINT_FIELD_U(", ", ndtst, ndts_hits);
		PRINT_FIELD_U(", ", ndtst, ndts_rcv_probes_mcast);
		PRINT_FIELD_U(", ", ndtst, ndts_rcv_probes_ucast);
		PRINT_FIELD_U(", ", ndtst, ndts_periodic_gc_runs);
		PRINT_FIELD_U(", ", ndtst, ndts_forced_gc_runs);
#ifdef HAVE_STRUCT_NDT_STATS_NDTS_TABLE_FULLS
		if (len >= def_size)
			PRINT_FIELD_U(", ", ndtst, ndts_table_fulls);
#endif
		tprints("}");
	}

	return true;
#else
	return false;
#endif
}

static const nla_decoder_t ndtmsg_nla_decoders[] = {
	[NDTA_NAME]		= decode_nla_str,
	[NDTA_THRESH1]		= decode_nla_u32,
	[NDTA_THRESH2]		= decode_nla_u32,
	[NDTA_THRESH3]		= decode_nla_u32,
	[NDTA_CONFIG]		= decode_ndt_config,
	[NDTA_PARMS]		= decode_ndta_parms,
	[NDTA_STATS]		= decode_ndt_stats,
	[NDTA_GC_INTERVAL]	= decode_nla_u64,
	[NDTA_PAD]		= NULL,
};

DECL_NETLINK_ROUTE_DECODER(decode_ndtmsg)
{
	struct ndtmsg ndtmsg = { .ndtm_family = family };

	PRINT_FIELD_XVAL("{", ndtmsg, ndtm_family, addrfams, "AF_???");
	tprints("}");

	const size_t offset = NLMSG_ALIGN(sizeof(ndtmsg));
	if (len > offset) {
		tprints(", ");
		decode_nlattr(tcp, addr + offset, len - offset,
			      rtnl_neightbl_attrs, "NDTA_???",
			      ndtmsg_nla_decoders,
			      ARRAY_SIZE(ndtmsg_nla_decoders), NULL);
	}
}
