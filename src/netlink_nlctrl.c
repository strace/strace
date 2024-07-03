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
		tprint_array_next();
		decode_nlattr(tcp, addr, len, NULL, NULL, NULL, 0, NULL);
	}
}
