/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_NETLINK_H
# define STRACE_NETLINK_H

# include <stdbool.h>
# include <sys/socket.h>
# include <linux/netlink.h>

# undef NLMSG_HDRLEN
# define NLMSG_HDRLEN ((unsigned int) NLMSG_ALIGN(sizeof(struct nlmsghdr)))

# undef NLA_HDRLEN
# define NLA_HDRLEN ((unsigned int) NLA_ALIGN(sizeof(struct nlattr)))

static inline bool
is_nlmsg_ok(const struct nlmsghdr *const nlh, const ssize_t len)
{
	return len >= (ssize_t) sizeof(*nlh)
	       && nlh->nlmsg_len >= sizeof(*nlh)
	       && (size_t) len >= nlh->nlmsg_len;
}

#endif /* !STRACE_NETLINK_H */
