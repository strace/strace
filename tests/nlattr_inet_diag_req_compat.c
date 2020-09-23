/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/tcp.h>
#include "test_nlattr.h"
#include <linux/inet_diag.h>
#include <linux/rtnetlink.h>
#include <linux/sock_diag.h>

static const char address[] = "10.11.12.13";

static void
init_inet_diag_req(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = msg_len,
		.nlmsg_type = TCPDIAG_GETSOCK,
		.nlmsg_flags = NLM_F_REQUEST
	);

	struct inet_diag_req *const req = NLMSG_DATA(nlh);
	SET_STRUCT(struct inet_diag_req, req,
		.idiag_family = AF_INET,
		.idiag_ext = 1 << (INET_DIAG_TOS - 1),
		.idiag_states = 1 << TCP_LAST_ACK,
		.id.idiag_if = ifindex_lo()
	);

	if (!inet_pton(AF_INET, address, req->id.idiag_src) ||
	    !inet_pton(AF_INET, address, req->id.idiag_dst))
		perror_msg_and_skip("inet_pton");
}

static void
print_inet_diag_req(const unsigned int msg_len)
{
	printf("{len=%u, type=TCPDIAG_GETSOCK, flags=NLM_F_REQUEST"
	       ", seq=0, pid=0}, {idiag_family=AF_INET"
	       ", idiag_src_len=0, idiag_dst_len=0"
	       ", idiag_ext=1<<(INET_DIAG_TOS-1)"
	       ", id={idiag_sport=htons(0), idiag_dport=htons(0)"
	       ", idiag_src=inet_addr(\"%s\")"
	       ", idiag_dst=inet_addr(\"%s\")"
	       ", idiag_if=" IFINDEX_LO_STR
	       ", idiag_cookie=[0, 0]}"
	       ", idiag_states=1<<TCP_LAST_ACK, idiag_dbs=0}",
	       msg_len, address, address);
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	int fd = create_nl_socket(NETLINK_SOCK_DIAG);
	const unsigned int hdrlen = sizeof(struct inet_diag_req);
	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen), NLA_HDRLEN + 4);

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

	char nla_type_str[256];
	sprintf(nla_type_str, "%#x /* INET_DIAG_REQ_??? */",
		INET_DIAG_REQ_PROTOCOL + 1);
	TEST_NLATTR_(fd, nlh0, hdrlen,
		     init_inet_diag_req, print_inet_diag_req,
		     INET_DIAG_REQ_PROTOCOL + 1, nla_type_str,
		     4, pattern, 4,
		     print_quoted_hex(pattern, 4));

	puts("+++ exited with 0 +++");
	return 0;
}
