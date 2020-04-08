/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_NETLINK_H
# define STRACE_NETLINK_H

# include <stdbool.h>
# include <sys/socket.h>
# include <linux/netlink.h>

# ifndef NETLINK_SOCK_DIAG
#  define NETLINK_SOCK_DIAG 4
# endif

# ifndef NLM_F_NONREC
#  define NLM_F_NONREC	0x100
# endif
# ifndef NLM_F_CAPPED
#  define NLM_F_CAPPED	0x100
# endif

# undef NLMSG_HDRLEN
# define NLMSG_HDRLEN ((unsigned int) NLMSG_ALIGN(sizeof(struct nlmsghdr)))

# ifndef NLMSG_MIN_TYPE
#  define NLMSG_MIN_TYPE		0x10
# endif

# ifndef NLA_ALIGN
#  define NLA_ALIGN(len) (((len) + 3) & ~3)
# endif

# undef NLA_HDRLEN
# define NLA_HDRLEN ((unsigned int) NLA_ALIGN(sizeof(struct nlattr)))

# ifndef NLA_TYPE_MASK
#  define NLA_F_NESTED		(1 << 15)
#  define NLA_F_NET_BYTEORDER	(1 << 14)
#  define NLA_TYPE_MASK		~(NLA_F_NESTED | NLA_F_NET_BYTEORDER)
# endif

static inline bool
is_nlmsg_ok(const struct nlmsghdr *const nlh, const ssize_t len)
{
	return len >= (ssize_t) sizeof(*nlh)
	       && nlh->nlmsg_len >= sizeof(*nlh)
	       && (size_t) len >= nlh->nlmsg_len;
}

#endif /* !STRACE_NETLINK_H */
