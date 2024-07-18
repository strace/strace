/*
 * Copyright (c) 2017-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "netlink_generic.h"
#include "nlattr.h"
#include <linux/cgroupstats.h>
#include <linux/devlink.h>
#include <linux/ethtool_netlink.h>
#include <linux/ioam6_genl.h>
#include <linux/mptcp_pm.h>
#include <linux/netdev.h>
#include <linux/nl80211.h>
#include <linux/seg6_genl.h>
#include <linux/tcp_metrics.h>
#include <linux/thermal.h>

#include "xlat/genl_ctrl_cmd.h"
#include "xlat/genl_ctrl_attr.h"
#include "xlat/genl_ctrl_attr_op.h"
#include "xlat/genl_ctrl_attr_op_flags.h"
#include "xlat/genl_ctrl_attr_mcast_grp.h"
#include "xlat/genl_ctrl_attr_policy.h"
#include "xlat/genl_devlink_cmd.h"
#include "xlat/genl_ethtool_msg_send.h"
#include "xlat/genl_ethtool_msg_recv.h"
#include "xlat/genl_ioam6_cmd.h"
#include "xlat/genl_mptcp_pm_cmd.h"
#include "xlat/genl_netdev_cmd.h"
#include "xlat/genl_nl80211_cmd.h"
#include "xlat/genl_seg6_cmd.h"
#include "xlat/genl_taskstats_cmd.h"
#include "xlat/genl_tcp_metrics_cmd.h"
#include "xlat/genl_thermal_cmd.h"
#include "xlat/nl_attr_type.h"
#include "xlat/nl_policy_type_attr.h"

#define ARG_PAIR(arg) { arg, arg }
static const struct {
	const char *family;
	const struct xlat *cmd[2];
	const char *dflt;
} family_names[] = {
	{
		"nlctrl",
		ARG_PAIR(genl_ctrl_cmd),
		"CTRL_CMD_???"
	}, {
		DEVLINK_GENL_NAME,
		ARG_PAIR(genl_devlink_cmd),
		"DEVLINK_CMD_???"
	}, {
		ETHTOOL_GENL_NAME,
		{ genl_ethtool_msg_send, genl_ethtool_msg_recv },
		"ETHTOOL_MSG_???"
	}, {
		IOAM6_GENL_NAME,
		ARG_PAIR(genl_ioam6_cmd),
		"IOAM6_CMD_???"
	}, {
		MPTCP_PM_NAME,
		ARG_PAIR(genl_mptcp_pm_cmd),
		"MPTCP_PM_CMD_???"
	}, {
		NETDEV_FAMILY_NAME,
		ARG_PAIR(genl_netdev_cmd),
		"NETDEV_CMD_???"
	}, {
		NL80211_GENL_NAME,
		ARG_PAIR(genl_nl80211_cmd),
		"NL80211_CMD_???"
	}, {
		SEG6_GENL_NAME,
		ARG_PAIR(genl_seg6_cmd),
		"SEG6_CMD_???"
	}, {
		TASKSTATS_GENL_NAME,
		ARG_PAIR(genl_taskstats_cmd),
		"TASKSTATS_CMD_???"
	}, {
		TCP_METRICS_GENL_NAME,
		ARG_PAIR(genl_tcp_metrics_cmd),
		"TCP_METRICS_CMD_???"
	}, {
		THERMAL_GENL_FAMILY_NAME,
		ARG_PAIR(genl_thermal_cmd),
		"THERMAL_GENL_CMD_???"
	},
};
#undef ARG_PAIR

static
DECL_NLA(ctrl_attr_family_name)
{
	char name[GENL_NAMSIZ];

	if (len <= sizeof(name) && umoven(tcp, addr, len, name) == 0) {
		unsigned int *idx = (void *) opaque_data;
		for (unsigned int i = 0; i < ARRAY_SIZE(family_names); ++i) {
			if (strncmp(name, family_names[i].family, len) == 0) {
				*idx = i;
				break;
			}
		}
	}

	return decode_nla_str(tcp, addr, len, NULL);
}

static
DECL_NLA(ctrl_attr_op_id)
{
	const unsigned int *idx = opaque_data;
	struct decode_nla_xlat_opts opts = {
		.xlat = family_names[*idx].cmd[!entering(tcp)],
		.dflt = family_names[*idx].dflt,
		.size = 4,
	};

	return decode_nla_xval(tcp, addr, len, &opts);
}

static
DECL_NLA(ctrl_attr_op_flags)
{
	static const struct decode_nla_xlat_opts opts = {
		.xlat = genl_ctrl_attr_op_flags,
		.dflt = "GENL_???",
		.size = 4,
	};

	return decode_nla_flags(tcp, addr, len, &opts);
}

static
DECL_NLA(ctrl_attr_op_item)
{
	static const nla_decoder_t decoders[] = {
		[CTRL_ATTR_OP_UNSPEC] = NULL,
		[CTRL_ATTR_OP_ID] = decode_nla_ctrl_attr_op_id,
		[CTRL_ATTR_OP_FLAGS] = decode_nla_ctrl_attr_op_flags,
	};

	decode_nlattr(tcp, addr, len,
		      genl_ctrl_attr_op, "CTRL_ATTR_OP_???",
		      ARRSZ_PAIR(decoders), opaque_data);
	return true;
}

static
DECL_NLA(ctrl_attr_ops)
{
	decode_nlattr_notype(tcp, addr, len, NULL, NULL,
			     decode_nla_ctrl_attr_op_item, opaque_data);
	return true;
}

static
DECL_NLA(ctrl_attr_mcast_group_item)
{
	static const nla_decoder_t decoders[] = {
		[CTRL_ATTR_MCAST_GRP_UNSPEC] = NULL,
		[CTRL_ATTR_MCAST_GRP_NAME] = decode_nla_str,
		[CTRL_ATTR_MCAST_GRP_ID] = decode_nla_x32,
	};

	decode_nlattr(tcp, addr, len,
		      genl_ctrl_attr_mcast_grp, "CTRL_ATTR_MCAST_GRP_???",
		      ARRSZ_PAIR(decoders), opaque_data);
	return true;
}

static
DECL_NLA(ctrl_attr_mcast_groups)
{
	decode_nlattr_notype(tcp, addr, len, NULL, NULL,
			     decode_nla_ctrl_attr_mcast_group_item, opaque_data);
	return true;
}

static
DECL_NLA(policy_type_attr_type)
{
	static const struct decode_nla_xlat_opts opts = {
		.xlat = nl_attr_type,
		.dflt = "NL_ATTR_TYPE_???",
		.size = 4,
	};

	return decode_nla_xval(tcp, addr, len, &opts);
}

static
DECL_NLA(ctrl_attr_policy_attr)
{
	static const nla_decoder_t decoders[] = {
		[NL_POLICY_TYPE_ATTR_UNSPEC] = NULL,
		[NL_POLICY_TYPE_ATTR_TYPE] = decode_nla_policy_type_attr_type,
		[NL_POLICY_TYPE_ATTR_MIN_VALUE_S] = decode_nla_s64,
		[NL_POLICY_TYPE_ATTR_MAX_VALUE_S] = decode_nla_s64,
		[NL_POLICY_TYPE_ATTR_MIN_VALUE_U] = decode_nla_u64,
		[NL_POLICY_TYPE_ATTR_MAX_VALUE_U] = decode_nla_u64,
		[NL_POLICY_TYPE_ATTR_MIN_LENGTH] = decode_nla_u32,
		[NL_POLICY_TYPE_ATTR_MAX_LENGTH] = decode_nla_u32,
		[NL_POLICY_TYPE_ATTR_POLICY_IDX] = decode_nla_x32,
		[NL_POLICY_TYPE_ATTR_POLICY_MAXTYPE] = decode_nla_x32,
		[NL_POLICY_TYPE_ATTR_BITFIELD32_MASK] = decode_nla_x32,
		[NL_POLICY_TYPE_ATTR_PAD] = NULL,
		[NL_POLICY_TYPE_ATTR_MASK] = decode_nla_x64,
	};

	decode_nlattr(tcp, addr, len,
		      nl_policy_type_attr, "NL_POLICY_TYPE_ATTR_???",
		      ARRSZ_PAIR(decoders), opaque_data);
	return true;
}

static
DECL_NLA(ctrl_attr_policy_item)
{
	decode_nlattr_notype(tcp, addr, len, NULL, NULL,
			     decode_nla_ctrl_attr_policy_attr, opaque_data);
	return true;
}

static
DECL_NLA(ctrl_attr_policy)
{
	decode_nlattr_notype(tcp, addr, len, NULL, NULL,
			     decode_nla_ctrl_attr_policy_item, opaque_data);
	return true;
}

static
DECL_NLA(ctrl_attr_op_policy_item)
{
	static const nla_decoder_t decoders[] = {
		[CTRL_ATTR_POLICY_UNSPEC] = NULL,
		[CTRL_ATTR_POLICY_DO] = decode_nla_u32,
		[CTRL_ATTR_POLICY_DUMP] = decode_nla_u32,
	};

	decode_nlattr(tcp, addr, len,
		      genl_ctrl_attr_policy, "CTRL_ATTR_POLICY_???",
		      ARRSZ_PAIR(decoders), opaque_data);
	return true;
}

static
DECL_NLA(ctrl_attr_op_policy)
{
	decode_nlattr_notype(tcp, addr, len, NULL, NULL,
			     decode_nla_ctrl_attr_op_policy_item, opaque_data);
	return true;
}

DECL_NETLINK_GENERIC_DECODER(decode_nlctrl)
{
	tprint_struct_begin();
	PRINT_FIELD_XVAL(*hdr, cmd, genl_ctrl_cmd, "CTRL_CMD_???");
	tprint_struct_next();
	PRINT_FIELD_U(*hdr, version);
	if (hdr->reserved) {
		tprint_struct_next();
		PRINT_FIELD_X(*hdr, reserved);
	}
	tprint_struct_end();

	if (len > 0) {
		static const nla_decoder_t decoders[] = {
			[CTRL_ATTR_UNSPEC] = NULL,
			[CTRL_ATTR_FAMILY_ID] = decode_nla_x16,
			[CTRL_ATTR_FAMILY_NAME] = decode_nla_ctrl_attr_family_name,
			[CTRL_ATTR_VERSION] = decode_nla_u32,
			[CTRL_ATTR_HDRSIZE] = decode_nla_u32,
			[CTRL_ATTR_MAXATTR] = decode_nla_u32,
			[CTRL_ATTR_OPS] = decode_nla_ctrl_attr_ops,
			[CTRL_ATTR_MCAST_GROUPS] = decode_nla_ctrl_attr_mcast_groups,
			[CTRL_ATTR_POLICY] = decode_nla_ctrl_attr_policy,
			[CTRL_ATTR_OP_POLICY] = decode_nla_ctrl_attr_op_policy,
			[CTRL_ATTR_OP] = decode_nla_u32,
		};

		unsigned int family_names_idx = 0;

		tprint_array_next();
		decode_nlattr(tcp, addr, len, genl_ctrl_attr, "CTRL_ATTR_???",
			      ARRSZ_PAIR(decoders), &family_names_idx);
	}
}
