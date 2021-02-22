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
#include <linux/netfilter/nf_tables.h>

static void
test_nlmsg_type(const int fd)
{
	long rc;
	struct nlmsghdr nlh = {
		.nlmsg_len = sizeof(nlh),
		.nlmsg_flags = NLM_F_REQUEST,
	};

	nlh.nlmsg_type = NFNL_SUBSYS_NFTABLES << 8 | NFT_MSG_NEWTABLE;
	rc = sendto(fd, &nlh, nlh.nlmsg_len, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {nlmsg_len=%u"
	       ", nlmsg_type=NFNL_SUBSYS_NFTABLES<<8|NFT_MSG_NEWTABLE"
	       ", nlmsg_flags=NLM_F_REQUEST, nlmsg_seq=0, nlmsg_pid=0}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh.nlmsg_len, nlh.nlmsg_len, sprintrc(rc));

	nlh.nlmsg_type = NFNL_SUBSYS_NFTABLES << 8 | 0xff;
	rc = sendto(fd, &nlh, nlh.nlmsg_len, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {nlmsg_len=%u"
	       ", nlmsg_type=NFNL_SUBSYS_NFTABLES<<8|0xff /* NFT_MSG_??? */"
	       ", nlmsg_flags=NLM_F_REQUEST, nlmsg_seq=0, nlmsg_pid=0}"
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

	nlh.nlmsg_type = NFNL_SUBSYS_NFTABLES << 8 | NFT_MSG_NEWTABLE;
	nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_REPLACE;
	rc = sendto(fd, &nlh, nlh.nlmsg_len, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {nlmsg_len=%u"
	       ", nlmsg_type=NFNL_SUBSYS_NFTABLES<<8|NFT_MSG_NEWTABLE"
	       ", nlmsg_flags=NLM_F_REQUEST|NLM_F_REPLACE, nlmsg_seq=0"
	       ", nlmsg_pid=0}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh.nlmsg_len, nlh.nlmsg_len, sprintrc(rc));

	nlh.nlmsg_type = NFNL_SUBSYS_NFTABLES << 8 | NFT_MSG_GETTABLE;
	nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	rc = sendto(fd, &nlh, nlh.nlmsg_len, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {nlmsg_len=%u"
	       ", nlmsg_type=NFNL_SUBSYS_NFTABLES<<8|NFT_MSG_GETTABLE"
	       ", nlmsg_flags=NLM_F_REQUEST|NLM_F_DUMP, nlmsg_seq=0"
	       ", nlmsg_pid=0}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh.nlmsg_len, nlh.nlmsg_len, sprintrc(rc));

	nlh.nlmsg_type = NFNL_SUBSYS_NFTABLES << 8 | NFT_MSG_DELTABLE;
	nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_NONREC;
	rc = sendto(fd, &nlh, nlh.nlmsg_len, MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {nlmsg_len=%u"
	       ", nlmsg_type=NFNL_SUBSYS_NFTABLES<<8|NFT_MSG_DELTABLE"
	       ", nlmsg_flags=NLM_F_REQUEST|NLM_F_NONREC, nlmsg_seq=0"
	       ", nlmsg_pid=0}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
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
