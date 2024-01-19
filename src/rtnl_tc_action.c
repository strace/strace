/*
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2016-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "netlink_route.h"
#include "nlattr.h"

#include "netlink.h"
#include <linux/pkt_cls.h>
#include <linux/rtnetlink.h>

#include "xlat/rtnl_tc_action_attrs.h"
#include "xlat/rtnl_tca_act_flags.h"
#include "xlat/rtnl_tca_act_hw_stats.h"
#include "xlat/rtnl_tca_root_flags.h"
#include "xlat/rtnl_tca_root_attrs.h"


static bool
decode_tca_act_flags(struct tcb *const tcp,
		        const kernel_ulong_t addr,
		        const unsigned int len,
		        const void *const opaque_data)
{
	static const struct decode_nla_xlat_opts opts = {
		rtnl_tca_act_flags, "TCA_ACT_FLAGS_???",
		.size = 4,
	};

	return decode_nla_flags(tcp, addr, len, &opts);
}

static bool
decode_tca_act_hw_stats(struct tcb *const tcp,
		        const kernel_ulong_t addr,
		        const unsigned int len,
		        const void *const opaque_data)
{
	static const struct decode_nla_xlat_opts opts = {
		rtnl_tca_act_hw_stats, "TCA_ACT_HW_STATS_???",
		.size = 4,
	};

	return decode_nla_flags(tcp, addr, len, &opts);
}

static const nla_decoder_t tca_act_nla_decoders[] = {
	[TCA_ACT_KIND]		= decode_nla_str,
	[TCA_ACT_OPTIONS]	= NULL, /* unimplemented */
	[TCA_ACT_INDEX]		= decode_nla_u32,
	[TCA_ACT_STATS]		= decode_nla_tc_stats,
	[TCA_ACT_PAD]		= NULL,
	[TCA_ACT_COOKIE]	= NULL, /* default parser */
	[TCA_ACT_FLAGS]		= decode_tca_act_flags,
	[TCA_ACT_HW_STATS]	= decode_tca_act_hw_stats,
	[TCA_ACT_USED_HW_STATS]	= decode_tca_act_hw_stats,
	[TCA_ACT_IN_HW_COUNT]	= decode_nla_u32,
};

static bool
decode_tca_action(struct tcb *const tcp,
		  const kernel_ulong_t addr,
		  const unsigned int len,
		  const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, rtnl_tc_action_attrs, "TCA_ACT_???",
		      ARRSZ_PAIR(tca_act_nla_decoders), NULL);

	return true;
}

static bool
decode_tca_root_act_tab(struct tcb *const tcp,
			const kernel_ulong_t addr,
			const unsigned int len,
			const void *const opaque_data)
{
	nla_decoder_t tca_action_decoder = &decode_tca_action;

	/* TCA_ROOT_TAB (nee TCA_ACT_TAB) misuses nesting for array */
	decode_nlattr(tcp, addr, len, NULL, NULL,
		      &tca_action_decoder, 0, NULL);

	return true;
}

static bool
decode_tca_root_act_flags(struct tcb *const tcp,
			  const kernel_ulong_t addr,
			  const unsigned int len,
			  const void *const opaque_data)
{
	static const struct decode_nla_xlat_opts opts = {
		rtnl_tca_root_flags, "TCA_ACT_FLAG_???",
		.size = 4,
	};

	return decode_nla_flags(tcp, addr, len, &opts);
}

static bool
decode_tca_msecs(struct tcb *const tcp,
		 const kernel_ulong_t addr,
		 const unsigned int len,
		 const void *const opaque_data)
{
	uint64_t val;

	if (len > sizeof(val))
		return false;

	if (!umoven_to_uint64_or_printaddr(tcp, addr, len, &val))
		print_ticks(val, 1000, 3);

	return true;
}

static const nla_decoder_t tcamsg_nla_decoders[] = {
	[TCA_ROOT_UNSPEC]	= NULL,
	[TCA_ROOT_TAB]		= decode_tca_root_act_tab,
	[TCA_ROOT_FLAGS]	= decode_tca_root_act_flags,
	[TCA_ROOT_COUNT]	= decode_nla_u32,
	[TCA_ROOT_TIME_DELTA]	= decode_tca_msecs,
	[TCA_ROOT_EXT_WARN_MSG]	= decode_nla_str,
};

DECL_NETLINK_ROUTE_DECODER(decode_tcamsg)
{
	struct tcamsg tca = { .tca_family = family };

	tprint_struct_begin();
	PRINT_FIELD_XVAL(tca, tca_family, addrfams, "AF_???");
	tprint_struct_end();

	const size_t offset = NLMSG_ALIGN(sizeof(tca));
	if (len > offset) {
		tprint_array_next();
		decode_nlattr(tcp, addr + offset, len - offset,
			      rtnl_tca_root_attrs, "TCA_ROOT_???",
			      tcamsg_nla_decoders,
			      ARRAY_SIZE(tcamsg_nla_decoders), NULL);
	}
}
