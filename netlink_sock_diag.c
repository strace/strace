/*
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "netlink.h"
#include "netlink_sock_diag.h"

#define XLAT_MACROS_ONLY
#include "xlat/addrfams.h"
#undef XLAT_MACROS_ONLY

static void
decode_family(struct tcb *const tcp, const uint8_t family,
	      const kernel_ulong_t addr, const unsigned int len)
{
	tprints("{family=");
	printxval(addrfams, family, "AF_???");
	if (len > sizeof(family)) {
		tprints(", ");
		printstr_ex(tcp, addr + sizeof(family),
			    len - sizeof(family), QUOTE_FORCE_HEX);
	}
	tprints("}");
}

typedef DECL_NETLINK_DIAG_DECODER((*netlink_diag_decoder_t));

static const struct {
	const netlink_diag_decoder_t request, response;
} diag_decoders[] = {
	[AF_UNIX] = { decode_unix_diag_req, decode_unix_diag_msg },
	[AF_INET] = { decode_inet_diag_req, decode_inet_diag_msg },
	[AF_INET6] = { decode_inet_diag_req, decode_inet_diag_msg },
	[AF_NETLINK] = { decode_netlink_diag_req, decode_netlink_diag_msg },
	[AF_PACKET] = { decode_packet_diag_req, decode_packet_diag_msg },
	[AF_SMC] = { decode_smc_diag_req, decode_smc_diag_msg },
};

bool
decode_netlink_sock_diag(struct tcb *const tcp,
			 const struct nlmsghdr *const nlmsghdr,
			 const kernel_ulong_t addr,
			 const unsigned int len)
{
	uint8_t family;

	if (nlmsghdr->nlmsg_type == NLMSG_DONE)
		return false;

	if (!umove_or_printaddr(tcp, addr, &family)) {
		if (family < ARRAY_SIZE(diag_decoders)
		    && len > sizeof(family)) {
			const netlink_diag_decoder_t decoder =
				(nlmsghdr->nlmsg_flags & NLM_F_REQUEST)
				? diag_decoders[family].request
				: diag_decoders[family].response;

			if (decoder) {
				decoder(tcp, nlmsghdr, family, addr, len);
				return true;
			}
		}

		decode_family(tcp, family, addr, len);
	}

	return true;
}
