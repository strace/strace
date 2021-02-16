/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "netlink.h"

#include <linux/selinux_netlink.h>

bool
decode_netlink_selinux(struct tcb *const tcp,
		       const struct nlmsghdr *const nlmsghdr,
		       const kernel_ulong_t addr,
		       const unsigned int len)
{
	switch (nlmsghdr->nlmsg_type) {
	case SELNL_MSG_SETENFORCE: {
		struct selnl_msg_setenforce msg;

		if (len < sizeof(msg))
			printstr_ex(tcp, addr, len, QUOTE_FORCE_HEX);
		else if (!umove_or_printaddr(tcp, addr, &msg)) {
			tprint_struct_begin();
			PRINT_FIELD_D(msg, val);
			tprint_struct_end();
		}
		break;
	}
	case SELNL_MSG_POLICYLOAD: {
		struct selnl_msg_policyload msg;

		if (len < sizeof(msg))
			printstr_ex(tcp, addr, len, QUOTE_FORCE_HEX);
		else if (!umove_or_printaddr(tcp, addr, &msg)) {
			tprint_struct_begin();
			PRINT_FIELD_U(msg, seqno);
			tprint_struct_end();
		}
		break;
	}
	default:
		return false;
	}

	return true;
}
