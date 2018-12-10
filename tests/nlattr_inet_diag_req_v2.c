/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2018 The strace developers.
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
static const unsigned int hdrlen = sizeof(struct inet_diag_req_v2);
static void *nlh0;
static char pattern[4096];

static void
init_inet_diag_req_v2(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = msg_len,
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_REQUEST
	);

	struct inet_diag_req_v2 *const req = NLMSG_DATA(nlh);
	SET_STRUCT(struct inet_diag_req_v2, req,
		.sdiag_family = AF_INET,
		.idiag_ext = 1 << (INET_DIAG_CONG - 1),
		.sdiag_protocol = IPPROTO_TCP,
		.idiag_states = 1 << TCP_CLOSE,
		.id.idiag_if = ifindex_lo()
	);

	if (!inet_pton(AF_INET, address, req->id.idiag_src) ||
	    !inet_pton(AF_INET, address, req->id.idiag_dst))
		perror_msg_and_skip("inet_pton");
}

static void
print_inet_diag_req_v2(const unsigned int msg_len)
{
	printf("{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}"
	       ", {sdiag_family=AF_INET, sdiag_protocol=IPPROTO_TCP"
	       ", idiag_ext=1<<(INET_DIAG_CONG-1)"
	       ", idiag_states=1<<TCP_CLOSE"
	       ", id={idiag_sport=htons(0), idiag_dport=htons(0)"
	       ", idiag_src=inet_addr(\"%s\")"
	       ", idiag_dst=inet_addr(\"%s\")"
	       ", idiag_if=" IFINDEX_LO_STR
	       ", idiag_cookie=[0, 0]}}",
	       msg_len, address, address);
}

static void
test_inet_diag_bc_op(const int fd)
{
	static const struct inet_diag_bc_op op = {
		.code = INET_DIAG_BC_S_COND,
		.yes = 0xaf,
		.no = 0xafcd
	};
	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_inet_diag_req_v2, print_inet_diag_req_v2,
			   INET_DIAG_REQ_BYTECODE, pattern, op,
			   printf("{code=INET_DIAG_BC_S_COND");
			   PRINT_FIELD_U(", ", op, yes);
			   PRINT_FIELD_U(", ", op, no);
			   printf("}"));
}

static void
print_inet_diag_bc_op(const char *const code)
{
	printf("{{code=%s, yes=0, no=0}, ", code);
}

static void
test_inet_diag_bc_s_cond(const int fd)
{
	static const struct inet_diag_bc_op op = {
		.code = INET_DIAG_BC_S_COND,
	};
	static const struct inet_diag_hostcond cond = {
		.family = AF_UNSPEC,
		.prefix_len = 0xad,
		.port = 0xadfa
	};
	char buf[sizeof(op) + sizeof(cond)];
	memcpy(buf, &op, sizeof(op));

	const unsigned int plen = sizeof(cond) - 1 > DEFAULT_STRLEN ?
		sizeof(op) + DEFAULT_STRLEN : sizeof(buf) - 1;
	memcpy(buf + sizeof(op), &pattern, sizeof(cond));
	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_inet_diag_req_v2, print_inet_diag_req_v2,
		    INET_DIAG_REQ_BYTECODE,
		    plen, buf, plen,
		    print_inet_diag_bc_op("INET_DIAG_BC_S_COND");
		    print_quoted_hex(buf + sizeof(op), plen - sizeof(op));
		    printf("}"));

	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_inet_diag_req_v2, print_inet_diag_req_v2,
		    INET_DIAG_REQ_BYTECODE,
		    sizeof(buf), buf, sizeof(buf) - 1,
		    print_inet_diag_bc_op("INET_DIAG_BC_S_COND");
		    printf("%p}", RTA_DATA(TEST_NLATTR_nla) + sizeof(op)));

	memcpy(buf + sizeof(op), &cond, sizeof(cond));
	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_inet_diag_req_v2, print_inet_diag_req_v2,
		    INET_DIAG_REQ_BYTECODE,
		    sizeof(buf), buf, sizeof(buf),
		    print_inet_diag_bc_op("INET_DIAG_BC_S_COND");
		    printf("{family=AF_UNSPEC");
		    PRINT_FIELD_U(", ", cond, prefix_len);
		    PRINT_FIELD_U(", ", cond, port);
		    printf("}}"));
}

