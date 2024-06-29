/*
 * Copyright (c) 2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_NETLINK_GENERIC_H
# define STRACE_NETLINK_GENERIC_H

# define DECL_NETLINK_GENERIC_DECODER(genl_decode_name)	\
void							\
genl_decode_name(struct tcb *tcp,			\
		 const struct genlmsghdr *genl,		\
		 kernel_ulong_t addr,			\
		 unsigned int len)			\
/* End of DECL_NETLINK_GENERIC_DECODER definition. */

extern DECL_NETLINK_GENERIC_DECODER(decode_nlctrl_msg);

#endif /* !STRACE_NETLINK_GENERIC_H */
