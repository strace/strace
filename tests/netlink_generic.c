/*
 * Copyright (c) 2017-2024 The strace developers.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "xmalloc.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "netlink.h"
#include "test_netlink.h"
#include "test_nlattr.h"
#include <linux/genetlink.h>

static void
test_hdr(const int fd)
{
	static const struct genlmsghdr hdr = {
		.cmd = CTRL_CMD_GETFAMILY,
		.version = 1
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(hdr));

	TEST_NETLINK_OBJECT_EX_(fd, nlh0,
				0xffff, "0xffff /* GENERIC_FAMILY_??? */",
				NLM_F_REQUEST | NLM_F_DUMP,
				"NLM_F_REQUEST|NLM_F_DUMP",
				hdr, print_quoted_hex,
				printf("{cmd=%#x, version=1}", hdr.cmd);
				);
}

static void
init_genlmsghdr(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = msg_len,
		.nlmsg_type = 0xffff,
		.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP
	);

	struct genlmsghdr *const hdr = NLMSG_DATA(nlh);
	SET_STRUCT(struct genlmsghdr, hdr,
		.cmd = CTRL_CMD_GETFAMILY,
		.version = 2,
		.reserved = 0xfeed
	);
}

static void
print_genlmsghdr(const unsigned int msg_len)
{
	printf("{nlmsg_len=%u, nlmsg_type=%s, nlmsg_flags=%s, nlmsg_seq=0"
	       ", nlmsg_pid=0}, {cmd=%#x, version=2, reserved=%#x}",
	       msg_len, "0xffff /* GENERIC_FAMILY_??? */",
	       "NLM_F_REQUEST|NLM_F_DUMP", CTRL_CMD_GETFAMILY, 0xfeed);
}

static void
test_nla(const int fd)
{
	static char pattern[DEFAULT_STRLEN];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);
	const char *nla_type_str = xasprintf("%#x", CTRL_ATTR_OP);

	const unsigned int hdrlen = sizeof(struct genlmsghdr);
	void *const nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen),
					 NLA_HDRLEN + sizeof(pattern));

	TEST_NLATTR_(fd, nlh0, hdrlen,
		     init_genlmsghdr, print_genlmsghdr,
		     CTRL_ATTR_OP, nla_type_str,
		     sizeof(pattern), pattern, sizeof(pattern),
		     print_quoted_hex(pattern, sizeof(pattern)));
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

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	int fd = create_nl_socket(NETLINK_GENERIC);

	test_hdr(fd);
	test_nla(fd);
	test_nlmsg_done(fd);

	printf("+++ exited with 0 +++\n");

	return 0;
}
