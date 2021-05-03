/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <stddef.h>
#include "test_nlattr.h"
#include <linux/gen_stats.h>
#include <linux/rtnetlink.h>

const unsigned int hdrlen = sizeof(struct tcmsg);

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

	struct nlattr *const nla = NLMSG_ATTR(nlh, sizeof(*msg));
	SET_STRUCT(struct nlattr, nla,
		.nla_len = msg_len - NLMSG_SPACE(hdrlen),
		.nla_type = TCA_STATS2
	);
}

static void
print_tcmsg(const unsigned int msg_len)
{
	printf("{nlmsg_len=%u, nlmsg_type=RTM_GETQDISC, nlmsg_flags=NLM_F_DUMP"
	       ", nlmsg_seq=0, nlmsg_pid=0}, {tcm_family=AF_UNIX"
	       ", tcm_ifindex=" IFINDEX_LO_STR
	       ", tcm_handle=0, tcm_parent=0, tcm_info=0}"
	       ", [{nla_len=%u, nla_type=TCA_STATS2}",
	       msg_len, msg_len - NLMSG_SPACE(hdrlen));
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	const int fd = create_nl_socket(NETLINK_ROUTE);
	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen), NLA_HDRLEN + 8 * 5);

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

	static const struct gnet_stats_basic sb = {
		.bytes = 0xabcdebdafefeadeb,
		.packets = 0xbdcdeabf
	};
	char buf[offsetofend(struct gnet_stats_basic, packets)];
	memcpy(buf, &sb, sizeof(buf));
	TEST_NESTED_NLATTR_OBJECT(fd, nlh0, hdrlen,
				  init_tcmsg, print_tcmsg,
				  TCA_STATS_BASIC, pattern, buf,
				  printf("{");
				  PRINT_FIELD_U(sb, bytes);
				  printf(", ");
				  PRINT_FIELD_U(sb, packets);
				  printf("}"));

	static const struct gnet_stats_rate_est est = {
		.bps = 0xebcdaebd,
		.pps = 0xabdceade,
	};
	TEST_NESTED_NLATTR_OBJECT(fd, nlh0, hdrlen,
				  init_tcmsg, print_tcmsg,
				  TCA_STATS_RATE_EST, pattern, est,
				  printf("{");
				  PRINT_FIELD_U(est, bps);
				  printf(", ");
				  PRINT_FIELD_U(est, pps);
				  printf("}"));

	static const struct gnet_stats_queue qstats = {
		.qlen = 0xabcdeabd,
		.backlog = 0xbcdaebad,
		.drops = 0xcdbeaedb,
		.requeues = 0xdebaefab,
		.overlimits = 0xefaebade
	};
	TEST_NESTED_NLATTR_OBJECT(fd, nlh0, hdrlen,
				  init_tcmsg, print_tcmsg,
				  TCA_STATS_QUEUE, pattern, qstats,
				  printf("{");
				  PRINT_FIELD_U(qstats, qlen);
				  printf(", ");
				  PRINT_FIELD_U(qstats, backlog);
				  printf(", ");
				  PRINT_FIELD_U(qstats, drops);
				  printf(", ");
				  PRINT_FIELD_U(qstats, requeues);
				  printf(", ");
				  PRINT_FIELD_U(qstats, overlimits);
				  printf("}"));

	static const struct gnet_stats_rate_est64 est64 = {
		.bps = 0xacbdcdefafecaebf,
		.pps = 0xcdabeabdfeabceaf
	};
	TEST_NESTED_NLATTR_OBJECT(fd, nlh0, hdrlen,
				  init_tcmsg, print_tcmsg,
				  TCA_STATS_RATE_EST64, pattern, est64,
				  printf("{");
				  PRINT_FIELD_U(est64, bps);
				  printf(", ");
				  PRINT_FIELD_U(est64, pps);
				  printf("}"));

	static const uint64_t pkt64 = 0xdeadc0defacefeedULL;
	TEST_NESTED_NLATTR_OBJECT(fd, nlh0, hdrlen,
				  init_tcmsg, print_tcmsg,
				  TCA_STATS_PKT64, pattern, pkt64,
				  printf("16045693111314087661"));

	puts("+++ exited with 0 +++");
	return 0;
}
