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
test_nla_unknown(const int fd, void *const nlh0)
{
	static char pattern[DEFAULT_STRLEN];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);
	const char *nla_type_str = xasprintf("%#x /* CTRL_ATTR_??? */",
					     CTRL_ATTR_OP + 1);

	TEST_NLATTR_(fd, nlh0, sizeof(struct genlmsghdr),
		     init_genlmsghdr, print_genlmsghdr,
		     CTRL_ATTR_OP + 1, nla_type_str,
		     sizeof(pattern), pattern, sizeof(pattern),
		     print_quoted_hex(pattern, sizeof(pattern)));
}

static void
test_nla_x16(const int fd, void *const nlh0)
{
	static const struct strval16 attrs[] = {
		{ ARG_STR(CTRL_ATTR_FAMILY_ID) },
	};
	char pattern[sizeof(uint16_t)];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

	for (size_t i = 0; i < ARRAY_SIZE(attrs); ++i) {
		check_x16_nlattr(fd, nlh0, sizeof(struct genlmsghdr),
				 init_genlmsghdr, print_genlmsghdr,
				 attrs[i].val, attrs[i].str, pattern, 0);
	}
}

static void
test_nla_u32(const int fd, void *const nlh0)
{
	static const struct strval16 attrs[] = {
		{ ARG_STR(CTRL_ATTR_VERSION) },
		{ ARG_STR(CTRL_ATTR_HDRSIZE) },
		{ ARG_STR(CTRL_ATTR_MAXATTR) },
		{ ARG_STR(CTRL_ATTR_OP) },
	};
	char pattern[sizeof(uint32_t)];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

	for (size_t i = 0; i < ARRAY_SIZE(attrs); ++i) {
		check_u32_nlattr(fd, nlh0, sizeof(struct genlmsghdr),
				 init_genlmsghdr, print_genlmsghdr,
				 attrs[i].val, attrs[i].str, pattern, 0);
	}
}

static void
test_nla_str(const int fd, void *const nlh0)
{
	static const struct strval16 attrs[] = {
		{ ARG_STR(CTRL_ATTR_FAMILY_NAME) },
	};

	for (size_t i = 0; i < ARRAY_SIZE(attrs); ++i) {
		char str[DEFAULT_STRLEN];

		fill_memory_ex(str, sizeof(str), '0', 10);
	        TEST_NLATTR_(fd, nlh0, sizeof(struct genlmsghdr),
			     init_genlmsghdr, print_genlmsghdr,
			     attrs[i].val, attrs[i].str,
			     sizeof(str), str, sizeof(str),
			     printf("\"%.*s\"...", (int) sizeof(str), str)
			    );

		str[sizeof(str) - 1] = '\0';
	        TEST_NLATTR_(fd, nlh0, sizeof(struct genlmsghdr),
			     init_genlmsghdr, print_genlmsghdr,
			     attrs[i].val, attrs[i].str,
			     sizeof(str), str, sizeof(str),
			     printf("\"%s\"", str)
			    );
	}

}

static void
test_nla_ops(const int fd)
{
	static const struct strval16 attrs[] = {
		{ ARG_STR(CTRL_ATTR_OP_ID) },
		{ ARG_STR(CTRL_ATTR_OP_FLAGS) },
	};
	static const struct strval32 cmds[] = {
		{ ARG_STR(CTRL_CMD_GETFAMILY) },
		{ ARG_STR(CTRL_CMD_GETPOLICY) },
	};
	static const struct strval32 flags[] = {
		{ ARG_STR(GENL_CMD_CAP_DO|GENL_CMD_CAP_DUMP|GENL_CMD_CAP_HASPOL) },
		{ ARG_STR(GENL_ADMIN_PERM|GENL_UNS_ADMIN_PERM) },
	};

	const struct {
		struct nlattr h;
		struct {
			struct nlattr h;
			uint32_t v;
		} a[2];
	} src[] = {
		{
			{ sizeof(src[0]), 1 },
			{
				{
					{ sizeof(src[0].a[0]), attrs[0].val },
					cmds[0].val
				}, {
					{ sizeof(src[0].a[1]), attrs[1].val },
					flags[0].val
				}
			}
		}, {
			{ sizeof(src[1]), 2 },
			{
				{
					{ sizeof(src[1].a[0]), attrs[0].val },
					cmds[1].val
				}, {
					{ sizeof(src[1].a[1]), attrs[1].val },
					flags[1].val
				}
			}
		}
	};
	void *const nlh0 = midtail_alloc(NLMSG_SPACE(sizeof(struct genlmsghdr)),
					 NLA_HDRLEN + sizeof(src));

	TEST_NLATTR(fd, nlh0, sizeof(struct genlmsghdr),
		    init_genlmsghdr, print_genlmsghdr, CTRL_ATTR_OPS,
		    sizeof(src), src, sizeof(src),
		    printf("["
			    "[{nla_len=%u, nla_type=%#x}, "
			     "["
			      "[{nla_len=%u, nla_type=%s}, %s], "
			      "[{nla_len=%u, nla_type=%s}, %s]"
			     "]"
			    "], "
			    "[{nla_len=%u, nla_type=%#x}, "
			     "["
			      "[{nla_len=%u, nla_type=%s}, %s], "
			      "[{nla_len=%u, nla_type=%s}, %s]"
			     "]"
			    "]"
			   "]",
			   src[0].h.nla_len, src[0].h.nla_type,
			   src[0].a[0].h.nla_len, attrs[0].str, cmds[0].str,
			   src[0].a[1].h.nla_len, attrs[1].str, flags[0].str,
			   src[1].h.nla_len, src[1].h.nla_type,
			   src[1].a[0].h.nla_len, attrs[0].str, cmds[1].str,
			   src[1].a[1].h.nla_len, attrs[1].str, flags[1].str)
		    );
}

