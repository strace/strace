/*
 * Copyright (c) 2017-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_NETLINK_GENERIC_H
# define STRACE_NETLINK_GENERIC_H

# include "netlink.h"
# include <linux/genetlink.h>

# define DECL_NETLINK_GENERIC_DECODER(genl_decode_name)	\
void							\
genl_decode_name(struct tcb *tcp,			\
		 const struct genlmsghdr *hdr,		\
		 kernel_ulong_t addr,			\
		 unsigned int len)			\
/* End of DECL_NETLINK_GENERIC_DECODER definition. */

extern DECL_NETLINK_GENERIC_DECODER(decode_nlctrl);

#endif /* !STRACE_NETLINK_GENERIC_H */
