/*
 * IFLA_AF_SPEC netlink attribute decoding check.
 *
 * Copyright (c) 2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <inttypes.h>
#include <stdio.h>
#include <stddef.h>
#include "test_nlattr.h"

#include <linux/if.h>
#include <linux/if_arp.h>
#ifdef HAVE_LINUX_IF_LINK_H
# include <linux/if_link.h>
#endif
#include <linux/rtnetlink.h>

#if !HAVE_DECL_IFLA_AF_SPEC
enum { IFLA_AF_SPEC = 26 };
#endif

#define XLAT_MACROS_ONLY
#include "xlat/rtnl_ifla_af_spec_inet_attrs.h"
#include "xlat/rtnl_ifla_af_spec_inet6_attrs.h"
#undef XLAT_MACROS_ONLY

#ifndef HAVE_STRUCT_IFLA_CACHEINFO
struct ifla_cacheinfo {
	uint32_t max_reasm_len;
	uint32_t tstamp;
	uint32_t reachable_time;
	uint32_t retrans_time;
};
#endif

#define IFLA_ATTR IFLA_AF_SPEC
#include "nlattr_ifla.h"

#define AF_SPEC_FUNCS(family_)						\
	static void							\
	init_##family_##_msg(struct nlmsghdr *const nlh,		\
			     const unsigned int msg_len)		\
	{								\
		init_ifinfomsg(nlh, msg_len);				\
									\
		struct nlattr *nla = NLMSG_ATTR(nlh, hdrlen);		\
		nla += 1;						\
		SET_STRUCT(struct nlattr, nla,				\
			.nla_len = msg_len - NLMSG_SPACE(hdrlen)	\
				  - NLA_HDRLEN,				\
			.nla_type = family_,				\
		);							\
	}								\
									\
	static void							\
	print_##family_##_msg(const unsigned int msg_len)		\
	{								\
		print_ifinfomsg(msg_len);				\
		printf(", {{nla_len=%u, nla_type=" #family_ "}",	\
		       msg_len - NLMSG_SPACE(hdrlen) - NLA_HDRLEN);	\
	}								\
	/* end of AF_SPEC_FUNCS definition */

AF_SPEC_FUNCS(AF_INET)
AF_SPEC_FUNCS(AF_INET6)

static void
print_arr_val(uint32_t *val, size_t idx, const char *idx_str)
{
	if (idx_str)
		printf("[%s] = ", idx_str);
	else
		printf("[%zu] = ", idx);

	printf("%d", *val);
}

static void
print_arr_uval(uint64_t *val, size_t idx, const char *idx_str)
{
	if (idx_str)
		printf("[%s] = ", idx_str);
	else
		printf("[%zu] = ", idx);

	printf("%" PRIu64, *val);
}

static void
print_inet_conf_val(uint32_t *val, size_t idx)
{
	static const char * const strs[] = {
		"IPV4_DEVCONF_FORWARDING-1",
		"IPV4_DEVCONF_MC_FORWARDING-1",
	};

	print_arr_val(val, idx, idx < ARRAY_SIZE(strs) ? strs[idx] : NULL);
}


static void
print_inet6_conf_val(uint32_t *val, size_t idx)
{
	static const char * const strs[] = {
		"DEVCONF_FORWARDING",
		"DEVCONF_HOPLIMIT",
	};

	print_arr_val(val, idx, idx < ARRAY_SIZE(strs) ? strs[idx] : NULL);
}

static void
print_inet6_stats_val(uint64_t *val, size_t idx)
{
	static const char * const strs[] = {
		"IPSTATS_MIB_NUM",
		"IPSTATS_MIB_INPKTS",
	};

	print_arr_uval(val, idx, idx < ARRAY_SIZE(strs) ? strs[idx] : NULL);
}

