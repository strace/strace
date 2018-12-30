/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include "test_nlattr.h"
#ifdef HAVE_LINUX_NEIGHBOUR_H
# include <linux/neighbour.h>
#endif
#include <linux/rtnetlink.h>

#define NDTA_PARMS 6
#define NDTPA_IFINDEX 1

static void
init_ndtmsg(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = msg_len,
		.nlmsg_type = RTM_GETNEIGHTBL,
		.nlmsg_flags = NLM_F_DUMP
	);

	struct ndtmsg *const msg = NLMSG_DATA(nlh);
	SET_STRUCT(struct ndtmsg, msg,
		.ndtm_family = AF_NETLINK
	);
}

static void
print_ndtmsg(const unsigned int msg_len)
{
	printf("{len=%u, type=RTM_GETNEIGHTBL, flags=NLM_F_DUMP"
	       ", seq=0, pid=0}, {ndtm_family=AF_NETLINK}",
	       msg_len);
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	const int fd = create_nl_socket(NETLINK_ROUTE);
	const unsigned int hdrlen = sizeof(struct ndtmsg);
	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen), NLA_HDRLEN + 11 * 8);

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

	const unsigned int nla_type = 0xffff & NLA_TYPE_MASK;
	char nla_type_str[256];
	sprintf(nla_type_str, "%#x /* NDTA_??? */", nla_type);
	TEST_NLATTR_(fd, nlh0, hdrlen,
		     init_ndtmsg, print_ndtmsg,
		     nla_type, nla_type_str,
		     4, pattern, 4,
		     print_quoted_hex(pattern, 4));

#ifdef HAVE_STRUCT_NDT_CONFIG
	static const struct ndt_config ndtc = {
		.ndtc_key_len = 0xabcd,
		.ndtc_entry_size = 0xbcda,
		.ndtc_entries = 0xcdabedad,
		.ndtc_last_flush = 0xdebaedba,
		.ndtc_last_rand = 0xedadedab,
		.ndtc_hash_rnd = 0xfeadedaf,
		.ndtc_hash_mask = 0xadbcdead,
		.ndtc_hash_chain_gc = 0xbdaedacd,
		.ndtc_proxy_qlen = 0xcdeaedab
	};
	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_ndtmsg, print_ndtmsg,
			   NDTA_CONFIG, pattern, ndtc,
			   PRINT_FIELD_U("{", ndtc, ndtc_key_len);
			   PRINT_FIELD_U(", ", ndtc, ndtc_entry_size);
			   PRINT_FIELD_U(", ", ndtc, ndtc_entries);
			   PRINT_FIELD_U(", ", ndtc, ndtc_last_flush);
			   PRINT_FIELD_U(", ", ndtc, ndtc_last_rand);
			   PRINT_FIELD_U(", ", ndtc, ndtc_hash_rnd);
			   PRINT_FIELD_0X(", ", ndtc, ndtc_hash_mask);
			   PRINT_FIELD_U(", ", ndtc, ndtc_hash_chain_gc);
			   PRINT_FIELD_U(", ", ndtc, ndtc_proxy_qlen);
			   printf("}"));
#endif /* HAVE_STRUCT_NDT_CONFIG */

	static const struct nlattr nla = {
		.nla_len = sizeof(nla),
		.nla_type = NDTPA_IFINDEX
	};
	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_ndtmsg, print_ndtmsg,
			   NDTA_PARMS, pattern, nla,
			   PRINT_FIELD_U("{", nla, nla_len);
			   printf(", nla_type=NDTPA_IFINDEX}"));

#ifdef HAVE_STRUCT_NDT_STATS
	static const struct ndt_stats ndtst = {
		.ndts_allocs		= 0xabcdedabedadedfa,
		.ndts_destroys		= 0xbcdefabefacdbaad,
		.ndts_hash_grows	= 0xcdbadefacdcbaede,
		.ndts_res_failed	= 0xdedbaecfdbcadcfe,
		.ndts_lookups		= 0xedfafdedbdadedec,
		.ndts_hits		= 0xfebdeadebcddeade,
		.ndts_rcv_probes_mcast	= 0xadebfeadecddeafe,
		.ndts_rcv_probes_ucast	= 0xbcdefeacdadecdfe,
		.ndts_periodic_gc_runs	= 0xedffeadedeffbecc,
		.ndts_forced_gc_runs	= 0xfeefefeabedeedcd,
# ifdef HAVE_STRUCT_NDT_STATS_NDTS_TABLE_FULLS
		.ndts_table_fulls	= 0xadebfefaecdfeade
# endif /* HAVE_STRUCT_NDT_STATS_NDTS_TABLE_FULLS */
	};
	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_ndtmsg, print_ndtmsg,
			   NDTA_STATS, pattern, ndtst,
			   PRINT_FIELD_U("{", ndtst, ndts_allocs);
			   PRINT_FIELD_U(", ", ndtst, ndts_destroys);
			   PRINT_FIELD_U(", ", ndtst, ndts_hash_grows);
			   PRINT_FIELD_U(", ", ndtst, ndts_res_failed);
			   PRINT_FIELD_U(", ", ndtst, ndts_lookups);
			   PRINT_FIELD_U(", ", ndtst, ndts_hits);
			   PRINT_FIELD_U(", ", ndtst, ndts_rcv_probes_mcast);
			   PRINT_FIELD_U(", ", ndtst, ndts_rcv_probes_ucast);
			   PRINT_FIELD_U(", ", ndtst, ndts_periodic_gc_runs);
			   PRINT_FIELD_U(", ", ndtst, ndts_forced_gc_runs);
# ifdef HAVE_STRUCT_NDT_STATS_NDTS_TABLE_FULLS
			   PRINT_FIELD_U(", ", ndtst, ndts_table_fulls);
# endif /* HAVE_STRUCT_NDT_STATS_NDTS_TABLE_FULLS */
			   printf("}"));
#endif /* HAVE_STRUCT_NDT_STATS */

	puts("+++ exited with 0 +++");
	return 0;
}
