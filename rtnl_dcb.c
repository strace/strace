/*
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#ifdef HAVE_STRUCT_DCBMSG

# include "netlink_route.h"
# include "nlattr.h"
# include "print_fields.h"

# include <linux/dcbnl.h>
# include "netlink.h"

# include "xlat/dcb_commands.h"
# include "xlat/rtnl_dcb_attrs.h"

DECL_NETLINK_ROUTE_DECODER(decode_dcbmsg)
{
	struct dcbmsg dcb = { .dcb_family = family };
	size_t offset = sizeof(dcb.dcb_family);
	bool decode_nla = false;

	PRINT_FIELD_XVAL("{", dcb, dcb_family, addrfams, "AF_???");

	tprints(", ");
	if (len >= sizeof(dcb)) {
		if (!umoven_or_printaddr(tcp, addr + offset,
					 sizeof(dcb) - offset,
					 (char *) &dcb + offset)) {
			PRINT_FIELD_XVAL("", dcb, cmd,
					 dcb_commands, "DCB_CMD_???");
			decode_nla = true;
		}
	} else
		tprints("...");
	tprints("}");

	offset = NLMSG_ALIGN(sizeof(dcb));
	if (decode_nla && len > offset) {
		tprints(", ");
		decode_nlattr(tcp, addr + offset, len - offset,
			      rtnl_dcb_attrs, "DCB_ATTR_???", NULL, 0, NULL);
	}
}

#endif
