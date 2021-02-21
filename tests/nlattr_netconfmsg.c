/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include "test_nlattr.h"
#include <linux/netconf.h>
#include <linux/rtnetlink.h>

static void
init_netconfmsg(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = msg_len,
		.nlmsg_type = RTM_GETNETCONF,
		.nlmsg_flags = NLM_F_DUMP
	);

	struct netconfmsg *const msg = NLMSG_DATA(nlh);
	SET_STRUCT(struct netconfmsg, msg,
		.ncm_family = AF_INET
	);
}

static void
print_netconfmsg(const unsigned int msg_len)
{
	printf("{nlmsg_len=%u, nlmsg_type=RTM_GETNETCONF"
	       ", nlmsg_flags=NLM_F_DUMP, nlmsg_seq=0, nlmsg_pid=0}"
	       ", {ncm_family=AF_INET}",
	       msg_len);
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	const int fd = create_nl_socket(NETLINK_ROUTE);

	const unsigned int hdrlen = sizeof(struct netconfmsg);
	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen), NLA_HDRLEN + 4);

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

	const unsigned int nla_type = 0xffff & NLA_TYPE_MASK;
	char nla_type_str[256];
	sprintf(nla_type_str, "%#x /* NETCONFA_??? */", nla_type);
	TEST_NLATTR_(fd, nlh0, hdrlen,
		     init_netconfmsg, print_netconfmsg,
		     nla_type, nla_type_str,
		     4, pattern, 4,
		     print_quoted_hex(pattern, 4));

	puts("+++ exited with 0 +++");
	return 0;
}
