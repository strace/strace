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
#include <linux/pkt_cls.h>
#include <linux/rtnetlink.h>

#include "xlat/rtnl_tc_action_attrs.h"
#include "xlat/rtnl_tca_act_flags.h"
#include "xlat/rtnl_tca_act_hw_stats.h"


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

static const nla_decoder_t tcamsg_nla_decoders[] = {
	[TCA_ACT_KIND]		= decode_nla_str,
	[TCA_ACT_OPTIONS]	= NULL, /* unimplemented */
	[TCA_ACT_INDEX]		= decode_nla_u32,
	[TCA_ACT_STATS]		= decode_nla_tc_stats,
	[TCA_ACT_PAD]		= NULL,
	[TCA_ACT_COOKIE]	= NULL, /* default parser */
	[TCA_ACT_FLAGS]		= decode_tca_act_flags,
	[TCA_ACT_HW_STATS]	= decode_tca_act_hw_stats,
	[TCA_ACT_USED_HW_STATS]	= decode_tca_act_hw_stats,
};

DECL_NETLINK_ROUTE_DECODER(decode_tcamsg)
{
	struct tcamsg tca = { .tca_family = family };

	PRINT_FIELD_XVAL("{", tca, tca_family, addrfams, "AF_???");
	tprints("}");

	const size_t offset = NLMSG_ALIGN(sizeof(tca));
	if (len > offset) {
		tprints(", ");
		decode_nlattr(tcp, addr + offset, len - offset,
			      rtnl_tc_action_attrs, "TCA_ACT_???",
			      tcamsg_nla_decoders,
			      ARRAY_SIZE(tcamsg_nla_decoders), NULL);
	}
}
