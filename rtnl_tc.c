/*
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "netlink_route.h"
#include "nlattr.h"
#include "print_fields.h"

#include "netlink.h"
#ifdef HAVE_STRUCT_GNET_STATS_BASIC
# include <linux/gen_stats.h>
#endif
#include <linux/pkt_sched.h>
#include <linux/rtnetlink.h>

#include "xlat/rtnl_tc_attrs.h"
#include "xlat/rtnl_tca_stab_attrs.h"
#include "xlat/rtnl_tca_stats_attrs.h"

static bool
decode_tc_stats(struct tcb *const tcp,
		const kernel_ulong_t addr,
		const unsigned int len,
		const void *const opaque_data)
{
	struct tc_stats st;
	const unsigned int sizeof_tc_stats =
		offsetofend(struct tc_stats, backlog);

	if (len < sizeof_tc_stats)
		return false;
	else if (!umoven_or_printaddr(tcp, addr, sizeof_tc_stats, &st)) {
		PRINT_FIELD_U("{", st, bytes);
		PRINT_FIELD_U(", ", st, packets);
		PRINT_FIELD_U(", ", st, drops);
		PRINT_FIELD_U(", ", st, overlimits);
		PRINT_FIELD_U(", ", st, bps);
		PRINT_FIELD_U(", ", st, pps);
		PRINT_FIELD_U(", ", st, qlen);
		PRINT_FIELD_U(", ", st, backlog);
		tprints("}");
	}

	return true;
}

static bool
decode_tc_estimator(struct tcb *const tcp,
		    const kernel_ulong_t addr,
		    const unsigned int len,
		    const void *const opaque_data)
{
	struct tc_estimator est;

	if (len < sizeof(est))
		return false;
	else if (!umove_or_printaddr(tcp, addr, &est)) {
		PRINT_FIELD_D("{", est, interval);
		PRINT_FIELD_U(", ", est, ewma_log);
		tprints("}");
	}

	return true;
}

static bool
decode_gnet_stats_basic(struct tcb *const tcp,
			const kernel_ulong_t addr,
			const unsigned int len,
			const void *const opaque_data)
{
#ifdef HAVE_STRUCT_GNET_STATS_BASIC
	struct gnet_stats_basic sb;
	const unsigned int sizeof_st_basic =
		offsetofend(struct gnet_stats_basic, packets);

	if (len < sizeof_st_basic)
		return false;
	else if (!umoven_or_printaddr(tcp, addr, sizeof_st_basic, &sb)) {
		PRINT_FIELD_U("{", sb, bytes);
		PRINT_FIELD_U(", ", sb, packets);
		tprints("}");
	}

	return true;
#else
	return false;
#endif
}

static bool
decode_gnet_stats_rate_est(struct tcb *const tcp,
			   const kernel_ulong_t addr,
			   const unsigned int len,
			   const void *const opaque_data)
{
#ifdef HAVE_STRUCT_GNET_STATS_RATE_EST
	struct gnet_stats_rate_est est;

	if (len < sizeof(est))
		return false;
	else if (!umove_or_printaddr(tcp, addr, &est)) {
		PRINT_FIELD_U("{", est, bps);
		PRINT_FIELD_U(", ", est, pps);
		tprints("}");
	}

	return true;
#else
	return false;
#endif
}

static bool
decode_gnet_stats_queue(struct tcb *const tcp,
			const kernel_ulong_t addr,
			const unsigned int len,
			const void *const opaque_data)
{
#ifdef HAVE_STRUCT_GNET_STATS_QUEUE
	struct gnet_stats_queue qstats;

	if (len < sizeof(qstats))
		return false;
	else if (!umove_or_printaddr(tcp, addr, &qstats)) {
		PRINT_FIELD_U("{", qstats, qlen);
		PRINT_FIELD_U(", ", qstats, backlog);
		PRINT_FIELD_U(", ", qstats, drops);
		PRINT_FIELD_U(", ", qstats, requeues);
		PRINT_FIELD_U(", ", qstats, overlimits);
		tprints("}");
	}

	return true;
#else
	return false;
#endif
}

static bool
decode_gnet_stats_rate_est64(struct tcb *const tcp,
			     const kernel_ulong_t addr,
			     const unsigned int len,
			     const void *const opaque_data)
{
#ifdef HAVE_STRUCT_GNET_STATS_RATE_EST64
	struct gnet_stats_rate_est64 est;

	if (len < sizeof(est))
		return false;
	else if (!umove_or_printaddr(tcp, addr, &est)) {
		PRINT_FIELD_U("{", est, bps);
		PRINT_FIELD_U(", ", est, pps);
		tprints("}");
	}

	return true;
#else
	return false;
#endif
}

static const nla_decoder_t tca_stats_nla_decoders[] = {
	[TCA_STATS_BASIC]	= decode_gnet_stats_basic,
	[TCA_STATS_RATE_EST]	= decode_gnet_stats_rate_est,
	[TCA_STATS_QUEUE]	= decode_gnet_stats_queue,
	[TCA_STATS_APP]		= NULL, /* unimplemented */
	[TCA_STATS_RATE_EST64]	= decode_gnet_stats_rate_est64,
	[TCA_STATS_PAD]		= NULL,
	[TCA_STATS_BASIC_HW]	= decode_gnet_stats_basic,
	[TCA_STATS_PKT64]	= decode_nla_u64,
};