static void
print_inet_diag_hostcond(const char *const family)
{
	printf("{family=%s, prefix_len=0, port=0, ", family);
}

static void
test_in_addr(const int fd)
{
	static const struct inet_diag_bc_op op = {
		.code = INET_DIAG_BC_S_COND,
	};
	static const struct inet_diag_hostcond cond = {
		.family = AF_INET,
	};
	struct in_addr addr;
	if (!inet_pton(AF_INET, address, &addr))
		perror_msg_and_skip("inet_pton");

	char buf[sizeof(op) + sizeof(cond) + sizeof(addr)];
	memcpy(buf, &op, sizeof(op));
	memcpy(buf + sizeof(op), &cond, sizeof(cond));

	const unsigned int plen = sizeof(addr) - 1 > DEFAULT_STRLEN ?
		sizeof(cond) + sizeof(cond) + DEFAULT_STRLEN : sizeof(buf) - 1;
	memcpy(buf + sizeof(op) + sizeof(cond), &pattern, sizeof(addr));
	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_inet_diag_req_v2, print_inet_diag_req_v2,
		    INET_DIAG_REQ_BYTECODE,
		    plen, buf, plen,
		    print_inet_diag_bc_op("INET_DIAG_BC_S_COND");
		    print_inet_diag_hostcond("AF_INET");
		    printf("addr=");
		    print_quoted_hex(pattern, plen - sizeof(op) - sizeof(cond));
		    printf("}}"));

	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_inet_diag_req_v2, print_inet_diag_req_v2,
		    INET_DIAG_REQ_BYTECODE,
		    sizeof(buf), buf, sizeof(buf) - 1,
		    print_inet_diag_bc_op("INET_DIAG_BC_S_COND");
		    print_inet_diag_hostcond("AF_INET");
		    printf("addr=%p}}",
			   RTA_DATA(TEST_NLATTR_nla)
			   + sizeof(op) + sizeof(cond)));

	memcpy(buf + sizeof(op) + sizeof(cond), &addr, sizeof(addr));
	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_inet_diag_req_v2, print_inet_diag_req_v2,
		    INET_DIAG_REQ_BYTECODE,
		    sizeof(buf), buf, sizeof(buf),
		    print_inet_diag_bc_op("INET_DIAG_BC_S_COND");
		    print_inet_diag_hostcond("AF_INET");
		    printf("addr=inet_addr(\"%s\")}}", address));
}

static void
test_in6_addr(const int fd)
{
	const char address6[] = "12:34:56:78:90:ab:cd:ef";
	static const struct inet_diag_bc_op op = {
		.code = INET_DIAG_BC_S_COND,
	};
	static const struct inet_diag_hostcond cond = {
		.family = AF_INET6,
	};
	struct in6_addr addr;
	if (!inet_pton(AF_INET6, address6, &addr))
		perror_msg_and_skip("inet_pton");

	char buf[sizeof(op) + sizeof(cond) + sizeof(addr)];
	memcpy(buf, &op, sizeof(op));
	memcpy(buf + sizeof(op), &cond, sizeof(cond));

	const unsigned int plen = sizeof(addr) - 1 > DEFAULT_STRLEN ?
		sizeof(cond) + sizeof(cond) + DEFAULT_STRLEN : sizeof(buf) - 1;
	memcpy(buf + sizeof(op) + sizeof(cond), &pattern, sizeof(addr));
	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_inet_diag_req_v2, print_inet_diag_req_v2,
		    INET_DIAG_REQ_BYTECODE,
		    plen, buf, plen,
		    print_inet_diag_bc_op("INET_DIAG_BC_S_COND");
		    print_inet_diag_hostcond("AF_INET6");
		    printf("addr=");
		    print_quoted_hex(pattern, plen - sizeof(op) - sizeof(cond));
		    printf("}}"));

	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_inet_diag_req_v2, print_inet_diag_req_v2,
		    INET_DIAG_REQ_BYTECODE,
		    sizeof(buf), buf, sizeof(buf) - 1,
		    print_inet_diag_bc_op("INET_DIAG_BC_S_COND");
		    print_inet_diag_hostcond("AF_INET6");
		    printf("addr=%p}}",
			   RTA_DATA(TEST_NLATTR_nla)
			   + sizeof(op) + sizeof(cond)));

	memcpy(buf + sizeof(op) + sizeof(cond), &addr, sizeof(addr));
	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_inet_diag_req_v2, print_inet_diag_req_v2,
		    INET_DIAG_REQ_BYTECODE,
		    sizeof(buf), buf, sizeof(buf),
		    print_inet_diag_bc_op("INET_DIAG_BC_S_COND");
		    print_inet_diag_hostcond("AF_INET6");
		    printf("inet_pton(AF_INET6, \"%s\", &addr)}}", address6));
}

