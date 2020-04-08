/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#ifdef HAVE_STRUCT_GNET_STATS_BASIC

# include <stdio.h>
# include <stddef.h>
# include "test_nlattr.h"
# include <linux/gen_stats.h>
# include <linux/rtnetlink.h>

# if !HAVE_DECL_TCA_STATS_PKT64
enum { TCA_STATS_PKT64 = 8 };
# endif

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
	printf("{len=%u, type=RTM_GETQDISC, flags=NLM_F_DUMP"
	       ", seq=0, pid=0}, {tcm_family=AF_UNIX"
	       ", tcm_ifindex=" IFINDEX_LO_STR
	       ", tcm_handle=0, tcm_parent=0, tcm_info=0}"
	       ", {{nla_len=%u, nla_type=TCA_STATS2}",
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
				  PRINT_FIELD_U("{", sb, bytes);
				  PRINT_FIELD_U(", ", sb, packets);
				  printf("}"));

# ifdef HAVE_STRUCT_GNET_STATS_RATE_EST
	static const struct gnet_stats_rate_est est = {
		.bps = 0xebcdaebd,
		.pps = 0xabdceade,
	};
	TEST_NESTED_NLATTR_OBJECT(fd, nlh0, hdrlen,
				  init_tcmsg, print_tcmsg,
				  TCA_STATS_RATE_EST, pattern, est,
				  PRINT_FIELD_U("{", est, bps);
				  PRINT_FIELD_U(", ", est, pps);
				  printf("}"));
# endif

# ifdef HAVE_STRUCT_GNET_STATS_QUEUE
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
				  PRINT_FIELD_U("{", qstats, qlen);
				  PRINT_FIELD_U(", ", qstats, backlog);
				  PRINT_FIELD_U(", ", qstats, drops);
				  PRINT_FIELD_U(", ", qstats, requeues);
				  PRINT_FIELD_U(", ", qstats, overlimits);
				  printf("}"));
# endif

# ifdef HAVE_STRUCT_GNET_STATS_RATE_EST64
	static const struct gnet_stats_rate_est64 est64 = {
		.bps = 0xacbdcdefafecaebf,
		.pps = 0xcdabeabdfeabceaf
	};
	TEST_NESTED_NLATTR_OBJECT(fd, nlh0, hdrlen,
				  init_tcmsg, print_tcmsg,
				  TCA_STATS_RATE_EST64, pattern, est64,
				  PRINT_FIELD_U("{", est64, bps);
				  PRINT_FIELD_U(", ", est64, pps);
				  printf("}"));
# endif

	static const uint64_t pkt64 = 0xdeadc0defacefeedULL;
	TEST_NESTED_NLATTR_OBJECT(fd, nlh0, hdrlen,
				  init_tcmsg, print_tcmsg,
				  TCA_STATS_PKT64, pattern, pkt64,
				  printf("16045693111314087661"));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_STRUCT_GNET_STATS_BASIC")

#endif
