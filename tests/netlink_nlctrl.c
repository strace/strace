/*
 * Copyright (c) 2017-2023 The strace developers.
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
	static const struct strval32 cmds[] = {
		{ ARG_STR(CTRL_CMD_UNSPEC) },
		{ ARG_STR(CTRL_CMD_NEWFAMILY) },
		{ ARG_STR(CTRL_CMD_DELFAMILY) },
		{ ARG_STR(CTRL_CMD_GETFAMILY) },
		{ ARG_STR(CTRL_CMD_NEWOPS) },
		{ ARG_STR(CTRL_CMD_DELOPS) },
		{ ARG_STR(CTRL_CMD_GETOPS) },
		{ ARG_STR(CTRL_CMD_NEWMCAST_GRP) },
		{ ARG_STR(CTRL_CMD_DELMCAST_GRP) },
		{ ARG_STR(CTRL_CMD_GETMCAST_GRP) },
		{ ARG_STR(CTRL_CMD_GETPOLICY) },
	};
	struct genlmsghdr hdr = {
		.version = 1
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(hdr));

	for (size_t i = 0; i < ARRAY_SIZE(cmds); ++i) {
		hdr.cmd = cmds[i].val;
		TEST_NETLINK_OBJECT_EX_(fd, nlh0,
					GENL_ID_CTRL, "nlctrl",
					NLM_F_REQUEST | NLM_F_DUMP,
					"NLM_F_REQUEST|NLM_F_DUMP",
					hdr, print_quoted_hex,
					printf("{cmd=%s, version=1}",
					       cmds[i].str);
				       );
	}
}

static void
init_genlmsghdr(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = msg_len,
		.nlmsg_type = GENL_ID_CTRL,
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
	       ", nlmsg_pid=0}, {cmd=%s, version=2, reserved=%#x}",
	       msg_len, "nlctrl", "NLM_F_REQUEST|NLM_F_DUMP",
	       "CTRL_CMD_GETFAMILY", 0xfeed);
}

static void
test_nla(const int fd, void *const nlh0)
{
	static char pattern[DEFAULT_STRLEN];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);
	const char *nla_type_str = xasprintf("%#x", CTRL_ATTR_FAMILY_NAME);

	TEST_NLATTR_(fd, nlh0, sizeof(struct genlmsghdr),
		     init_genlmsghdr, print_genlmsghdr,
		     CTRL_ATTR_FAMILY_NAME, nla_type_str,
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
	void *const nlh0 = midtail_alloc(NLMSG_SPACE(sizeof(struct genlmsghdr)),
					 NLA_HDRLEN + DEFAULT_STRLEN);

	test_hdr(fd);
	test_nla(fd, nlh0);
	test_nlmsg_done(fd);

	printf("+++ exited with 0 +++\n");

	return 0;
}
