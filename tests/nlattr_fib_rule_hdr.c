/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#ifdef HAVE_LINUX_FIB_RULES_H

# include <stdio.h>
# include <inttypes.h>
# include "test_nlattr.h"
# include <linux/fib_rules.h>
# include <linux/in.h>
# include <linux/ip.h>
# include <linux/rtnetlink.h>

# define FRA_TUN_ID 12
# define FRA_TABLE 15
# define FRA_UID_RANGE 20
# define FRA_PROTOCOL 21
# define FRA_IP_PROTO 22
# define FRA_SPORT_RANGE 23
# define FRA_DPORT_RANGE 24

# ifndef HAVE_STRUCT_FIB_RULE_PORT_RANGE
struct fib_rule_port_range {
	uint16_t start;
	uint16_t end;
};
# endif /* HAVE_STRUCT_FIB_RULE_PORT_RANGE */

static void
init_rtmsg(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = msg_len,
		.nlmsg_type = RTM_GETRULE,
		.nlmsg_flags = NLM_F_DUMP
	);

	struct rtmsg *const msg = NLMSG_DATA(nlh);
	SET_STRUCT(struct rtmsg, msg,
		.rtm_family = AF_UNIX,
		.rtm_tos = IPTOS_LOWDELAY,
		.rtm_table = RT_TABLE_UNSPEC,
		.rtm_type = FR_ACT_TO_TBL,
		.rtm_flags = FIB_RULE_INVERT
	);
}

static void
print_rtmsg(const unsigned int msg_len)
{
	printf("{len=%u, type=RTM_GETRULE, flags=NLM_F_DUMP"
	       ", seq=0, pid=0}, {family=AF_UNIX"
	       ", dst_len=0, src_len=0"
	       ", tos=IPTOS_LOWDELAY"
	       ", table=RT_TABLE_UNSPEC"
	       ", action=FR_ACT_TO_TBL"
	       ", flags=FIB_RULE_INVERT}",
	       msg_len);
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	const int fd = create_nl_socket(NETLINK_ROUTE);
	const unsigned int hdrlen = sizeof(struct rtmsg);
	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen), NLA_HDRLEN + 8);

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

	const unsigned int nla_type = 0xffff & NLA_TYPE_MASK;
	char nla_type_str[256];
	sprintf(nla_type_str, "%#x /* FRA_??? */", nla_type);
	TEST_NLATTR_(fd, nlh0, hdrlen,
		     init_rtmsg, print_rtmsg,
		     nla_type, nla_type_str,
		     4, pattern, 4,
		     print_quoted_hex(pattern, 4));

	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_rtmsg, print_rtmsg,
		    FRA_DST, 4, pattern, 4,
		    print_quoted_hex(pattern, 4));

	const uint32_t table_id = RT_TABLE_DEFAULT;
	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_rtmsg, print_rtmsg,
			   FRA_TABLE, pattern, table_id,
			   printf("RT_TABLE_DEFAULT"));

# ifdef HAVE_STRUCT_FIB_RULE_UID_RANGE
	static const struct fib_rule_uid_range range = {
		.start = 0xabcdedad,
		.end = 0xbcdeadba
	};
	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_rtmsg, print_rtmsg,
			   FRA_UID_RANGE, pattern, range,
			   PRINT_FIELD_U("{", range, start);
			   PRINT_FIELD_U(", ", range, end);
			   printf("}"));
# endif
# if defined HAVE_BE64TOH || defined be64toh
	const uint64_t tun_id = 0xabcdcdbeedabadef;
	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_rtmsg, print_rtmsg,
			   FRA_TUN_ID, pattern, tun_id,
			   printf("htobe64(%" PRIu64 ")", be64toh(tun_id)));
# endif

	uint8_t proto;

	static const struct {
		uint8_t arg;
		const char *str;
	} proto_args[] = {
		{ ARG_STR(RTPROT_UNSPEC) },
		{ 5, "0x5 /* RTPROT_??? */" },
		{ 17, "RTPROT_MROUTED" },
		{ 42, "RTPROT_BABEL" },
		{ 43, "0x2b /* RTPROT_??? */" },
		{ ARG_STR(0xde) " /* RTPROT_??? */" },
	};

	for (unsigned i = 0; i < ARRAY_SIZE(proto_args); i++) {
		proto = proto_args[i].arg;
		TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
				   init_rtmsg, print_rtmsg,
				   FRA_PROTOCOL, pattern, proto,
				   printf("%s", proto_args[i].str));
	}

	static const struct {
		uint8_t arg;
		const char *str;
	} ipproto_args[] = {
		{ ARG_STR(IPPROTO_TCP) },
		{ 254, "0xfe /* IPPROTO_??? */" },
		{ ARG_STR(IPPROTO_RAW) },
	};

	for (unsigned i = 0; i < ARRAY_SIZE(ipproto_args); i++) {
		proto = ipproto_args[i].arg;
		TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
				   init_rtmsg, print_rtmsg,
				   FRA_IP_PROTO, pattern, proto,
				   printf("%s", ipproto_args[i].str));
	}

	static const struct fib_rule_port_range prange = {
		.start = 0xabcd,
		.end = 0xfeed,
	};
	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_rtmsg, print_rtmsg,
			   FRA_SPORT_RANGE, pattern, prange,
			   PRINT_FIELD_U("{", prange, start);
			   PRINT_FIELD_U(", ", prange, end);
			   printf("}"));
	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_rtmsg, print_rtmsg,
			   FRA_DPORT_RANGE, pattern, prange,
			   PRINT_FIELD_U("{", prange, start);
			   PRINT_FIELD_U(", ", prange, end);
			   printf("}"));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_LINUX_FIB_RULES_H")

#endif
