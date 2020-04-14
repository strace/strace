/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include "test_nlattr.h"
#include <linux/pkt_cls.h>
#include <linux/rtnetlink.h>

#if !HAVE_DECL_TCA_ACT_PAD
enum { TCA_ACT_PAD = 5 };
#endif
#if !HAVE_DECL_TCA_ACT_COOKIE
enum { TCA_ACT_COOKIE = 6 };
#endif
#if !HAVE_DECL_TCA_ACT_FLAGS
enum { TCA_ACT_FLAGS = 7 };
#endif
#if !HAVE_DECL_TCA_ACT_HW_STATS
enum { TCA_ACT_HW_STATS = 8 };
#endif
#if !HAVE_DECL_TCA_ACT_USED_HW_STATS
enum { TCA_ACT_USED_HW_STATS = 9 };
#endif


static void
init_tcamsg(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = msg_len,
		.nlmsg_type = RTM_GETACTION,
		.nlmsg_flags = NLM_F_DUMP
	);

	struct tcamsg *const msg = NLMSG_DATA(nlh);
	SET_STRUCT(struct tcamsg, msg,
		.tca_family = AF_INET
	);
}

static void
print_tcamsg(const unsigned int msg_len)
{
	printf("{len=%u, type=RTM_GETACTION, flags=NLM_F_DUMP"
	       ", seq=0, pid=0}, {tca_family=AF_INET}",
	       msg_len);
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	const int fd = create_nl_socket(NETLINK_ROUTE);
	const unsigned int hdrlen = sizeof(struct tcamsg);
	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen), NLA_HDRLEN + 4);

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

	/* Invalid */
	static const unsigned int nla_invalid[] = { 10, 0xffff & NLA_TYPE_MASK };
	for (size_t i = 0; i < ARRAY_SIZE(nla_invalid); i++) {
		char nla_type_str[256];
		sprintf(nla_type_str, "%#x /* TCA_ACT_??? */", nla_invalid[i]);
		TEST_NLATTR_(fd, nlh0, hdrlen,
			     init_tcamsg, print_tcamsg,
			     nla_invalid[i], nla_type_str,
			     21, pattern, 21,
			     print_quoted_hex(pattern, 21));
	}

	/* Default decoder */
	static const struct {
		unsigned int val;
		const char *str;
	} nla_default[] = {
		{ ARG_STR(TCA_ACT_UNSPEC) },
		{ ARG_STR(TCA_ACT_OPTIONS) },
		{ ARG_STR(TCA_ACT_PAD) },
		{ ARG_STR(TCA_ACT_COOKIE) },
	};
	for (size_t i = 0; i < ARRAY_SIZE(nla_default); i++) {
		TEST_NLATTR_(fd, nlh0, hdrlen,
			     init_tcamsg, print_tcamsg,
			     nla_default[i].val, nla_default[i].str,
			     17, pattern, 17,
			     print_quoted_hex(pattern, 17));
	}

	/* TCA_ACT_KIND */
	TEST_NLATTR(fd, nlh0, hdrlen, init_tcamsg, print_tcamsg,
		    TCA_ACT_KIND, 21, pattern, 21,
		    print_quoted_cstring(pattern, 22));

	static const char kind[] = "Hello\tthere";
	TEST_NLATTR(fd, nlh0, hdrlen, init_tcamsg, print_tcamsg,
		    TCA_ACT_KIND, sizeof(kind), kind, sizeof(kind),
		    print_quoted_string(kind));

	/* TCA_ACT_INDEX */
	static uint32_t idx = 0xdeadc0de;
	TEST_NLATTR(fd, nlh0, hdrlen, init_tcamsg, print_tcamsg,
		    TCA_ACT_INDEX, sizeof(idx), &idx, sizeof(idx),
		    printf("%u", idx));

	/* TCA_ACT_FLAGS */
	static uint32_t flags = 0xfacebeef;
	TEST_NLATTR(fd, nlh0, hdrlen, init_tcamsg, print_tcamsg,
		    TCA_ACT_FLAGS, sizeof(flags), &flags, sizeof(flags),
		    printf("TCA_ACT_FLAGS_NO_PERCPU_STATS|0xfacebeee"));

	/* TCA_ACT_HW_STATS, TCA_ACT_USED_HW_STATS */
	static const struct strval32 nla_hw_st[] = {
		{ ARG_STR(TCA_ACT_HW_STATS) },
		{ ARG_STR(TCA_ACT_USED_HW_STATS) },
	};

	static uint32_t hw_st = 0xfacebeef;
	for (size_t i = 0; i < ARRAY_SIZE(nla_hw_st); i++) {
		TEST_NLATTR_(fd, nlh0, hdrlen, init_tcamsg, print_tcamsg,
			     nla_hw_st[i].val, nla_hw_st[i].str,
			     sizeof(hw_st), &hw_st, sizeof(hw_st),
			     printf("TCA_ACT_HW_STATS_IMMEDIATE|"
				    "TCA_ACT_HW_STATS_DELAYED|0xfacebeec"));
	}

	puts("+++ exited with 0 +++");
	return 0;
}
