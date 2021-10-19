/*
 * netlink attribute ifinfomsg common code.
 *
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef STRACE_TESTS_NLATTR_IFLA_H
# define STRACE_TESTS_NLATTR_IFLA_H

# include "tests.h"

# ifndef IFLA_ATTR
#  error "Please define IFLA_ATTR before including this file"
# endif

# ifndef IFLA_AF
#  define IFLA_AF AF_UNIX
# endif
# ifndef IFLA_AF_STR
#  define IFLA_AF_STR "AF_UNIX"
# endif

static const unsigned int hdrlen = sizeof(struct ifinfomsg);

static void
init_ifinfomsg(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = msg_len,
		.nlmsg_type = RTM_GETLINK,
		.nlmsg_flags = NLM_F_DUMP
	);

	struct ifinfomsg *const msg = NLMSG_DATA(nlh);
	SET_STRUCT(struct ifinfomsg, msg,
		.ifi_family = IFLA_AF,
		.ifi_type = ARPHRD_LOOPBACK,
		.ifi_index = ifindex_lo(),
		.ifi_flags = IFF_UP,
	);

	struct nlattr *const nla = NLMSG_ATTR(nlh, sizeof(*msg));
	SET_STRUCT(struct nlattr, nla,
		.nla_len = msg_len - NLMSG_SPACE(hdrlen),
		.nla_type = IFLA_ATTR
	);
}

static void
print_ifinfomsg(const unsigned int msg_len)
{
	printf("{nlmsg_len=%u, nlmsg_type=" XLAT_FMT ", nlmsg_flags=" XLAT_FMT
	       ", nlmsg_seq=0, nlmsg_pid=0}, {ifi_family=" XLAT_FMT
	       ", ifi_type=" XLAT_FMT ", ifi_index=" XLAT_FMT_U
	       ", ifi_flags=" XLAT_FMT ", ifi_change=0}"
	       ", [{nla_len=%u, nla_type=" XLAT_FMT "}",
	       msg_len, XLAT_ARGS(RTM_GETLINK), XLAT_ARGS(NLM_F_DUMP),
	       XLAT_SEL(IFLA_AF, IFLA_AF_STR), XLAT_ARGS(ARPHRD_LOOPBACK),
	       XLAT_SEL(ifindex_lo(), IFINDEX_LO_STR), XLAT_ARGS(IFF_UP),
	       msg_len - NLMSG_SPACE(hdrlen),
	       XLAT_SEL(IFLA_ATTR, STRINGIFY_VAL(IFLA_ATTR)));
}

#endif /* STRACE_TESTS_NLATTR_IFLA_H */
