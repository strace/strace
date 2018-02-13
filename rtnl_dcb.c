/*
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2016-2018 The strace developers.
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
