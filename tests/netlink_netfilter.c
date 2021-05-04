/*
 * Copyright (c) 2017, 2018 Chen Jingpiao <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "test_netlink.h"
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

	nlh.nlmsg_type = NFNL_MSG_BATCH_BEGIN;
	rc = sendto(fd, &nlh, sizeof(nlh), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {nlmsg_len=%u, nlmsg_type=NFNL_MSG_BATCH_BEGIN"
	       ", nlmsg_flags=NLM_F_REQUEST, nlmsg_seq=0, nlmsg_pid=0}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh.nlmsg_len, (unsigned) sizeof(nlh), sprintrc(rc));

	nlh.nlmsg_type = NFNL_SUBSYS_CTNETLINK << 8 | 0xff;
	rc = sendto(fd, &nlh, sizeof(nlh), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {nlmsg_len=%u"
	       ", nlmsg_type=NFNL_SUBSYS_CTNETLINK<<8|0xff"
	       " /* IPCTNL_MSG_CT_??? */, nlmsg_flags=NLM_F_REQUEST"
	       ", nlmsg_seq=0, nlmsg_pid=0}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh.nlmsg_len, (unsigned) sizeof(nlh), sprintrc(rc));

	nlh.nlmsg_type = 0xffff;
	rc = sendto(fd, &nlh, sizeof(nlh), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {nlmsg_len=%u, nlmsg_type=0xff"
	       " /* NFNL_SUBSYS_??? */<<8|0xff, nlmsg_flags=NLM_F_REQUEST"
	       ", nlmsg_seq=0, nlmsg_pid=0}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh.nlmsg_len, (unsigned) sizeof(nlh), sprintrc(rc));
}

static void
test_nlmsg_done(const int fd)
{
	const int num = 0xabcdefad;
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(num));

	TEST_NETLINK(fd, nlh0, NLMSG_DONE, NLM_F_REQUEST,
		     sizeof(num), &num, sizeof(num),
		     printf("%d", num));
}

static void
test_nfgenmsg(const int fd)
{
	static const struct nlattr nla = {
		.nla_len = sizeof(nla),
		.nla_type = 0x0bcd
	};

	struct nfgenmsg msg = {
		.nfgen_family = AF_UNIX,
		.version = NFNETLINK_V0,
		.res_id = NFNL_SUBSYS_NFTABLES
	};
	char str_buf[NLMSG_ALIGN(sizeof(msg)) + 4];
	char nla_buf[NLMSG_ALIGN(sizeof(msg)) + sizeof(nla)];

	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN,
					 MAX(sizeof(str_buf), sizeof(nla_buf)));

	TEST_NETLINK_OBJECT_EX_(fd, nlh0,
				NFNL_SUBSYS_NFTABLES << 8 | NFT_MSG_NEWTABLE,
				"NFNL_SUBSYS_NFTABLES<<8|NFT_MSG_NEWTABLE",
				NLM_F_REQUEST, "NLM_F_REQUEST",
				msg, print_quoted_hex,
				printf("{nfgen_family=AF_UNIX");
				printf(", version=NFNETLINK_V0");
				printf(", res_id=");
				if (htons(NFNL_SUBSYS_NFTABLES) == NFNL_SUBSYS_NFTABLES)
					printf("htons(NFNL_SUBSYS_NFTABLES)");
				else
					printf("NFNL_SUBSYS_NFTABLES");
				printf("}");
				);

	msg.res_id = htons(NFNL_SUBSYS_NFTABLES);
	TEST_NETLINK_(fd, nlh0,
		      NFNL_SUBSYS_NFTABLES << 8 | NFT_MSG_NEWTABLE,
		      "NFNL_SUBSYS_NFTABLES<<8|NFT_MSG_NEWTABLE",
		      NLM_F_REQUEST, "NLM_F_REQUEST",
		      sizeof(msg), &msg, sizeof(msg),
		      printf("{nfgen_family=AF_UNIX");
		      printf(", version=NFNETLINK_V0");
		      printf(", res_id=htons(NFNL_SUBSYS_NFTABLES)}"));

	msg.res_id = htons(0xabcd);
	TEST_NETLINK_(fd, nlh0,
		      NFNL_SUBSYS_NFTABLES << 8 | NFT_MSG_NEWTABLE,
		      "NFNL_SUBSYS_NFTABLES<<8|NFT_MSG_NEWTABLE",
		      NLM_F_REQUEST, "NLM_F_REQUEST",
		      sizeof(msg), &msg, sizeof(msg),
		      printf("{nfgen_family=AF_UNIX");
		      printf(", version=NFNETLINK_V0");
		      printf(", res_id=htons(%d)}", 0xabcd));

	msg.res_id = htons(NFNL_SUBSYS_NFTABLES);
	TEST_NETLINK(fd, nlh0,
		     NFNL_MSG_BATCH_BEGIN, NLM_F_REQUEST,
		     sizeof(msg), &msg, sizeof(msg),
		     printf("{nfgen_family=AF_UNIX");
		     printf(", version=NFNETLINK_V0");
		     printf(", res_id=htons(%d)}", NFNL_SUBSYS_NFTABLES));

	msg.res_id = htons(0xabcd);
	memcpy(str_buf, &msg, sizeof(msg));
	memcpy(str_buf + NLMSG_ALIGN(sizeof(msg)), "1234", 4);

	TEST_NETLINK(fd, nlh0,
		     NFNL_MSG_BATCH_BEGIN, NLM_F_REQUEST,
		     sizeof(str_buf), str_buf, sizeof(str_buf),
		     printf("{nfgen_family=AF_UNIX");
		     printf(", version=NFNETLINK_V0");
		     printf(", res_id=htons(%d)}"
			    ", \"\\x31\\x32\\x33\\x34\"", 0xabcd));

	msg.res_id = htons(NFNL_SUBSYS_NFTABLES);
	memcpy(nla_buf, &msg, sizeof(msg));
	memcpy(nla_buf + NLMSG_ALIGN(sizeof(msg)), &nla, sizeof(nla));

	TEST_NETLINK_(fd, nlh0,
		      NFNL_SUBSYS_NFTABLES << 8 | 0xff,
		      "NFNL_SUBSYS_NFTABLES<<8|0xff /* NFT_MSG_??? */",
		      NLM_F_REQUEST, "NLM_F_REQUEST",
		      sizeof(nla_buf), nla_buf, sizeof(nla_buf),
		      printf("{nfgen_family=AF_UNIX");
		      printf(", version=NFNETLINK_V0");
		      printf(", res_id=htons(NFNL_SUBSYS_NFTABLES)}"
			     ", {nla_len=%d, nla_type=%#x}",
			     nla.nla_len, nla.nla_type));
}

int main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	int fd = create_nl_socket(NETLINK_NETFILTER);

	test_nlmsg_type(fd);
	test_nlmsg_done(fd);
	test_nfgenmsg(fd);

	printf("+++ exited with 0 +++\n");

	return 0;
}
