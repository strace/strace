/*
 * Copyright (c) 2017-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "netlink_generic.h"
#include "nlattr.h"

#include "xlat/genl_ctrl_cmd.h"
#include "xlat/genl_ctrl_attr.h"
#include "xlat/genl_ctrl_attr_op.h"
#include "xlat/genl_ctrl_attr_op_flags.h"
#include "xlat/genl_ctrl_attr_mcast_grp.h"
#include "xlat/genl_ctrl_attr_policy.h"

static
DECL_NLA(ctrl_attr_op_id)
{
	static const struct decode_nla_xlat_opts opts = {
		.xlat = genl_ctrl_cmd,
		.dflt = "CTRL_CMD_???",
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
	nla_decoder_t decoder = &decode_nla_ctrl_attr_op_item;
	decode_nlattr(tcp, addr, len, NULL, NULL, &decoder, 0, opaque_data);
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
	nla_decoder_t decoder = &decode_nla_ctrl_attr_mcast_group_item;
	decode_nlattr(tcp, addr, len, NULL, NULL, &decoder, 0, opaque_data);
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
	nla_decoder_t decoder = &decode_nla_ctrl_attr_op_policy_item;
	decode_nlattr(tcp, addr, len, NULL, NULL, &decoder, 0, opaque_data);
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
			[CTRL_ATTR_FAMILY_NAME] = decode_nla_str,
			[CTRL_ATTR_VERSION] = decode_nla_u32,
			[CTRL_ATTR_HDRSIZE] = decode_nla_u32,
			[CTRL_ATTR_MAXATTR] = decode_nla_u32,
			[CTRL_ATTR_OPS] = decode_nla_ctrl_attr_ops,
			[CTRL_ATTR_MCAST_GROUPS] = decode_nla_ctrl_attr_mcast_groups,
			[CTRL_ATTR_POLICY] = NULL,
			[CTRL_ATTR_OP_POLICY] = decode_nla_ctrl_attr_op_policy,
			[CTRL_ATTR_OP] = decode_nla_u32,
		};

		tprint_array_next();
		decode_nlattr(tcp, addr, len, genl_ctrl_attr, "CTRL_ATTR_???",
			      ARRSZ_PAIR(decoders), NULL);
	}
}
