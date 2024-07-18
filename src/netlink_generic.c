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

static struct {
	const char *family;
	const netlink_generic_decoder_t decoder;
	uint16_t id;		/* Assigned dynamically */
} genl_decoders[] = {
	{ "nlctrl", decode_nlctrl, 0 },
};

static void
initialize_genl_decoders(struct tcb *const tcp)
{
	static bool initialized = false;

	if (initialized)
		return;
	initialized = true;

	const struct xlat *xlat = genl_families_xlat(tcp);
	for (size_t i = 0; i < ARRAY_SIZE(genl_decoders); ++i)
		genl_decoders[i].id =
			xrlookup(xlat, genl_decoders[i].family, 0);
}

static netlink_generic_decoder_t
lookup_genl_decoder(uint16_t id)
{
	for (size_t i = 0; i < ARRAY_SIZE(genl_decoders); ++i) {
		if (genl_decoders[i].id == id) {
			return genl_decoders[i].decoder;
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
		initialize_genl_decoders(tcp);

		netlink_generic_decoder_t decoder =
			lookup_genl_decoder(nlmsghdr->nlmsg_type);
		decoder(tcp, &h, addr + GENL_HDRLEN, len - GENL_HDRLEN);
	}

	return true;
}
