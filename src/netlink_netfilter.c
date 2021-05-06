/*
 * Copyright (c) 2018 Chen Jingpiao <chenjingpiao@gmail.com>
 * Copyright (c) 2018-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "nlattr.h"

#include <netinet/in.h>
#include <arpa/inet.h>
#include "netlink.h"
#include <linux/netfilter/nfnetlink.h>

#include "xlat/netfilter_versions.h"
#include "xlat/nl_netfilter_msg_types.h"
#include "xlat/nl_netfilter_subsys_ids.h"

bool
decode_netlink_netfilter(struct tcb *const tcp,
			 const struct nlmsghdr *const nlmsghdr,
			 const kernel_ulong_t addr,
			 const unsigned int len)
{
	if (nlmsghdr->nlmsg_type == NLMSG_DONE)
		return false;

	struct nfgenmsg nfmsg;

	if (len < sizeof(nfmsg))
		printstr_ex(tcp, addr, len, QUOTE_FORCE_HEX);
	else if (!umove_or_printaddr(tcp, addr, &nfmsg)) {
		const uint8_t subsys_id = (uint8_t) (nlmsghdr->nlmsg_type >> 8);
		uint16_t res_id = ntohs(nfmsg.res_id);

		tprint_struct_begin();
		PRINT_FIELD_XVAL(nfmsg, nfgen_family, addrfams, "AF_???");
		tprint_struct_next();
		PRINT_FIELD_XVAL(nfmsg, version, netfilter_versions,
				 "NFNETLINK_???");

		/*
		 * Work around wrong endianness in res_id field,
		 * see linux commit v4.3-rc1~28^2~47^2~1
		 */
		tprint_struct_next();
		tprints_field_name("res_id");
		if (subsys_id == NFNL_SUBSYS_NFTABLES
		    && res_id == NFNL_SUBSYS_NFTABLES) {
			print_xlat_ex(nfmsg.res_id,
				      "htons(NFNL_SUBSYS_NFTABLES)",
				      XLAT_STYLE_DEFAULT);
		} else if (subsys_id == NFNL_SUBSYS_NFTABLES
			   && nfmsg.res_id == NFNL_SUBSYS_NFTABLES) {
			print_xlat_ex(nfmsg.res_id, "NFNL_SUBSYS_NFTABLES",
				      XLAT_STYLE_DEFAULT);
		} else {
			tprints_arg_begin("htons");
			PRINT_VAL_U(res_id);
			tprint_arg_end();
		}
		tprint_struct_end();

		const size_t offset = NLMSG_ALIGN(sizeof(nfmsg));
		if (len > offset) {
			tprint_array_next();
			if ((nlmsghdr->nlmsg_type >= NFNL_MSG_BATCH_BEGIN
			     && nlmsghdr->nlmsg_type <= NFNL_MSG_BATCH_END)
			    || nlmsghdr->nlmsg_type < NLMSG_MIN_TYPE)
				printstr_ex(tcp, addr + offset,
					    len - offset, QUOTE_FORCE_HEX);
			else
				decode_nlattr(tcp, addr + offset, len - offset,
					      NULL, NULL, NULL, 0, NULL);
		}
	}

	return true;
}
