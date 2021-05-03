/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include "test_nlattr.h"
#include <linux/pkt_sched.h>
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
		.nla_type = TCA_STAB
	);
}

static void
print_tcmsg(const unsigned int msg_len)
{
	printf("{nlmsg_len=%u, nlmsg_type=RTM_GETQDISC, nlmsg_flags=NLM_F_DUMP"
	       ", nlmsg_seq=0, nlmsg_pid=0}, {tcm_family=AF_UNIX"
	       ", tcm_ifindex=" IFINDEX_LO_STR
	       ", tcm_handle=0, tcm_parent=0, tcm_info=0}"
	       ", [{nla_len=%u, nla_type=TCA_STAB}",
	       msg_len, msg_len - NLMSG_SPACE(hdrlen));
}

static void
print_uint16(const uint16_t *p, size_t idx)
{
	printf("%u", *p);
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	const int fd = create_nl_socket(NETLINK_ROUTE);
	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen),
				   NLA_HDRLEN + sizeof(struct tc_sizespec));

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

	static const struct tc_sizespec s = {
		.cell_log = 0xab,
		.size_log = 0xcd,
		.cell_align = 0xefab,
		.overhead = 0xcdadeefa,
		.linklayer = 0xefbaafeb,
		.mpu = 0xfebfaefb,
		.mtu = 0xacdbefab,
		.tsize = 0xbdeaabed
	};
	TEST_NESTED_NLATTR_OBJECT(fd, nlh0, hdrlen,
				  init_tcmsg, print_tcmsg,
				  TCA_STAB_BASE, pattern, s,
				  printf("{");
				  PRINT_FIELD_U(s, cell_log);
				  printf(", ");
				  PRINT_FIELD_U(s, size_log);
				  printf(", ");
				  PRINT_FIELD_D(s, cell_align);
				  printf(", ");
				  PRINT_FIELD_D(s, overhead);
				  printf(", ");
				  PRINT_FIELD_U(s, linklayer);
				  printf(", ");
				  PRINT_FIELD_U(s, mpu);
				  printf(", ");
				  PRINT_FIELD_U(s, mtu);
				  printf(", ");
				  PRINT_FIELD_U(s, tsize);
				  printf("}"));

	uint16_t data[2] = { 0xacbd, 0xefba };
	TEST_NESTED_NLATTR_ARRAY(fd, nlh0, hdrlen,
				 init_tcmsg, print_tcmsg,
				 TCA_STAB_DATA, pattern, data, print_uint16);

	puts("+++ exited with 0 +++");
	return 0;
}
