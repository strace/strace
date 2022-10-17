/*
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2016-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "netlink_route.h"
#include "nlattr.h"

#include <linux/neighbour.h>

#include "xlat/rtnl_neightbl_attrs.h"
#include "xlat/rtnl_neightbl_parms_attrs.h"

static bool
decode_ndt_config(struct tcb *const tcp,
		  const kernel_ulong_t addr,
		  const unsigned int len,
		  const void *const opaque_data)
{
	struct ndt_config ndtc;

	if (len < sizeof(ndtc))
		return false;
	else if (!umove_or_printaddr(tcp, addr, &ndtc)) {
		tprint_struct_begin();
		PRINT_FIELD_U(ndtc, ndtc_key_len);
		tprint_struct_next();
		PRINT_FIELD_U(ndtc, ndtc_entry_size);
		tprint_struct_next();
		PRINT_FIELD_U(ndtc, ndtc_entries);
		tprint_struct_next();
		PRINT_FIELD_U(ndtc, ndtc_last_flush);
		tprint_struct_next();
		PRINT_FIELD_U(ndtc, ndtc_last_rand);
		tprint_struct_next();
		PRINT_FIELD_U(ndtc, ndtc_hash_rnd);
		tprint_struct_next();
		PRINT_FIELD_0X(ndtc, ndtc_hash_mask);
		tprint_struct_next();
		PRINT_FIELD_U(ndtc, ndtc_hash_chain_gc);
		tprint_struct_next();
		PRINT_FIELD_U(ndtc, ndtc_proxy_qlen);
		tprint_struct_end();
	}

	return true;
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
	[NDTPA_PAD]			= NULL,
	[NDTPA_INTERVAL_PROBE_TIME_MS]	= decode_nla_u64,
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
		tprint_struct_begin();
		PRINT_FIELD_U(ndtst, ndts_allocs);
		tprint_struct_next();
		PRINT_FIELD_U(ndtst, ndts_destroys);
		tprint_struct_next();
		PRINT_FIELD_U(ndtst, ndts_hash_grows);
		tprint_struct_next();
		PRINT_FIELD_U(ndtst, ndts_res_failed);
		tprint_struct_next();
		PRINT_FIELD_U(ndtst, ndts_lookups);
		tprint_struct_next();
		PRINT_FIELD_U(ndtst, ndts_hits);
		tprint_struct_next();
		PRINT_FIELD_U(ndtst, ndts_rcv_probes_mcast);
		tprint_struct_next();
		PRINT_FIELD_U(ndtst, ndts_rcv_probes_ucast);
		tprint_struct_next();
		PRINT_FIELD_U(ndtst, ndts_periodic_gc_runs);
		tprint_struct_next();
		PRINT_FIELD_U(ndtst, ndts_forced_gc_runs);
		if (len >= def_size) {
			tprint_struct_next();
			PRINT_FIELD_U(ndtst, ndts_table_fulls);
		}
		tprint_struct_end();
	}

	return true;
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

	tprint_struct_begin();
	PRINT_FIELD_XVAL(ndtmsg, ndtm_family, addrfams, "AF_???");
	tprint_struct_end();

	const size_t offset = NLMSG_ALIGN(sizeof(ndtmsg));
	if (len > offset) {
		tprint_array_next();
		decode_nlattr(tcp, addr + offset, len - offset,
			      rtnl_neightbl_attrs, "NDTA_???",
			      ndtmsg_nla_decoders,
			      ARRAY_SIZE(ndtmsg_nla_decoders), NULL);
	}
}
