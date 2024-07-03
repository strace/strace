/*
 * Copyright (c) 2017-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "netlink_generic.h"
#include "nlattr.h"

static
DECL_NETLINK_GENERIC_DECODER(decode_genl_msg)
{
	tprint_struct_begin();
	PRINT_FIELD_X(*hdr, cmd);
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

typedef DECL_NETLINK_GENERIC_DECODER((*netlink_generic_decoder_t));

static netlink_generic_decoder_t
lookup_genl_decoder(const char *family)
{
	static const struct {
		const char *family;
		netlink_generic_decoder_t decoder;
	} decoders[] = {
		{ "nlctrl", decode_nlctrl },
	};

	if (family) {
		for (size_t i = 0; i < ARRAY_SIZE(decoders); ++i) {
			if (strcmp(family, decoders[i].family) == 0) {
				return decoders[i].decoder;
			}
		}
	}

        return decode_genl_msg;
}

bool
decode_netlink_generic(struct tcb *const tcp,
		       const struct nlmsghdr *const nlmsghdr,
		       const kernel_ulong_t addr,
		       const unsigned int len)
{
	struct genlmsghdr h;

	if (len < GENL_HDRLEN || nlmsghdr->nlmsg_type == NLMSG_DONE)
		return false;

	if (!umove_or_printaddr(tcp, addr, &h)) {
		const char *family =
			xlookup(genl_families_xlat(tcp), nlmsghdr->nlmsg_type);
		netlink_generic_decoder_t decoder = lookup_genl_decoder(family);
		decoder(tcp, &h, addr + GENL_HDRLEN, len - GENL_HDRLEN);
	}

	return true;
}
