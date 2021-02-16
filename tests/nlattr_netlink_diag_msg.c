/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "test_nlattr.h"
#include <linux/netlink_diag.h>
#include <linux/sock_diag.h>

static void
init_netlink_diag_msg(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = msg_len,
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_DUMP
	);

	struct netlink_diag_msg *const msg = NLMSG_DATA(nlh);
	SET_STRUCT(struct netlink_diag_msg, msg,
		.ndiag_family = AF_NETLINK,
		.ndiag_type = SOCK_RAW,
		.ndiag_protocol = NETLINK_ROUTE,
		.ndiag_state = NETLINK_CONNECTED
	);
}

static void
print_netlink_diag_msg(const unsigned int msg_len)
{
	printf("{nlmsg_len=%u, nlmsg_type=SOCK_DIAG_BY_FAMILY"
	       ", nlmsg_flags=NLM_F_DUMP, nlmsg_seq=0, nlmsg_pid=0}"
	       ", {ndiag_family=AF_NETLINK, ndiag_type=SOCK_RAW"
	       ", ndiag_protocol=NETLINK_ROUTE, ndiag_state=NETLINK_CONNECTED"
	       ", ndiag_portid=0, ndiag_dst_portid=0, ndiag_dst_group=0"
	       ", ndiag_ino=0, ndiag_cookie=[0, 0]}",
	       msg_len);
}

static void
print_xlong(const unsigned long *p, size_t i)
{
	printf("%#lx", *p);
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	static const unsigned long groups[] = {
		(unsigned long) 0xdeadbeefbadc0dedULL,
		(unsigned long) 0xdeadbeefbadc0dedULL
	};
	static const struct netlink_diag_ring ndr = {
		.ndr_block_size = 0xfabfabdc,
		.ndr_block_nr = 0xabcdabda,
		.ndr_frame_size = 0xcbadbafa,
		.ndr_frame_nr = 0xdbcafadb
	};
	static const uint32_t flags =
		NDIAG_FLAG_CB_RUNNING | NDIAG_FLAG_PKTINFO;

	const int fd = create_nl_socket(NETLINK_SOCK_DIAG);
	const unsigned int hdrlen = sizeof(struct netlink_diag_msg);
	void *const nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen),
					 NLA_HDRLEN +
					 MAX(sizeof(groups), sizeof(ndr)));

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

	TEST_NLATTR_ARRAY(fd, nlh0, hdrlen,
			  init_netlink_diag_msg, print_netlink_diag_msg,
			  NETLINK_DIAG_GROUPS, pattern, groups, print_xlong);

	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_netlink_diag_msg, print_netlink_diag_msg,
			   NETLINK_DIAG_RX_RING, pattern, ndr,
			   printf("{");
			   PRINT_FIELD_U(ndr, ndr_block_size);
			   printf(", ");
			   PRINT_FIELD_U(ndr, ndr_block_nr);
			   printf(", ");
			   PRINT_FIELD_U(ndr, ndr_frame_size);
			   printf(", ");
			   PRINT_FIELD_U(ndr, ndr_frame_nr);
			   printf("}"));

	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_netlink_diag_msg, print_netlink_diag_msg,
			   NETLINK_DIAG_FLAGS, pattern, flags,
			   printf("NDIAG_FLAG_CB_RUNNING|NDIAG_FLAG_PKTINFO"));

	puts("+++ exited with 0 +++");
	return 0;
}