static void
test_inet_diag_bc_dev_cond(const int fd)
{
	static const struct inet_diag_bc_op op = {
		.code = INET_DIAG_BC_DEV_COND,
	};
	const uint32_t ifindex = ifindex_lo();
	char buf[sizeof(op) + sizeof(ifindex)];
	memcpy(buf, &op, sizeof(op));
	memcpy(buf + sizeof(op), pattern, sizeof(ifindex));

	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_inet_diag_req_v2, print_inet_diag_req_v2,
		    INET_DIAG_REQ_BYTECODE,
		    sizeof(buf) - 1, buf, sizeof(buf) - 1,
		    print_inet_diag_bc_op("INET_DIAG_BC_DEV_COND");
		    print_quoted_hex(pattern, sizeof(ifindex) - 1);
		    printf("}"));

	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_inet_diag_req_v2, print_inet_diag_req_v2,
		    INET_DIAG_REQ_BYTECODE,
		    sizeof(buf), buf, sizeof(buf) - 1,
		    print_inet_diag_bc_op("INET_DIAG_BC_DEV_COND");
		    printf("%p}", RTA_DATA(TEST_NLATTR_nla) + sizeof(op)));

	memcpy(buf + sizeof(op), &ifindex, sizeof(ifindex));
	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_inet_diag_req_v2, print_inet_diag_req_v2,
		    INET_DIAG_REQ_BYTECODE,
		    sizeof(buf), buf, sizeof(buf),
		    print_inet_diag_bc_op("INET_DIAG_BC_DEV_COND");
		    printf(IFINDEX_LO_STR "}"));
}

static void
test_inet_diag_bc_s_le(const int fd)
{
	static const struct inet_diag_bc_op op[] = {
		{
			.code = INET_DIAG_BC_S_LE,
		},
		{
			.code = INET_DIAG_BC_DEV_COND,
			.yes = 0xaf,
			.no = 0xafcd
		}
	};

	char buf[sizeof(op)];
	memcpy(buf, op, sizeof(op[0]));
	memcpy(buf + sizeof(op[0]), pattern, sizeof(op[1]));

	const unsigned int plen = sizeof(op[1]) - 1 > DEFAULT_STRLEN ?
		sizeof(op[0]) + DEFAULT_STRLEN : sizeof(buf) - 1;
	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_inet_diag_req_v2, print_inet_diag_req_v2,
		    INET_DIAG_REQ_BYTECODE,
		    plen, buf, plen,
		    print_inet_diag_bc_op("INET_DIAG_BC_S_LE");
		    print_quoted_hex(buf + sizeof(op[0]), plen - sizeof(op[0]));
		    printf("}"));

	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_inet_diag_req_v2, print_inet_diag_req_v2,
		    INET_DIAG_REQ_BYTECODE,
		    sizeof(buf), buf, sizeof(buf) - 1,
		    print_inet_diag_bc_op("INET_DIAG_BC_S_LE");
		    printf("%p}", RTA_DATA(TEST_NLATTR_nla) + sizeof(op[0])));

	memcpy(buf + sizeof(op[0]), &op[1], sizeof(op[1]));
	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_inet_diag_req_v2, print_inet_diag_req_v2,
		    INET_DIAG_REQ_BYTECODE,
		    sizeof(buf), buf, sizeof(buf),
		    print_inet_diag_bc_op("INET_DIAG_BC_S_LE");
		    printf("{code=INET_DIAG_BC_DEV_COND");
		    PRINT_FIELD_U(", ", op[1], yes);
		    PRINT_FIELD_U(", ", op[1], no);
		    printf("}}"));
};

