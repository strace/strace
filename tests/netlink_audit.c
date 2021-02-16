/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "pidns.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/audit.h>
#include "netlink.h"

static void
test_nlmsg_type(const int fd)
{
	PIDNS_TEST_INIT;

	long rc;
	struct nlmsghdr nlh = {
		.nlmsg_len = sizeof(nlh),
		.nlmsg_type = AUDIT_GET,
		.nlmsg_flags = NLM_F_REQUEST,
		.nlmsg_pid = getpid(),
	};

	rc = sendto(fd, &nlh, sizeof(nlh), MSG_DONTWAIT, NULL, 0);
	pidns_print_leader();
	printf("sendto(%d, {nlmsg_len=%u, nlmsg_type=AUDIT_GET"
	       ", nlmsg_flags=NLM_F_REQUEST, nlmsg_seq=0, nlmsg_pid=%d%s}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh.nlmsg_len, nlh.nlmsg_pid, pidns_pid2str(PT_TGID),
	       (unsigned) sizeof(nlh), sprintrc(rc));
}

int main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	int fd = create_nl_socket(NETLINK_AUDIT);

	test_nlmsg_type(fd);

	pidns_print_leader();
	printf("+++ exited with 0 +++\n");

	return 0;
}
