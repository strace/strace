/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <stdint.h>
#include "test_nlattr.h"

#define NLMSGERR_ATTR_COOKIE 3

static void
init_nlmsgerr(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = msg_len,
		.nlmsg_type = NLMSG_ERROR,
		.nlmsg_flags = NLM_F_REQUEST | NLM_F_CAPPED
	);

	struct nlmsgerr *const err = NLMSG_DATA(nlh);
	SET_STRUCT(struct nlmsgerr, err,
		.error = -13,
		.msg = {
			.nlmsg_len = NLMSG_HDRLEN + 4,
			.nlmsg_type = NLMSG_NOOP,
			.nlmsg_flags = NLM_F_REQUEST,
		}
	);
}

static void
print_nlmsgerr(const unsigned int msg_len)
{
	printf("{nlmsg_len=%u, nlmsg_type=NLMSG_ERROR"
	       ", nlmsg_flags=NLM_F_REQUEST|NLM_F_CAPPED"
	       ", nlmsg_seq=0, nlmsg_pid=0}, [{error=-EACCES"
	       ", msg={nlmsg_len=%u, nlmsg_type=NLMSG_NOOP"
	       ", nlmsg_flags=NLM_F_REQUEST, nlmsg_seq=0, nlmsg_pid=0}}",
	       msg_len, NLMSG_HDRLEN + 4);
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	static const uint8_t cookie[] = { 0xab, 0xfe };

	const int fd = create_nl_socket(NETLINK_SOCK_DIAG);
	const unsigned int hdrlen = sizeof(struct nlmsgerr);
	void *const nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen),
			NLA_HDRLEN + sizeof(cookie));

	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_nlmsgerr, print_nlmsgerr,
		    NLMSGERR_ATTR_COOKIE,
		    sizeof(cookie), cookie, sizeof(cookie),
		    printf("[%u, %u]", cookie[0], cookie[1]);
		    printf("]"));

	printf("+++ exited with 0 +++\n");
	return 0;
}