static void
test_nla_mcast(const int fd)
{
	static const struct strval16 attrs[] = {
		{ ARG_STR(CTRL_ATTR_MCAST_GRP_ID) },
		{ ARG_STR(CTRL_ATTR_MCAST_GRP_NAME) },
	};

	struct {
		struct nlattr h;
		struct {
			struct nlattr h;
			uint32_t v;
		} a0;
		struct {
			struct nlattr h;
			char s[DEFAULT_STRLEN];
		} a1;
	} src[] = {
		{
			{ sizeof(src[0]), 1 },
			{
				{ sizeof(src[0].a0), attrs[0].val },
				0xcafef00d
			}, {
				{ sizeof(src[0].a1), attrs[1].val },
				""
			}
		}, {
			{ sizeof(src[1]), 2 },
			{
				{ sizeof(src[1].a0), attrs[0].val },
				0xdeadface
			}, {
				{ sizeof(src[1].a1), attrs[1].val },
				""
			}
		}
	};

	char strs[2][DEFAULT_STRLEN];
	fill_memory_ex(strs, sizeof(strs), '0', 10);
	strs[0][sizeof(strs[0]) - 1] = '\0';
	strs[1][sizeof(strs[1]) - 1] = '\0';
	memcpy(src[0].a1.s, strs[0], DEFAULT_STRLEN);
	memcpy(src[1].a1.s, strs[1], DEFAULT_STRLEN);

	void *const nlh0 = midtail_alloc(NLMSG_SPACE(sizeof(struct genlmsghdr)),
					 NLA_HDRLEN + sizeof(src));

	TEST_NLATTR(fd, nlh0, sizeof(struct genlmsghdr),
		    init_genlmsghdr, print_genlmsghdr, CTRL_ATTR_MCAST_GROUPS,
		    sizeof(src), src, sizeof(src),
		    printf("["
			    "[{nla_len=%u, nla_type=%#x}, "
			     "["
			      "[{nla_len=%u, nla_type=%s}, %#x], "
			      "[{nla_len=%u, nla_type=%s}, \"%s\"]"
			     "]"
			    "], "
			    "[{nla_len=%u, nla_type=%#x}, "
			     "["
			      "[{nla_len=%u, nla_type=%s}, %#x], "
			      "[{nla_len=%u, nla_type=%s}, \"%s\"]"
			     "]"
			    "]"
			   "]",
			   src[0].h.nla_len, src[0].h.nla_type,
			   src[0].a0.h.nla_len, attrs[0].str, src[0].a0.v,
			   src[0].a1.h.nla_len, attrs[1].str, src[0].a1.s,
			   src[1].h.nla_len, src[1].h.nla_type,
			   src[1].a0.h.nla_len, attrs[0].str, src[1].a0.v,
			   src[1].a1.h.nla_len, attrs[1].str, src[1].a1.s)
		    );
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
	test_nla_unknown(fd, nlh0);
	test_nla_x16(fd, nlh0);
	test_nla_u32(fd, nlh0);
	test_nla_str(fd, nlh0);
	test_nla_ops(fd);
	test_nla_mcast(fd);
	test_nlmsg_done(fd);

	printf("+++ exited with 0 +++\n");

	return 0;
}
