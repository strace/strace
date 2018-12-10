/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include "test_nlattr.h"
#include <linux/rtnetlink.h>

static void
init_tcamsg(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = msg_len,
		.nlmsg_type = RTM_GETACTION,
		.nlmsg_flags = NLM_F_DUMP
	);

	struct tcamsg *const msg = NLMSG_DATA(nlh);
	SET_STRUCT(struct tcamsg, msg,
		.tca_family = AF_INET
	);
}

static void
print_tcamsg(const unsigned int msg_len)
{
	printf("{len=%u, type=RTM_GETACTION, flags=NLM_F_DUMP"
	       ", seq=0, pid=0}, {tca_family=AF_INET}",
	       msg_len);
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	const int fd = create_nl_socket(NETLINK_ROUTE);
	const unsigned int hdrlen = sizeof(struct tcamsg);
	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen), NLA_HDRLEN + 4);

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

	const unsigned int nla_type = 0xffff & NLA_TYPE_MASK;
	char nla_type_str[256];
	sprintf(nla_type_str, "%#x /* TCA_ACT_??? */", nla_type);
	TEST_NLATTR_(fd, nlh0, hdrlen,
		     init_tcamsg, print_tcamsg,
		     nla_type, nla_type_str,
		     4, pattern, 4,
		     print_quoted_hex(pattern, 4));

	puts("+++ exited with 0 +++");
	return 0;
}
