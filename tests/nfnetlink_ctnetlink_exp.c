/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#ifdef HAVE_LINUX_NETFILTER_NFNETLINK_CONNTRACK_H

# include <stdio.h>
# include <sys/socket.h>
# include "netlink.h"
# include <linux/netfilter/nfnetlink.h>
# include <linux/netfilter/nfnetlink_conntrack.h>

static void
test_nlmsg_type(const int fd)
{
	long rc;
	struct nlmsghdr nlh = {
		.nlmsg_len = sizeof(nlh),
		.nlmsg_flags = NLM_F_REQUEST,
	};

	nlh.nlmsg_type = NFNL_SUBSYS_CTNETLINK_EXP << 8 | IPCTNL_MSG_EXP_NEW;
	rc = sendto(fd, &nlh, nlh.nlmsg_len, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {len=%u"
	       ", type=NFNL_SUBSYS_CTNETLINK_EXP<<8|IPCTNL_MSG_EXP_NEW"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh.nlmsg_len, nlh.nlmsg_len, sprintrc(rc));

	nlh.nlmsg_type = NFNL_SUBSYS_CTNETLINK_EXP << 8 | 0xff;
	rc = sendto(fd, &nlh, nlh.nlmsg_len, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {len=%u"
	       ", type=NFNL_SUBSYS_CTNETLINK_EXP<<8|0xff /* IPCTNL_MSG_EXP_??? */"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh.nlmsg_len, nlh.nlmsg_len, sprintrc(rc));
}

static void
test_nlmsg_flags(const int fd)
{
	long rc;
	struct nlmsghdr nlh = {
		.nlmsg_len = sizeof(nlh),
	};

	nlh.nlmsg_type = NFNL_SUBSYS_CTNETLINK_EXP << 8 | IPCTNL_MSG_EXP_NEW;
	nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_EXCL;
	rc = sendto(fd, &nlh, nlh.nlmsg_len, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {len=%u"
	       ", type=NFNL_SUBSYS_CTNETLINK_EXP<<8|IPCTNL_MSG_EXP_NEW"
	       ", flags=NLM_F_REQUEST|NLM_F_EXCL, seq=0, pid=0}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh.nlmsg_len, nlh.nlmsg_len, sprintrc(rc));

	nlh.nlmsg_type = NFNL_SUBSYS_CTNETLINK_EXP << 8 | IPCTNL_MSG_EXP_GET;
	nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_ROOT;
	rc = sendto(fd, &nlh, nlh.nlmsg_len, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {len=%u"
	       ", type=NFNL_SUBSYS_CTNETLINK_EXP<<8|IPCTNL_MSG_EXP_GET"
	       ", flags=NLM_F_REQUEST|NLM_F_ROOT, seq=0, pid=0}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh.nlmsg_len, nlh.nlmsg_len, sprintrc(rc));

	nlh.nlmsg_type = NFNL_SUBSYS_CTNETLINK_EXP << 8 | IPCTNL_MSG_EXP_DELETE;
	nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_NONREC;
	rc = sendto(fd, &nlh, nlh.nlmsg_len, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {len=%u"
	       ", type=NFNL_SUBSYS_CTNETLINK_EXP<<8|IPCTNL_MSG_EXP_DELETE"
	       ", flags=NLM_F_REQUEST|NLM_F_NONREC, seq=0, pid=0}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh.nlmsg_len, nlh.nlmsg_len, sprintrc(rc));
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	int fd = create_nl_socket(NETLINK_NETFILTER);

	test_nlmsg_type(fd);
	test_nlmsg_flags(fd);

	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_LINUX_NETFILTER_NFNETLINK_CONNTRACK_H")

#endif
