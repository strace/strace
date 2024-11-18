/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2024 The strace developers.
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
#if !HAVE_DECL_TCA_ACT_IN_HW_COUNT
enum { TCA_ACT_IN_HW_COUNT = 10 };
#endif

static const unsigned int hdrlen = sizeof(struct tcamsg);

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
	printf("{nlmsg_len=%u, nlmsg_type=" XLAT_FMT ", nlmsg_flags=" XLAT_FMT
	       ", nlmsg_seq=0, nlmsg_pid=0}, {tca_family=" XLAT_FMT "}",
	       msg_len, XLAT_ARGS(RTM_GETACTION), XLAT_ARGS(NLM_F_DUMP),
	       XLAT_ARGS(AF_INET));
}

static void
init_tcamsg_tab(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	init_tcamsg(nlh, msg_len);

	struct nlattr *nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = msg_len - NLMSG_SPACE(hdrlen),
		.nla_type = 1,
	);
}

static void
print_tcamsg_tab(const unsigned int msg_len)
{
	print_tcamsg(msg_len);
	printf(", [{nla_len=%u, nla_type=" XLAT_FMT "}",
	       msg_len - NLMSG_SPACE(hdrlen), XLAT_ARGS(TCA_ROOT_TAB));
}

static uint16_t tab_idx;

static void
init_tcamsg_tab_item(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	init_tcamsg_tab(nlh, msg_len);

	struct nlattr *nla = NLMSG_ATTR(nlh, hdrlen + NLA_HDRLEN);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = msg_len - NLMSG_SPACE(hdrlen) - NLA_HDRLEN,
		.nla_type = tab_idx,
	);
}

