/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <sys/socket.h>
#include "netlink.h"
#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nfnetlink_osf.h>

static void
test_nlmsg_type(const int fd)
{
	long rc;
	struct nlmsghdr nlh = {
		.nlmsg_len = sizeof(nlh),
		.nlmsg_flags = NLM_F_REQUEST,
	};

	nlh.nlmsg_type = NFNL_SUBSYS_OSF << 8 | OSF_MSG_ADD;
	rc = sendto(fd, &nlh, nlh.nlmsg_len, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {nlmsg_len=%u"
	       ", nlmsg_type=NFNL_SUBSYS_OSF<<8|OSF_MSG_ADD"
	       ", nlmsg_flags=NLM_F_REQUEST, nlmsg_seq=0, nlmsg_pid=0}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh.nlmsg_len, nlh.nlmsg_len, sprintrc(rc));

	nlh.nlmsg_type = NFNL_SUBSYS_OSF << 8 | 0xff;
	rc = sendto(fd, &nlh, nlh.nlmsg_len, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {nlmsg_len=%u"
	       ", nlmsg_type=NFNL_SUBSYS_OSF<<8|0xff /* OSF_MSG_??? */"
	       ", nlmsg_flags=NLM_F_REQUEST, nlmsg_seq=0, nlmsg_pid=0}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh.nlmsg_len, nlh.nlmsg_len, sprintrc(rc));
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	int fd = create_nl_socket(NETLINK_NETFILTER);

	test_nlmsg_type(fd);

	puts("+++ exited with 0 +++");

	return 0;
}
