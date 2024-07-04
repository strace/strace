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
			[CTRL_ATTR_OPS] = NULL,
			[CTRL_ATTR_MCAST_GROUPS] = NULL,
			[CTRL_ATTR_POLICY] = NULL,
			[CTRL_ATTR_OP_POLICY] = NULL,
			[CTRL_ATTR_OP] = decode_nla_u32,
		};

		tprint_array_next();
		decode_nlattr(tcp, addr, len, genl_ctrl_attr, "CTRL_ATTR_???",
			      ARRSZ_PAIR(decoders), NULL);
	}
}
