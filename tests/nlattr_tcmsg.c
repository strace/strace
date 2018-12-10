/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include <stddef.h>
#include "test_nlattr.h"
#include <linux/pkt_sched.h>
#include <linux/rtnetlink.h>

static void
init_tcmsg(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = msg_len,
		.nlmsg_type = RTM_GETQDISC,
		.nlmsg_flags = NLM_F_DUMP
	);

	struct tcmsg *const msg = NLMSG_DATA(nlh);
	SET_STRUCT(struct tcmsg, msg,
		.tcm_family = AF_UNIX,
		.tcm_ifindex = ifindex_lo()
	);

}

static void
print_tcmsg(const unsigned int msg_len)
{
	printf("{len=%u, type=RTM_GETQDISC, flags=NLM_F_DUMP"
	       ", seq=0, pid=0}, {tcm_family=AF_UNIX"
	       ", tcm_ifindex=" IFINDEX_LO_STR
	       ", tcm_handle=0, tcm_parent=0, tcm_info=0}",
	       msg_len);
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	const int fd = create_nl_socket(NETLINK_ROUTE);
	const unsigned int hdrlen = sizeof(struct tcmsg);
	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen),
				   NLA_HDRLEN + sizeof(struct tc_stats));

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

	const unsigned int nla_type = 0xffff & NLA_TYPE_MASK;
	char nla_type_str[256];
	sprintf(nla_type_str, "%#x /* TCA_??? */", nla_type);
	TEST_NLATTR_(fd, nlh0, hdrlen,
		     init_tcmsg, print_tcmsg,
		     nla_type, nla_type_str,
		     4, pattern, 4,
		     print_quoted_hex(pattern, 4));

	static const struct tc_stats st = {
		.bytes = 0xabcdcdbefeadefac,
		.packets = 0xbcdeaefd,
		.drops = 0xcdedafed,
		.overlimits = 0xdcdbefad,
		.bps = 0xefaebfad,
		.pps = 0xfefbaedb,
		.qlen = 0xabcdefab,
		.backlog = 0xbdeabeab
	};
	char buf[offsetofend(struct tc_stats, backlog)];
	memcpy(buf, &st, sizeof(buf));
	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_tcmsg, print_tcmsg,
			   TCA_STATS, pattern, buf,
			   PRINT_FIELD_U("{", st, bytes);
			   PRINT_FIELD_U(", ", st, packets);
			   PRINT_FIELD_U(", ", st, drops);
			   PRINT_FIELD_U(", ", st, overlimits);
			   PRINT_FIELD_U(", ", st, bps);
			   PRINT_FIELD_U(", ", st, pps);
			   PRINT_FIELD_U(", ", st, qlen);
			   PRINT_FIELD_U(", ", st, backlog);
			   printf("}"));

	static const struct tc_estimator est = {
		.interval = 0xcd,
		.ewma_log = 0xab
	};
	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_tcmsg, print_tcmsg,
			   TCA_RATE, pattern, est,
			   PRINT_FIELD_D("{", est, interval);
			   PRINT_FIELD_U(", ", est, ewma_log);
			   printf("}"));

	puts("+++ exited with 0 +++");
	return 0;
}