static void
test_inet_diag_bc_mark_cond(const int fd)
{
	static const struct inet_diag_bc_op op = {
		.code = INET_DIAG_BC_MARK_COND,
	};
	static const struct inet_diag_markcond markcond = {
		.mark = 0xafbcafcd,
		.mask = 0xbafaacda
	};
	char buf[sizeof(op) + sizeof(markcond)];
	memcpy(buf, &op, sizeof(op));
	memcpy(buf + sizeof(op), pattern, sizeof(markcond));

	const unsigned int plen = sizeof(markcond) - 1 > DEFAULT_STRLEN ?
		sizeof(markcond) + DEFAULT_STRLEN : sizeof(buf) - 1;
	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_inet_diag_req_v2, print_inet_diag_req_v2,
		    INET_DIAG_REQ_BYTECODE,
		    plen, buf, plen,
		    print_inet_diag_bc_op("INET_DIAG_BC_MARK_COND");
		    print_quoted_hex(buf + sizeof(op), plen - sizeof(op));
		    printf("}"));

	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_inet_diag_req_v2, print_inet_diag_req_v2,
		    INET_DIAG_REQ_BYTECODE,
		    sizeof(buf), buf, sizeof(buf) - 1,
		    print_inet_diag_bc_op("INET_DIAG_BC_MARK_COND");
		    printf("%p}", RTA_DATA(TEST_NLATTR_nla) + sizeof(op)));

	memcpy(buf + sizeof(op), &markcond, sizeof(markcond));
	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_inet_diag_req_v2, print_inet_diag_req_v2,
		    INET_DIAG_REQ_BYTECODE,
		    sizeof(buf), buf, sizeof(buf),
		    print_inet_diag_bc_op("INET_DIAG_BC_MARK_COND");
		    PRINT_FIELD_U("{", markcond, mark);
		    PRINT_FIELD_U(", ", markcond, mask);
		    printf("}}"));
}

static void
test_inet_diag_bc_nop(const int fd)
{
	static const struct inet_diag_bc_op op = {
		.code = INET_DIAG_BC_AUTO,
	};
	char buf[sizeof(op) + 4];
	memcpy(buf, &op, sizeof(op));
	memcpy(buf + sizeof(op), pattern, 4);

	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_inet_diag_req_v2, print_inet_diag_req_v2,
		    INET_DIAG_REQ_BYTECODE,
		    sizeof(buf), buf, sizeof(buf),
		    print_inet_diag_bc_op("INET_DIAG_BC_AUTO");
		    print_quoted_hex(buf + sizeof(op),
				     sizeof(buf) - sizeof(op));
		    printf("}"));
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	int fd = create_nl_socket(NETLINK_SOCK_DIAG);
	nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen), NLA_HDRLEN +
			sizeof(struct inet_diag_bc_op) +
				sizeof(struct inet_diag_hostcond) +
				sizeof(struct in6_addr) + DEFAULT_STRLEN);
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

	test_inet_diag_bc_op(fd);
	test_inet_diag_bc_s_cond(fd);
	test_in_addr(fd);
	test_in6_addr(fd);
	test_inet_diag_bc_dev_cond(fd);
	test_inet_diag_bc_s_le(fd);
	test_inet_diag_bc_mark_cond(fd);
	test_inet_diag_bc_nop(fd);

	printf("+++ exited with 0 +++\n");
	return 0;
}