static void
print_tcamsg_tab_item(const unsigned int msg_len)
{
	print_tcamsg_tab(msg_len);
	printf(", [{nla_len=%u, nla_type=%#x}",
	       msg_len - NLMSG_SPACE(hdrlen) - NLA_HDRLEN, tab_idx);
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
	static const unsigned int nla_invalid[] = { 6, 0xffff & NLA_TYPE_MASK };
	for (size_t i = 0; i < ARRAY_SIZE(nla_invalid); i++) {
		char nla_type_str[256];
		sprintf(nla_type_str, "%#x" NRAW(" /* TCA_ROOT_??? */"),
			nla_invalid[i]);
		TEST_NLATTR_(fd, nlh0, hdrlen,
			     init_tcamsg, print_tcamsg,
			     nla_invalid[i], nla_type_str,
			     21, pattern, 21,
			     print_quoted_hex(pattern, 21));
	}

	/* Default decoder */
	static const struct strval32 nla_default[] = {
		{ ARG_XLAT_KNOWN(0, "TCA_ROOT_UNSPEC") },
	};
	for (size_t i = 0; i < ARRAY_SIZE(nla_default); i++) {
		TEST_NLATTR_(fd, nlh0, hdrlen,
			     init_tcamsg, print_tcamsg,
			     nla_default[i].val, nla_default[i].str,
			     17, pattern, 17,
			     print_quoted_hex(pattern, 17));
	}

	/* TCA_ROOT_TAB: Invalid */
	TEST_NLATTR_(fd, nlh0, hdrlen,
		     init_tcamsg, print_tcamsg,
		     TCA_ROOT_TAB, XLAT_KNOWN(0x1, "TCA_ROOT_TAB"),
		     3, &pattern, 3,
		     printf("\"\\x61\\x62\\x63\""));

	/* TCA_ROOT_TAB: item: invalid */
	TEST_NESTED_NLATTR_(fd, nlh0, hdrlen,
			    init_tcamsg_tab, print_tcamsg_tab,
			    0, "0", 3, &pattern, 3, 1,
			    printf("\"\\x61\\x62\\x63\""));
	tab_idx++;

	/* TCA_ROOT_TAB: item: default decoder */
	static const struct strval32 tcaa_default[] = {
		{ ARG_XLAT_KNOWN(0, "TCA_ACT_UNSPEC") },
		{ ARG_XLAT_KNOWN(0x2, "TCA_ACT_OPTIONS") },
		{ ARG_XLAT_KNOWN(0x5, "TCA_ACT_PAD") },
		{ ARG_XLAT_KNOWN(0x6, "TCA_ACT_COOKIE") },
		{ 11, "0xb" NRAW(" /* TCA_ACT_??? */") },
	};
	for (size_t i = 0; i < ARRAY_SIZE(tcaa_default); i++) {
		TEST_NESTED_NLATTR_(fd, nlh0, hdrlen,
				    init_tcamsg_tab_item, print_tcamsg_tab_item,
				    tcaa_default[i].val, tcaa_default[i].str,
				    17, pattern, 17, 2,
				    print_quoted_hex(pattern, 17));
		tab_idx++;
	}

	/* TCA_ROOT_TAB: item: TCA_ACT_KIND */
	TEST_NESTED_NLATTR_(fd, nlh0, hdrlen,
			    init_tcamsg_tab_item, print_tcamsg_tab_item,
			    TCA_ACT_KIND, XLAT_KNOWN(0x1, "TCA_ACT_KIND"),
			    21, pattern, 21, 2,
			    print_quoted_cstring(pattern, 22));
	tab_idx++;

	static const char kind[] = "Hello\tthere";
	TEST_NESTED_NLATTR_(fd, nlh0, hdrlen,
			    init_tcamsg_tab_item, print_tcamsg_tab_item,
			    TCA_ACT_KIND, XLAT_KNOWN(0x1, "TCA_ACT_KIND"),
			    sizeof(kind), kind, sizeof(kind), 2,
			    print_quoted_string(kind));
	tab_idx++;

	/* TCA_ROOT_TAB: item: TCA_ACT_INDEX */
	static uint32_t idx = 0xdeadc0de;
	TEST_NESTED_NLATTR_(fd, nlh0, hdrlen,
			    init_tcamsg_tab_item, print_tcamsg_tab_item,
			    TCA_ACT_INDEX, XLAT_KNOWN(0x3, "TCA_ACT_INDEX"),
			    sizeof(idx), &idx, sizeof(idx), 2,
			    printf("%u", idx));
	tab_idx++;

	/* TCA_ROOT_TAB: item: TCA_ACT_FLAGS */
	static uint32_t flags = 0xfacebeff;
	TEST_NESTED_NLATTR_(fd, nlh0, hdrlen,
			    init_tcamsg_tab_item, print_tcamsg_tab_item,
			    TCA_ACT_FLAGS, XLAT_KNOWN(0x7, "TCA_ACT_FLAGS"),
			    sizeof(flags), &flags, sizeof(flags), 2,
			    printf(XLAT_FMT, XLAT_SEL(0xfacebeff,
				   "TCA_ACT_FLAGS_NO_PERCPU_STATS|"
				   "TCA_ACT_FLAGS_SKIP_HW|"
				   "TCA_ACT_FLAGS_SKIP_SW|0xfacebef8")));
	tab_idx++;

	/* TCA_ROOT_TAB: item: TCA_ACT_HW_STATS, TCA_ACT_USED_HW_STATS */
	static const struct strval32 nla_hw_st[] = {
		{ ARG_XLAT_KNOWN(0x8, "TCA_ACT_HW_STATS") },
		{ ARG_XLAT_KNOWN(0x9, "TCA_ACT_USED_HW_STATS") },
	};

	static uint32_t hw_st = 0xfacebeef;
	for (size_t i = 0; i < ARRAY_SIZE(nla_hw_st); i++) {
		TEST_NESTED_NLATTR_(fd, nlh0, hdrlen,
				    init_tcamsg_tab_item, print_tcamsg_tab_item,
				    nla_hw_st[i].val, nla_hw_st[i].str,
				    sizeof(hw_st), &hw_st, sizeof(hw_st), 2,
				    printf(XLAT_FMT, XLAT_SEL(0xfacebeef,
					   "TCA_ACT_HW_STATS_IMMEDIATE|"
					   "TCA_ACT_HW_STATS_DELAYED|"
					   "0xfacebeec")));
		tab_idx++;
	}

	/* TCA_ROOT_TAB: item: TCA_ACT_IN_HW_COUNT */
	static uint32_t hw_count = 0xdeadface;
	TEST_NESTED_NLATTR_(fd, nlh0, hdrlen,
			    init_tcamsg_tab_item, print_tcamsg_tab_item,
			    TCA_ACT_IN_HW_COUNT,
			    XLAT_KNOWN(0xa, "TCA_ACT_IN_HW_COUNT"),
			    sizeof(hw_count), &hw_count, sizeof(hw_count), 2,
			    printf("%u", hw_count));

	/* TCA_ROOT_FLAGS */
	static const struct strval32 root_flags[] = {
		{ ARG_STR(0) },
		{ ARG_XLAT_KNOWN(0x1, "TCA_ACT_FLAG_LARGE_DUMP_ON") },
		{ ARG_XLAT_KNOWN(0x3, "TCA_ACT_FLAG_LARGE_DUMP_ON|"
				      "TCA_ACT_FLAG_TERSE_DUMP") },
		{ ARG_XLAT_KNOWN(0xcafebeef, "TCA_ACT_FLAG_LARGE_DUMP_ON|"
					     "TCA_ACT_FLAG_TERSE_DUMP|"
					     "0xcafebeec") },
		{ ARG_XLAT_UNKNOWN(0xbadc0dec, "TCA_ACT_FLAG_???") },
	};

	for (size_t i = 0; i < ARRAY_SIZE(root_flags); i++) {
		TEST_NLATTR_(fd, nlh0, hdrlen, init_tcamsg, print_tcamsg,
			     TCA_ROOT_FLAGS, XLAT_KNOWN(0x2, "TCA_ROOT_FLAGS"),
			     4, &root_flags[i].val, 4,
			     printf("%s", root_flags[i].str));
	}

	/* TCA_ROOT_COUNT */
	static const uint32_t cnt_vals[] = { 0, 1, 0xbac0ded };

	for (size_t i = 0; i < ARRAY_SIZE(cnt_vals); i++) {
		TEST_NLATTR_(fd, nlh0, hdrlen, init_tcamsg, print_tcamsg,
			     TCA_ROOT_COUNT, XLAT_KNOWN(0x3, "TCA_ROOT_COUNT"),
			     4, &cnt_vals[i], 4,
			     printf("%u", cnt_vals[i]));
	}

	/* TCA_ROOT_TIME_DELTA */
	static const struct strval32 time_deltas[] = {
		{ 0,		"0" },
		{ 1,		"1" NRAW(" /* 0.001 s */") },
		{ 10,		"10" NRAW(" /* 0.010 s */") },
		{ 100,		"100" NRAW(" /* 0.100 s */") },
		{ 999,		"999" NRAW(" /* 0.999 s */") },
		{ 1000,		"1000" NRAW(" /* 1.000 s */") },
		{ 1001,		"1001" NRAW(" /* 1.001 s */") },
		{ 1010,		"1010" NRAW(" /* 1.010 s */") },
		{ 0xfeedface,	"4277009102" NRAW(" /* 4277009.102 s */") },
	};

	for (size_t i = 0; i < ARRAY_SIZE(time_deltas); i++) {
		TEST_NLATTR_(fd, nlh0, hdrlen, init_tcamsg, print_tcamsg,
			     TCA_ROOT_TIME_DELTA,
			     XLAT_KNOWN(0x4, "TCA_ROOT_TIME_DELTA"),
			     4, &time_deltas[i].val, 4,
			     printf("%s", time_deltas[i].str));
	}

	static const struct strval64 time_deltas64[] = {
		{ 0,		"0" },
		{ 1,		"1" NRAW(" /* 0.001 s */") },
		{ 10,		"10" NRAW(" /* 0.010 s */") },
		{ 100,		"100" NRAW(" /* 0.100 s */") },
		{ 999,		"999" NRAW(" /* 0.999 s */") },
		{ 1000,		"1000" NRAW(" /* 1.000 s */") },
		{ 1001,		"1001" NRAW(" /* 1.001 s */") },
		{ 1010,		"1010" NRAW(" /* 1.010 s */") },
		{ 0xfeedface,	"4277009102" NRAW(" /* 4277009.102 s */") },
		{ 0xbadfacedeadc0ded, "13465671548708589037"
				      NRAW(" /* 13465671548708589.037 s */") },
	};

	for (size_t i = 0; i < ARRAY_SIZE(time_deltas64); i++) {
		TEST_NLATTR_(fd, nlh0, hdrlen, init_tcamsg, print_tcamsg,
			     TCA_ROOT_TIME_DELTA,
			     XLAT_KNOWN(0x4, "TCA_ROOT_TIME_DELTA"),
			     8, &time_deltas64[i].val, 8,
			     printf("%s", time_deltas64[i].str));
	}

	/* TCA_ROOT_EXT_WARN_MSG */
	static const char msg[] = "Hello\tthere";
	TEST_NLATTR_(fd, nlh0, hdrlen, init_tcamsg, print_tcamsg,
		     TCA_ROOT_EXT_WARN_MSG,
		     XLAT_KNOWN(0x5, "TCA_ROOT_EXT_WARN_MSG"),
		     sizeof(msg), msg, sizeof(msg),
		     print_quoted_string(msg));

	puts("+++ exited with 0 +++");
	return 0;
}
