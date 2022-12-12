/*
 * IFLA_INET6_* protocol-specific netlink attribute decoding check common code.
 *
 * Copyright (c) 2018-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef STRACE_TESTS_NLATTR_IFLA_AF_INET6
# define STRACE_TESTS_NLATTR_IFLA_AF_INET6

# include "tests.h"

static void
print_arr_val(uint32_t *val, size_t idx, const char *idx_str)
{
	if (idx_str)
		printf("[%s]=", idx_str);
	else
		printf("[%zu]=", idx);

	printf("%d", *val);
}

static void
print_arr_uval(uint64_t *val, size_t idx, const char *idx_str)
{
	if (idx_str)
		printf("[%s]=", idx_str);
	else
		printf("[%zu]=", idx);

	printf("%" PRIu64, *val);
}

static void
print_inet6_conf_val(uint32_t *val, size_t idx)
{
	static const char * const strs[] = {
		XLAT_KNOWN(0, "DEVCONF_FORWARDING"),
		XLAT_KNOWN(1, "DEVCONF_HOPLIMIT"),
	};

	print_arr_val(val, idx, idx < ARRAY_SIZE(strs) ? strs[idx] : NULL);
}

static void
print_inet6_stats_val(uint64_t *val, size_t idx)
{
	static const char * const strs[] = {
		XLAT_KNOWN(0, "IPSTATS_MIB_NUM"),
		XLAT_KNOWN(1, "IPSTATS_MIB_INPKTS"),
	};

	print_arr_uval(val, idx, idx < ARRAY_SIZE(strs) ? strs[idx] : NULL);
}

static void
print_icmp6_stats_val(uint64_t *val, size_t idx)
{
	static const char * const strs[] = {
		XLAT_KNOWN(0, "ICMP6_MIB_NUM"),
		XLAT_KNOWN(1, "ICMP6_MIB_INMSGS"),
		XLAT_KNOWN(2, "ICMP6_MIB_INERRORS"),
		XLAT_KNOWN(3, "ICMP6_MIB_OUTMSGS"),
		XLAT_KNOWN(4, "ICMP6_MIB_OUTERRORS"),
		XLAT_KNOWN(5, "ICMP6_MIB_CSUMERRORS"),
		XLAT_UNKNOWN(6, "ICMP6_MIB_???"),
	};

	print_arr_uval(val, idx, idx < ARRAY_SIZE(strs) ? strs[idx] : NULL);
}

static void
check_ifla_af_inet6(int fd, void *nlh0, size_t hdrlen,
		    void (*init_msg)(struct nlmsghdr *, unsigned int),
		    void (*print_msg)(unsigned int),
		    void *pattern, size_t depth)
{
	static const uint8_t unknown_msg[] = { 0xab, 0xac, 0xdb, 0xcd };

	/* Undecoded, unknown */
	TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen, init_msg, print_msg,
				      0, XLAT_KNOWN(0, "IFLA_INET6_UNSPEC"),
				      pattern, unknown_msg, print_quoted_hex,
				      depth,
				      printf("\"\\xab\\xac\\xdb\\xcd\""));
	TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen, init_msg, print_msg,
				      10, "0xa" NRAW(" /* IFLA_INET6_??? */"),
				      pattern, unknown_msg, print_quoted_hex,
				      depth,
				      printf("\"\\xab\\xac\\xdb\\xcd\""));


	/* IFLA_INET6_FLAGS */
	static const struct {
		uint32_t flags;
		const char *str;
	} inet6_flags[] = {
		{ 0xf, "0xf" NRAW(" /* IF_??? */") },
		{ ARG_XLAT_KNOWN(0x10, "IF_RS_SENT") },
		{ ARG_XLAT_KNOWN(0xc0, "IF_RA_MANAGED|IF_RA_OTHERCONF") },
		{ ARG_XLAT_KNOWN(0xdeadc0de, "IF_RS_SENT|IF_RA_MANAGED"
					     "|IF_RA_OTHERCONF|IF_READY"
					     "|0x5eadc00e") },
	};

	for (size_t i = 0; i < ARRAY_SIZE(inet6_flags); i++) {
		TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
					      init_msg, print_msg,
					      1, XLAT_KNOWN(0x1,
							    "IFLA_INET6_FLAGS"),
					      pattern, inet6_flags[i].flags,
					      print_quoted_hex, depth,
					      printf("%s", inet6_flags[i].str));
	}


	/* IFLA_INET6_CONF */
	uint32_t inet6_conf_vals[] = { 0xdeadc0de, 0xda7aface };
	TEST_NESTED_NLATTR_ARRAY_EX_(fd, nlh0, hdrlen, init_msg, print_msg,
				     IFLA_INET6_CONF,
				     XLAT_KNOWN(0x2, "IFLA_INET6_CONF"),
				     pattern, inet6_conf_vals, depth,
				     print_inet6_conf_val);


	/* IFLA_INET6_STATS */
	uint64_t inet6_stats_vals[] = { 0xdeadc0deda7aface, 0xdec0deedbadc0ded };
	TEST_NESTED_NLATTR_ARRAY_EX_(fd, nlh0, hdrlen, init_msg, print_msg,
				    IFLA_INET6_STATS,
				    XLAT_KNOWN(0x3, "IFLA_INET6_STATS"),
				    pattern, inet6_stats_vals, depth,
				    print_inet6_stats_val);


	/* IFLA_INET6_MCAST */
	TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen, init_msg, print_msg,
				      4, XLAT_KNOWN(0x4, "IFLA_INET6_MCAST"),
				      pattern, unknown_msg, print_quoted_hex,
				      depth,
				      printf("\"\\xab\\xac\\xdb\\xcd\""));


	/* IFLA_INET6_CACHEINFO */
	static const struct ifla_cacheinfo ci = {
		0xbadc0ded, 0xfacebeef, 0xdecafeed, 0xdeadfeed,
	};
	TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen, init_msg, print_msg, 5,
				      XLAT_KNOWN(0x5, "IFLA_INET6_CACHEINFO"),
				      pattern, ci, print_quoted_hex, depth,
				      printf("{");
				      PRINT_FIELD_U(ci, max_reasm_len);
				      printf(", ");
				      PRINT_FIELD_U(ci, tstamp);
				      printf(", ");
				      PRINT_FIELD_U(ci, reachable_time);
				      printf(", ");
				      PRINT_FIELD_U(ci, retrans_time);
				      printf("}"));


	/* IFLA_INET6_ICMP6STATS */
	uint64_t icmp6_stats_vals[] = {
		0xdeadc0deda7aface, 0xdec0deedbadc0ded, 0xfacebeefdeadfeed,
		0xdeadc0deda7afacd, 0xdec0deedbadc0dee, 0xfacebeefdeadfeef,
		0xdeadc0deda7afacc
	};
	TEST_NESTED_NLATTR_ARRAY_EX_(fd, nlh0, hdrlen, init_msg, print_msg,
				     IFLA_INET6_ICMP6STATS,
				     XLAT_KNOWN(0x6, "IFLA_INET6_ICMP6STATS"),
				     pattern, icmp6_stats_vals, depth,
				     print_icmp6_stats_val);


	/* IFLA_INET6_TOKEN */
	uint8_t inet6_addr[16] = {
		0xba, 0xdc, 0x0d, 0xed, 0xfa, 0xce, 0xbe, 0xef,
		0xde, 0xca, 0xfe, 0xed, 0xde, 0xad, 0xfe, 0xed,
	};
	TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen, init_msg, print_msg,
				      7, XLAT_KNOWN(0x7, "IFLA_INET6_TOKEN"),
				      pattern, inet6_addr, print_quoted_hex,
				      depth,
				      printf(XLAT_KNOWN_FMT("%s", "%s"),
					     XLAT_SEL("\"\\xba\\xdc\\x0d\\xed"
					     "\\xfa\\xce\\xbe\\xef"
					     "\\xde\\xca\\xfe\\xed"
					     "\\xde\\xad\\xfe\\xed\"",
					     "inet_pton(AF_INET6"
					     ", \"badc:ded:face:beef"
					     ":deca:feed:dead:feed\")")));


	/* IFLA_INET6_ADDR_GEN_MODE */
	static const struct {
		uint8_t flags;
		const char *str;
	} agms[] = {
		{ ARG_XLAT_KNOWN(0, "IN6_ADDR_GEN_MODE_EUI64") },
		{ ARG_XLAT_KNOWN(0x3, "IN6_ADDR_GEN_MODE_RANDOM") },
		{ ARG_XLAT_UNKNOWN(0x4, "IN6_ADDR_GEN_MODE_???") },
		{ ARG_XLAT_UNKNOWN(0xff, "IN6_ADDR_GEN_MODE_???") },
	};

	for (size_t i = 0; i < ARRAY_SIZE(agms); i++) {
		TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
					      init_msg, print_msg, 8,
					      XLAT_KNOWN(0x8, "IFLA_INET6_ADDR_GEN_MODE"),
					      pattern, agms[i].flags,
					      print_quoted_hex, depth,
					      printf("%s", agms[i].str));
	}


	/* IFLA_INET6_RA_MTU */
	static const uint32_t ra_mtu = 0xdeadc0de;
	TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen, init_msg, print_msg,
				      9, XLAT_KNOWN(0x9, "IFLA_INET6_RA_MTU"),
				      pattern, ra_mtu, print_quoted_hex, depth,
				      printf("3735929054"));
}

#endif /* STRACE_TESTS_NLATTR_IFLA_AF_INET6 */