static void
print_icmp6_stats_val(uint64_t *val, size_t idx)
{
	static const char * const strs[] = {
		"ICMP6_MIB_NUM",
		"ICMP6_MIB_INMSGS",
		"ICMP6_MIB_INERRORS",
		"ICMP6_MIB_OUTMSGS",
		"ICMP6_MIB_OUTERRORS",
		"ICMP6_MIB_CSUMERRORS",
		"6 /* ICMP6_MIB_??? */",
	};

	print_arr_uval(val, idx, idx < ARRAY_SIZE(strs) ? strs[idx] : NULL);
}

int
main(void)
{
	static const uint8_t unknown_msg[] = { 0xab, 0xac, 0xdb, 0xcd };

	skip_if_unavailable("/proc/self/fd/");

	const int fd = create_nl_socket(NETLINK_ROUTE);

	const unsigned int hdrlen = sizeof(struct ifinfomsg);
	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen), 3 * NLA_HDRLEN + 256);

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);


	/* unknown AF_* */
	TEST_NESTED_NLATTR_OBJECT(fd, nlh0, hdrlen,
				  init_ifinfomsg, print_ifinfomsg,
				  AF_UNIX, pattern, unknown_msg,
				  printf("\"\\xab\\xac\\xdb\\xcd\""));

	/* AF_INET */
	TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
				      init_AF_INET_msg, print_AF_INET_msg,
				      0, "IFLA_INET_UNSPEC", pattern,
				      unknown_msg, print_quoted_hex, 2,
				      printf("\"\\xab\\xac\\xdb\\xcd\""));
	TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
				      init_AF_INET_msg, print_AF_INET_msg,
				      2, "0x2 /* IFLA_INET_??? */", pattern,
				      unknown_msg, print_quoted_hex, 2,
				      printf("\"\\xab\\xac\\xdb\\xcd\""));

	/* AF_INET: IFLA_INET_CONF */
	uint32_t inet_conf_vals[] = { 0xdeadc0de, 0xda7aface };
	TEST_NESTED_NLATTR_ARRAY_EX(fd, nlh0, hdrlen,
				    init_AF_INET_msg, print_AF_INET_msg,
				    IFLA_INET_CONF, pattern,
				    inet_conf_vals, 2, print_inet_conf_val);

	/* AF_INET6 */
	TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
				      init_AF_INET6_msg, print_AF_INET6_msg,
				      0, "IFLA_INET6_UNSPEC", pattern,
				      unknown_msg, print_quoted_hex, 2,
				      printf("\"\\xab\\xac\\xdb\\xcd\""));
	TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
				      init_AF_INET6_msg, print_AF_INET6_msg,
				      9, "0x9 /* IFLA_INET6_??? */", pattern,
				      unknown_msg, print_quoted_hex, 2,
				      printf("\"\\xab\\xac\\xdb\\xcd\""));

	/* AF_INET6: IFLA_INET6_FLAGS */
	static const struct {
		uint32_t flags;
		const char *str;
	} inet6_flags[] = {
		{ 0xf, "0xf /* IF_??? */" },
		{ 0x10, "IF_RS_SENT" },
		{ 0xc0, "IF_RA_MANAGED|IF_RA_OTHERCONF" },
		{ 0xdeadc0de, "IF_RS_SENT|IF_RA_MANAGED|IF_RA_OTHERCONF"
			      "|IF_READY|0x5eadc00e" },
	};

	for (size_t i = 0; i < ARRAY_SIZE(inet6_flags); i++) {
		TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
					      init_AF_INET6_msg,
					      print_AF_INET6_msg,
					      1, "IFLA_INET6_FLAGS", pattern,
					      inet6_flags[i].flags,
					      print_quoted_hex, 2,
					      printf("%s", inet6_flags[i].str));
	}

	/* AF_INET6: IFLA_INET6_CONF */
	uint32_t inet6_conf_vals[] = { 0xdeadc0de, 0xda7aface };
	TEST_NESTED_NLATTR_ARRAY_EX(fd, nlh0, hdrlen,
				    init_AF_INET6_msg, print_AF_INET6_msg,
				    IFLA_INET6_CONF, pattern,
				    inet6_conf_vals, 2, print_inet6_conf_val);

	/* AF_INET6: IFLA_INET6_STATS */
	uint64_t inet6_stats_vals[] = { 0xdeadc0deda7aface, 0xdec0deedbadc0ded };
	TEST_NESTED_NLATTR_ARRAY_EX(fd, nlh0, hdrlen,
				    init_AF_INET6_msg, print_AF_INET6_msg,
				    IFLA_INET6_STATS, pattern,
				    inet6_stats_vals, 2, print_inet6_stats_val);

	/* AF_INET6: IFLA_INET6_MCAST */
	TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
				      init_AF_INET6_msg, print_AF_INET6_msg,
				      4, "IFLA_INET6_MCAST", pattern,
				      unknown_msg, print_quoted_hex, 2,
				      printf("\"\\xab\\xac\\xdb\\xcd\""));

	/* AF_INET6: IFLA_INET6_CACHEINFO */
	static const struct ifla_cacheinfo ci = {
		0xbadc0ded, 0xfacebeef, 0xdecafeed, 0xdeadfeed,
	};
	TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
				      init_AF_INET6_msg, print_AF_INET6_msg,
				      5, "IFLA_INET6_CACHEINFO", pattern,
				      ci, print_quoted_hex, 2,
				      PRINT_FIELD_U("{", ci, max_reasm_len);
				      PRINT_FIELD_U(", ", ci, tstamp);
				      PRINT_FIELD_U(", ", ci, reachable_time);
				      PRINT_FIELD_U(", ", ci, retrans_time);
				      printf("}"));

	/* AF_INET6: IFLA_INET6_ICMP6STATS */
	uint64_t icmp6_stats_vals[] = {
		0xdeadc0deda7aface, 0xdec0deedbadc0ded, 0xfacebeefdeadfeed,
		0xdeadc0deda7afacd, 0xdec0deedbadc0dee, 0xfacebeefdeadfeef,
		0xdeadc0deda7afacc
	};
	TEST_NESTED_NLATTR_ARRAY_EX(fd, nlh0, hdrlen,
				    init_AF_INET6_msg, print_AF_INET6_msg,
				    IFLA_INET6_ICMP6STATS, pattern,
				    icmp6_stats_vals, 2, print_icmp6_stats_val);

	/* AF_INET6: IFLA_INET6_TOKEN */
	uint8_t inet6_addr[16] = {
		0xba, 0xdc, 0x0d, 0xed, 0xfa, 0xce, 0xbe, 0xef,
		0xde, 0xca, 0xfe, 0xed, 0xde, 0xad, 0xfe, 0xed,
	};
	TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
				      init_AF_INET6_msg, print_AF_INET6_msg,
				      7, "IFLA_INET6_TOKEN", pattern,
				      inet6_addr, print_quoted_hex, 2,
				      printf("inet_pton(AF_INET6"
					     ", \"badc:ded:face:beef"
					     ":deca:feed:dead:feed\")"));

	/* AF_INET6: IFLA_INET6_ */
	static const struct {
		uint8_t flags;
		const char *str;
	} agms[] = {
		{ 0x0, "IN6_ADDR_GEN_MODE_EUI64" },
		{ 0x3, "IN6_ADDR_GEN_MODE_RANDOM" },
		{ 0x4, "0x4 /* IN6_ADDR_GEN_MODE_??? */" },
		{ 0xff, "0xff /* IN6_ADDR_GEN_MODE_??? */" },
	};

	for (size_t i = 0; i < ARRAY_SIZE(agms); i++) {
		TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
					      init_AF_INET6_msg,
					      print_AF_INET6_msg,
					      8, "IFLA_INET6_ADDR_GEN_MODE",
					      pattern, agms[i].flags,
					      print_quoted_hex, 2,
					      printf("%s", agms[i].str));
	}


	puts("+++ exited with 0 +++");
	return 0;
}
