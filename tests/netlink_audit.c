/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/audit.h>
#include "netlink.h"

static void
test_nlmsg_type(const int fd)
{
	long rc;
	struct nlmsghdr nlh = {
		.nlmsg_len = sizeof(nlh),
		.nlmsg_type = AUDIT_GET,
		.nlmsg_flags = NLM_F_REQUEST,
	};

	rc = sendto(fd, &nlh, sizeof(nlh), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {len=%u, type=AUDIT_GET"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh.nlmsg_len, (unsigned) sizeof(nlh), sprintrc(rc));
}

int main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	int fd = create_nl_socket(NETLINK_AUDIT);

	test_nlmsg_type(fd);

	printf("+++ exited with 0 +++\n");

	return 0;
}