bool
decode_nla_tc_stats(struct tcb *const tcp,
		    const kernel_ulong_t addr,
		    const unsigned int len,
		    const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, rtnl_tca_stats_attrs, "TCA_STATS_???",
		      tca_stats_nla_decoders,
		      ARRAY_SIZE(tca_stats_nla_decoders), opaque_data);

	return true;
}

static bool
decode_tc_sizespec(struct tcb *const tcp,
		   const kernel_ulong_t addr,
		   const unsigned int len,
		   const void *const opaque_data)
{
#ifdef HAVE_STRUCT_TC_SIZESPEC
	struct tc_sizespec s;

	if (len < sizeof(s))
		return false;
	else if (!umove_or_printaddr(tcp, addr, &s)) {
		PRINT_FIELD_U("{", s, cell_log);
		PRINT_FIELD_U(", ", s, size_log);
		PRINT_FIELD_D(", ", s, cell_align);
		PRINT_FIELD_D(", ", s, overhead);
		PRINT_FIELD_U(", ", s, linklayer);
		PRINT_FIELD_U(", ", s, mpu);
		PRINT_FIELD_U(", ", s, mtu);
		PRINT_FIELD_U(", ", s, tsize);
		tprints("}");
	}

	return true;
#else
	return false;
#endif
}

static bool
print_stab_data(struct tcb *const tcp, void *const elem_buf,
		const size_t elem_size, void *const opaque_data)
{
	tprintf("%" PRIu16, *(uint16_t *) elem_buf);

	return true;
}

static bool
decode_tca_stab_data(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const unsigned int len,
		     const void *const opaque_data)
{
	uint16_t data;
	const size_t nmemb = len / sizeof(data);

	if (!nmemb)
		return false;

	print_array(tcp, addr, nmemb, &data, sizeof(data),
		    tfetch_mem, print_stab_data, NULL);

	return true;
}

static const nla_decoder_t tca_stab_nla_decoders[] = {
	[TCA_STAB_BASE]	= decode_tc_sizespec,
	[TCA_STAB_DATA] = decode_tca_stab_data
};

static bool
decode_tca_stab(struct tcb *const tcp,
		const kernel_ulong_t addr,
		const unsigned int len,
		const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, rtnl_tca_stab_attrs, "TCA_STAB_???",
		      tca_stab_nla_decoders,
		      ARRAY_SIZE(tca_stab_nla_decoders), opaque_data);

	return true;
}

static const nla_decoder_t tcmsg_nla_decoders[] = {
	[TCA_KIND]		= decode_nla_str,
	[TCA_OPTIONS]		= NULL, /* unimplemented */
	[TCA_STATS]		= decode_tc_stats,
	[TCA_XSTATS]		= NULL, /* unimplemented */
	[TCA_RATE]		= decode_tc_estimator,
	[TCA_FCNT]		= decode_nla_u32,
	[TCA_STATS2]		= decode_nla_tc_stats,
	[TCA_STAB]		= decode_tca_stab,
	[TCA_PAD]		= NULL,
	[TCA_DUMP_INVISIBLE]	= NULL,
	[TCA_CHAIN]		= decode_nla_u32,
	[TCA_HW_OFFLOAD]	= decode_nla_u8,
	[TCA_INGRESS_BLOCK]	= decode_nla_u32,
	[TCA_EGRESS_BLOCK]	= decode_nla_u32,
	[TCA_DUMP_FLAGS]	= decode_nla_u32,
};

DECL_NETLINK_ROUTE_DECODER(decode_tcmsg)
{
	struct tcmsg tcmsg = { .tcm_family = family };
	size_t offset = sizeof(tcmsg.tcm_family);
	bool decode_nla = false;

	PRINT_FIELD_XVAL("{", tcmsg, tcm_family, addrfams, "AF_???");

	tprints(", ");
	if (len >= sizeof(tcmsg)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(tcmsg) - offset,
					 (char *) &tcmsg + offset)) {
			PRINT_FIELD_IFINDEX("", tcmsg, tcm_ifindex);
			PRINT_FIELD_U(", ", tcmsg, tcm_handle);
			PRINT_FIELD_U(", ", tcmsg, tcm_parent);
			PRINT_FIELD_U(", ", tcmsg, tcm_info);
			decode_nla = true;
		}
	} else
		tprints("...");
	tprints("}");

	offset = NLMSG_ALIGN(sizeof(tcmsg));
	if (decode_nla && len > offset) {
		tprints(", ");
		decode_nlattr(tcp, addr + offset, len - offset,
			      rtnl_tc_attrs, "TCA_???", tcmsg_nla_decoders,
			      ARRAY_SIZE(tcmsg_nla_decoders), NULL);
	}
}
