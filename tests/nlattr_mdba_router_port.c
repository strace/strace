/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include "test_nlattr.h"
#include <linux/if_bridge.h>
#include <linux/rtnetlink.h>

const unsigned int hdrlen = sizeof(struct br_port_msg);

static void
init_br_port_msg(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = msg_len,
		.nlmsg_type = RTM_GETMDB,
		.nlmsg_flags = NLM_F_DUMP
	);

	struct br_port_msg *const msg = NLMSG_DATA(nlh);
	SET_STRUCT(struct br_port_msg, msg,
		.family = AF_UNIX,
		.ifindex = ifindex_lo()
	);

	struct nlattr *nla = NLMSG_ATTR(nlh, sizeof(*msg));
	SET_STRUCT(struct nlattr, nla,
		.nla_len = msg_len - NLMSG_SPACE(hdrlen),
		.nla_type = MDBA_ROUTER
	);
}

static void
print_br_port_msg(const unsigned int msg_len)
{
	printf("{nlmsg_len=%u, nlmsg_type=RTM_GETMDB, nlmsg_flags=NLM_F_DUMP"
	       ", nlmsg_seq=0, nlmsg_pid=0}, {family=AF_UNIX"
	       ", ifindex=" IFINDEX_LO_STR "}"
	       ", [{nla_len=%u, nla_type=MDBA_ROUTER}",
	       msg_len, msg_len - NLMSG_SPACE(hdrlen));
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	const uint32_t ifindex = ifindex_lo();
	const uint8_t type = MDB_RTR_TYPE_DISABLED;
	static const struct nlattr nla = {
		.nla_len = NLA_HDRLEN + sizeof(type),
		.nla_type = MDBA_ROUTER_PATTR_TYPE
	};
	char buf[NLMSG_ALIGN(ifindex) + NLA_HDRLEN + sizeof(type)];

	const int fd = create_nl_socket(NETLINK_ROUTE);

	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen),
				   NLA_HDRLEN + sizeof(buf));

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

	TEST_NESTED_NLATTR_OBJECT(fd, nlh0, hdrlen,
				  init_br_port_msg, print_br_port_msg,
				  MDBA_ROUTER_PORT, pattern, ifindex,
				  printf(IFINDEX_LO_STR));

	memcpy(buf, &ifindex, sizeof(ifindex));
	memcpy(buf + NLMSG_ALIGN(ifindex), &nla, sizeof(nla));
	memcpy(buf + NLMSG_ALIGN(ifindex) + NLA_HDRLEN, &type, sizeof(type));
	TEST_NLATTR(fd, nlh0 - NLA_HDRLEN, hdrlen + NLA_HDRLEN,
		    init_br_port_msg, print_br_port_msg,
		    MDBA_ROUTER_PORT, sizeof(buf), buf, sizeof(buf),
		    printf(IFINDEX_LO_STR
			   ", [{nla_len=%u, nla_type=MDBA_ROUTER_PATTR_TYPE}"
			   ", MDB_RTR_TYPE_DISABLED]]",
			   nla.nla_len));

	/* timers */
	static const struct strval32 pattrs[] = {
		{ ARG_STR(MDBA_ROUTER_PATTR_TIMER) },
		{ ARG_STR(MDBA_ROUTER_PATTR_INET_TIMER) },
		{ ARG_STR(MDBA_ROUTER_PATTR_INET6_TIMER) },
	};
	uint32_t timer = 0xdead;
	long clk_tck;
	int precision = 0;

	clk_tck = sysconf(_SC_CLK_TCK);
	if (clk_tck > 0) {
		precision = clk_tck > 1 ? MIN((int) ceil(log10(clk_tck - 1)), 9)
					: 0;
		timer *= clk_tck;
	}

	struct nlattr nla_timer = {
		.nla_len = NLA_HDRLEN + sizeof(timer),
	};
	char buf_timer[NLMSG_ALIGN(ifindex) + NLA_HDRLEN + sizeof(timer)];

	memcpy(buf_timer, &ifindex, sizeof(ifindex));
	memcpy(buf_timer + NLMSG_ALIGN(ifindex) + NLA_HDRLEN,
	       &timer, sizeof(timer));

	for (size_t i = 0; i < ARRAY_SIZE(pattrs); i++) {
		nla_timer.nla_type = pattrs[i].val;
		memcpy(buf_timer + NLMSG_ALIGN(ifindex),
		       &nla_timer, sizeof(nla_timer));

		TEST_NLATTR(fd, nlh0 - NLA_HDRLEN, hdrlen + NLA_HDRLEN,
			    init_br_port_msg, print_br_port_msg,
			    MDBA_ROUTER_PORT,
			    sizeof(buf_timer), buf_timer, sizeof(buf_timer),
			    printf(IFINDEX_LO_STR
				   ", [{nla_len=%u, nla_type=%s}, %u",
				   nla_timer.nla_len, pattrs[i].str, timer);
			    if (clk_tck > 0)
				    printf(" /* 57005.%0*u s */", precision, 0);
			    printf("]]"));
	}

	puts("+++ exited with 0 +++");
	return 0;
}
