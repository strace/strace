/*
 * Copyright (c) 2018-2021 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2018-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <arpa/inet.h>
#include <inttypes.h>
#include <linux/ip.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "test_nlattr.h"

#include <linux/if_link.h>
#include <linux/if_bonding.h>
#include <linux/if_bridge.h>
#include <linux/mpls.h>

#include "xlat.h"
#include "xlat/addrfams.h"
#define XLAT_MACROS_ONLY
# include "xlat/ifstats_af_spec_mpls_attrs.h"
# include "xlat/ifstats_attrs.h"
# include "xlat/ifstats_attr_flags.h"
# include "xlat/ifstats_offload_attrs.h"
# include "xlat/ifstats_xstats_bond_attrs.h"
# include "xlat/ifstats_xstats_bond_3ad_attrs.h"
# include "xlat/ifstats_xstats_bridge_attrs.h"
# include "xlat/ifstats_xstats_bridge_mcast_indices.h"
# include "xlat/ifstats_xstats_type_attrs.h"
# include "xlat/nl_bridge_vlan_flags.h"
#undef XLAT_MACROS_ONLY

static const unsigned int hdrlen = sizeof(struct if_stats_msg);
static char pattern[4096];
static char nla_type_str[256];

static void
init_ifstats(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = msg_len,
		.nlmsg_type = RTM_GETSTATS,
		.nlmsg_flags = NLM_F_DUMP,
	);

	struct if_stats_msg *const msg = NLMSG_DATA(nlh);
	SET_STRUCT(struct if_stats_msg, msg,
		.family = AF_UNIX,
		.ifindex = ifindex_lo(),
		.filter_mask = 0x22,
	);
}

static void
print_ifstats(const unsigned int msg_len)
{
	printf("{nlmsg_len=%u, nlmsg_type=" XLAT_FMT ", nlmsg_flags=" XLAT_FMT
	       ", nlmsg_seq=0, nlmsg_pid=0}, {family=" XLAT_FMT ", ifindex="
	       XLAT_FMT_U ", filter_mask=" XLAT_FMT "}",
	       msg_len, XLAT_ARGS(RTM_GETSTATS), XLAT_ARGS(NLM_F_DUMP),
	       XLAT_ARGS(AF_UNIX), XLAT_SEL(ifindex_lo(), IFINDEX_LO_STR),
	       XLAT_ARGS(1<<IFLA_STATS_LINK_64|1<<IFLA_STATS_AF_SPEC));
}

/*
 * NB: these functions use global variables to control which top-level
 * netlink attribute is to be used.
 */
#define DEF_NLATTR_FUNCS_NESTED(sfx_, attr_var_, attr_str_var_,		\
				parent_sfx_, lvl_)			\
	static void							\
	init_ifstats_##sfx_(struct nlmsghdr *const nlh,			\
			    const unsigned int msg_len)			\
	{								\
		(init_##parent_sfx_)(nlh, msg_len);			\
		struct nlattr *nla = NLMSG_ATTR(nlh,			\
						sizeof(struct if_stats_msg) \
						+ ((lvl_) - 1) * NLA_HDRLEN); \
		SET_STRUCT(struct nlattr, nla,				\
			.nla_len = msg_len				\
				   - NLMSG_SPACE(sizeof(struct if_stats_msg)) \
				   - ((lvl_) - 1) * NLA_HDRLEN,	\
			.nla_type = (attr_var_),			\
		);							\
	}								\
									\
	static void							\
	print_ifstats_##sfx_(const unsigned int msg_len)		\
	{								\
		(print_##parent_sfx_)(msg_len);				\
		printf(", [{nla_len=%u, nla_type=%s}",			\
		       (unsigned int) (msg_len - NLMSG_HDRLEN		\
				       - NLMSG_ALIGN(sizeof(struct	\
							    if_stats_msg)) \
				       - ((lvl_) - 1) * NLA_HDRLEN),	\
		       attr_str_var_);					\
	}								\
	/* End of DEF_NLATTR_FUNCS_NESTED */

static uint16_t l1_attr;
static char l1_attr_str[256];

static uint16_t l2_attr;
static char l2_attr_str[256];

static uint16_t l3_attr;
static char l3_attr_str[256];

DEF_NLATTR_FUNCS_NESTED(l1, l1_attr, l1_attr_str, ifstats, 1)
DEF_NLATTR_FUNCS_NESTED(l2, l2_attr, l2_attr_str, ifstats_l1, 2)
DEF_NLATTR_FUNCS_NESTED(l3, l3_attr, l3_attr_str, ifstats_l2, 3)


static void
print_stats_64(struct rtnl_link_stats64 *st, size_t sz)
{
	printf("{");  PRINT_FIELD_U(*st, rx_packets);
	printf(", "); PRINT_FIELD_U(*st, tx_packets);
	printf(", "); PRINT_FIELD_U(*st, rx_bytes);
	printf(", "); PRINT_FIELD_U(*st, tx_bytes);
	printf(", "); PRINT_FIELD_U(*st, rx_errors);
	printf(", "); PRINT_FIELD_U(*st, tx_errors);
	printf(", "); PRINT_FIELD_U(*st, rx_dropped);
	printf(", "); PRINT_FIELD_U(*st, tx_dropped);
	printf(", "); PRINT_FIELD_U(*st, multicast);
	printf(", "); PRINT_FIELD_U(*st, collisions);
	printf(", "); PRINT_FIELD_U(*st, rx_length_errors);
	printf(", "); PRINT_FIELD_U(*st, rx_over_errors);
	printf(", "); PRINT_FIELD_U(*st, rx_crc_errors);
	printf(", "); PRINT_FIELD_U(*st, rx_frame_errors);
	printf(", "); PRINT_FIELD_U(*st, rx_fifo_errors);
	printf(", "); PRINT_FIELD_U(*st, rx_missed_errors);
	printf(", "); PRINT_FIELD_U(*st, tx_aborted_errors);
	printf(", "); PRINT_FIELD_U(*st, tx_carrier_errors);
	printf(", "); PRINT_FIELD_U(*st, tx_fifo_errors);
	printf(", "); PRINT_FIELD_U(*st, tx_heartbeat_errors);
	printf(", "); PRINT_FIELD_U(*st, tx_window_errors);
	printf(", "); PRINT_FIELD_U(*st, rx_compressed);
	printf(", "); PRINT_FIELD_U(*st, tx_compressed);
	if (sz >= offsetofend(struct rtnl_link_stats64, rx_nohandler)) {
		printf(", ");
		PRINT_FIELD_U(*st, rx_nohandler);
	}
	if (sz >= offsetofend(struct rtnl_link_stats64, rx_otherhost_dropped)) {
		printf(", ");
		PRINT_FIELD_U(*st, rx_otherhost_dropped);
	}
	printf("}");
}

static void
check_stats_64(const int fd, unsigned int cmd, const char *cmd_str, bool nest)
{
	static const size_t minsz = offsetofend(struct rtnl_link_stats64,
						tx_compressed);

	struct rtnl_link_stats64 st;
	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen),
				   (!!nest + 1) * NLA_HDRLEN + sizeof(st));

	snprintf(nla_type_str, sizeof(nla_type_str), XLAT_FMT,
		 XLAT_SEL(cmd, cmd_str));
	fill_memory(&st, sizeof(st));

	TEST_NESTED_NLATTR_OBJECT_EX_MINSZ_(fd, nlh0, hdrlen,
					    nest ? init_ifstats_l1
						 : init_ifstats,
					    nest ? print_ifstats_l1
						 : print_ifstats,
					    cmd, nla_type_str,
					    pattern, st, minsz,
					    print_quoted_hex, (unsigned) !!nest,
					    print_stats_64(&st, sizeof(st)));

	TEST_NLATTR_(fd, nlh0 - !!nest * NLA_HDRLEN,
		     hdrlen + !!nest * NLA_HDRLEN,
		     nest ? init_ifstats_l1 : init_ifstats,
		     nest ? print_ifstats_l1 : print_ifstats,
		     cmd, nla_type_str, minsz, &st, minsz,
		     print_stats_64(&st, minsz);
		     for (size_t i = 0; i < (unsigned) !!nest; i++)
			     printf("]"));
}

static void
fmt_str(char *dst, size_t dst_sz, uint32_t cmd, const char *s, const char *dflt)
{
	if (s) {
		snprintf(dst, dst_sz, XLAT_FMT, XLAT_SEL(cmd, s));
	} else {
		snprintf(dst, dst_sz, "%#x" NRAW(" /* %s */"),
			 cmd NRAW(, dflt));
	}
}

static void
print_mcast_stats(struct br_mcast_stats *br_xst_mc)
{
#define PR_FIELD_(pfx_, field_) \
	printf(pfx_ #field_ "=[[" XLAT_KNOWN(0, "BR_MCAST_DIR_RX")	\
	       "]=%llu, [" XLAT_KNOWN(1, "BR_MCAST_DIR_TX") "]=%llu]", \
	       (unsigned long long) br_xst_mc->field_[0],		\
	       (unsigned long long) br_xst_mc->field_[1])

	PR_FIELD_("{",  igmp_v1queries);
	PR_FIELD_(", ", igmp_v2queries);
	PR_FIELD_(", ", igmp_v3queries);
	PR_FIELD_(", ", igmp_leaves);
	PR_FIELD_(", ", igmp_v1reports);
	PR_FIELD_(", ", igmp_v2reports);
	PR_FIELD_(", ", igmp_v3reports);
	printf(", igmp_parse_errors=%llu",
	       (unsigned long long) br_xst_mc->igmp_parse_errors);
	PR_FIELD_(", ", mld_v1queries);
	PR_FIELD_(", ", mld_v2queries);
	PR_FIELD_(", ", mld_leaves);
	PR_FIELD_(", ", mld_v1reports);
	PR_FIELD_(", ", mld_v2reports);
	printf(", mld_parse_errors=%llu",
	       (unsigned long long) br_xst_mc->mld_parse_errors);
	PR_FIELD_(", ", mcast_bytes);
	PR_FIELD_(", ", mcast_packets);
	printf("}");

#undef PR_FIELD_
}

static void
check_xstats(const int fd, unsigned int cmd, const char *cmd_str)
{
	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen) + 3 * NLA_HDRLEN,
				   NLA_HDRLEN + 256);

	l1_attr = cmd;
	snprintf(l1_attr_str, sizeof(l1_attr_str), XLAT_FMT,
		 XLAT_SEL(cmd, cmd_str));

	/* Unknown, unimplemented, no semantics.  */
	static const struct strval32 undec_types[] = {
		{ ARG_STR(LINK_XSTATS_TYPE_UNSPEC) },
		{ 0x3 },
		{ 0xbad },
	};
	for (size_t i = 0; i < ARRAY_SIZE(undec_types); i++) {
		fmt_str(nla_type_str, sizeof(nla_type_str),
			undec_types[i].val, undec_types[i].str,
			"LINK_XSTATS_TYPE_???");
		TEST_NLATTR_(fd, nlh0, hdrlen + NLA_HDRLEN,
			     init_ifstats_l1, print_ifstats_l1,
			     undec_types[i].val, nla_type_str,
			     37, pattern, 37,
			     print_quoted_hex(pattern, 32);
			     printf("...]"));
	}

	/* LINK_XSTATS_TYPE_BRIDGE */
	l2_attr = LINK_XSTATS_TYPE_BRIDGE;
	snprintf(l2_attr_str, sizeof(l2_attr_str), XLAT_FMT,
		 XLAT_ARGS(LINK_XSTATS_TYPE_BRIDGE));

	/* LINK_XSTATS_TYPE_BRIDGE: Unknown, unimplemented, no semantics.  */
	static const struct strval32 undec_br_types[] = {
		{ ARG_STR(BRIDGE_XSTATS_UNSPEC) },
		{ ARG_STR(BRIDGE_XSTATS_PAD) },
		{ 0x5 },
		{ 0xbad },
	};
	for (size_t i = 0; i < ARRAY_SIZE(undec_br_types); i++) {
		fmt_str(nla_type_str, sizeof(nla_type_str),
			undec_br_types[i].val, undec_br_types[i].str,
			"BRIDGE_XSTATS_???");
		TEST_NLATTR_(fd, nlh0, hdrlen + 2 * NLA_HDRLEN,
			     init_ifstats_l2, print_ifstats_l2,
			     undec_br_types[i].val, nla_type_str,
			     37, pattern, 37,
			     print_quoted_hex(pattern, 32);
			     printf("...]]"));
	}

	/* LINK_XSTATS_TYPE_BRIDGE: BRIDGE_XSTATS_VLAN */
	static const struct {
		struct bridge_vlan_xstats val;
		const char *str;
	} br_xst_vlan_vecs[] = {
		{ { .rx_bytes=0, .rx_packets=0xdeadfacebeeffeedULL,
		    .tx_bytes=0x8090a0b0c0d0e0f0ULL, .tx_packets=0,
		    .vid=0xdead, .flags=2 },
		  "{rx_bytes=0, rx_packets=16045756813264551661"
		  ", tx_bytes=9264081114510713072, tx_packets=0"
		  ", vid=57005, flags="
		  XLAT_KNOWN(0x2, "BRIDGE_VLAN_INFO_PVID") "}" },
		{ { .rx_bytes=12345678901234567890ULL, .rx_packets=0,
		    .tx_bytes=0, .tx_packets=9876543210987654321ULL,
		    .vid=0, .flags=0, .pad2=0xbadc0ded },
		  "{rx_bytes=12345678901234567890, rx_packets=0"
		  ", tx_bytes=0, tx_packets=9876543210987654321"
		  ", vid=0, flags=0, pad2=0xbadc0ded}" },
		{ { .flags=0xdeed },
		  "{rx_bytes=0, rx_packets=0, tx_bytes=0, tx_packets=0, vid=0"
		  ", flags=" XLAT_KNOWN(0xdeed, "BRIDGE_VLAN_INFO_MASTER"
						"|BRIDGE_VLAN_INFO_UNTAGGED"
						"|BRIDGE_VLAN_INFO_RANGE_BEGIN"
						"|BRIDGE_VLAN_INFO_BRENTRY"
						"|BRIDGE_VLAN_INFO_ONLY_OPTS"
						"|0xde80") "}" },
		{ { .rx_bytes=0xdefaceddecaffeedULL,
		    .rx_packets=0xbeeffacedeadbabeULL,
		    .tx_bytes=0xbeeffeeddadfacedULL,
		    .tx_packets=0xbeeffadeeffaceedULL,
		    .vid=0xcafe, .flags=0xfa80, .pad2=0xdeadabba },
		  "{rx_bytes=16067382073151717101"
		  ", rx_packets=13758491153046289086"
		  ", tx_bytes=13758495684172950765"
		  ", tx_packets=13758491222056029933"
		  ", vid=51966, flags=0xfa80"
		  NRAW(" /* BRIDGE_VLAN_INFO_??? */")
		  ", pad2=0xdeadabba}" },
	};
	void *nlh_vlan = midtail_alloc(NLMSG_SPACE(hdrlen),
				       2 * NLA_HDRLEN
				       + sizeof(struct bridge_vlan_xstats));

	for (size_t i = 0; i < ARRAY_SIZE(br_xst_vlan_vecs); i++) {
		TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh_vlan, hdrlen,
					      init_ifstats_l2, print_ifstats_l2,
					      BRIDGE_XSTATS_VLAN,
					      XLAT_KNOWN(0x1,
							 "BRIDGE_XSTATS_VLAN"),
					      pattern, br_xst_vlan_vecs[i].val,
					      print_quoted_hex, 2,
					      printf("%s",
						     br_xst_vlan_vecs[i].str));

		char buf[sizeof(br_xst_vlan_vecs[0].val) + 42];
		fill_memory(buf, sizeof(buf));
		memcpy(buf, &br_xst_vlan_vecs[i].val,
		       sizeof(br_xst_vlan_vecs[i].val));
		TEST_NLATTR_(fd, nlh0, hdrlen + 2 * NLA_HDRLEN,
			     init_ifstats_l2, print_ifstats_l2,
			     BRIDGE_XSTATS_VLAN,
			     XLAT_KNOWN(0x1, "BRIDGE_XSTATS_VLAN"),
			     sizeof(buf), buf, sizeof(buf),
			     printf("%s", br_xst_vlan_vecs[i].str);
			     printf(", ");
			     print_quoted_hex(buf
					      + sizeof(br_xst_vlan_vecs[0].val),
					      32);
			     printf("...]]"));
	}

	/* LINK_XSTATS_TYPE_BRIDGE: BRIDGE_XSTATS_MCAST */
	struct br_mcast_stats br_xst_mc;
	void *nlh_mc = midtail_alloc(NLMSG_SPACE(hdrlen),
				     2 * NLA_HDRLEN + sizeof(br_xst_mc));

#define FIELD_STR_(field_) \
	#field_ "=[[" XLAT_KNOWN(0, "BR_MCAST_DIR_RX") "]=0, [" \
	XLAT_KNOWN(1, "BR_MCAST_DIR_TX") "]=0]"

	memset(&br_xst_mc, 0, sizeof(br_xst_mc));
	TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh_mc, hdrlen,
				      init_ifstats_l2, print_ifstats_l2,
				      BRIDGE_XSTATS_MCAST,
				      XLAT_KNOWN(0x2, "BRIDGE_XSTATS_MCAST"),
				      pattern, br_xst_mc,
				      print_quoted_hex, 2,
				      printf("{" FIELD_STR_(igmp_v1queries)
					     ", " FIELD_STR_(igmp_v2queries)
					     ", " FIELD_STR_(igmp_v3queries)
					     ", " FIELD_STR_(igmp_leaves)
					     ", " FIELD_STR_(igmp_v1reports)
					     ", " FIELD_STR_(igmp_v2reports)
					     ", " FIELD_STR_(igmp_v3reports)
					     ", igmp_parse_errors=0"
					     ", " FIELD_STR_(mld_v1queries)
					     ", " FIELD_STR_(mld_v2queries)
					     ", " FIELD_STR_(mld_leaves)
					     ", " FIELD_STR_(mld_v1reports)
					     ", " FIELD_STR_(mld_v2reports)
					     ", mld_parse_errors=0"
					     ", " FIELD_STR_(mcast_bytes)
					     ", " FIELD_STR_(mcast_packets)
					     "}"));
#undef FIELD_STR_

	fill_memory64(&br_xst_mc, sizeof(br_xst_mc));
	TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh_mc, hdrlen,
				      init_ifstats_l2, print_ifstats_l2,
				      BRIDGE_XSTATS_MCAST,
				      XLAT_KNOWN(0x2, "BRIDGE_XSTATS_MCAST"),
				      pattern, br_xst_mc,
				      print_quoted_hex, 2,
				      print_mcast_stats(&br_xst_mc));

	char mc_buf[sizeof(br_xst_mc) + 8];
	fill_memory(mc_buf, sizeof(mc_buf));
	memcpy(mc_buf, &br_xst_mc, sizeof(br_xst_mc));
	TEST_NLATTR_(fd, nlh0, hdrlen + 2 * NLA_HDRLEN,
		     init_ifstats_l2, print_ifstats_l2,
		     BRIDGE_XSTATS_MCAST,
		     XLAT_KNOWN(0x2, "BRIDGE_XSTATS_MCAST"),
		     sizeof(mc_buf), mc_buf, sizeof(mc_buf),
		     print_mcast_stats(&br_xst_mc);
		     printf(", ");
		     print_quoted_hex(mc_buf + sizeof(br_xst_mc), 8);
		     printf("]]"));

	/* LINK_XSTATS_TYPE_BRIDGE: BRIDGE_XSTATS_STP */
	static const struct {
		struct bridge_stp_xstats val;
		const char *str;
	} br_xst_stp_vecs[] = {
		{ { .transition_blk=0, .transition_fwd=0, .rx_bpdu=0,
		    .tx_bpdu=0, .rx_tcn=0, .tx_tcn=0, },
		  "{transition_blk=0, transition_fwd=0, rx_bpdu=0, tx_bpdu=0"
		  ", rx_tcn=0, tx_tcn=0}" },
		{ { .transition_blk=0x8090a0b0c0d0e0f0ULL,
		    .transition_fwd=0x8191a1b1c1d1e1f1ULL,
		    .rx_bpdu=0x8292a2b2c2d2e2f2ULL,
		    .tx_bpdu=0x8393a3b3c3d3e3f3ULL,
		    .rx_tcn=0x8494a4b4c4d4e4f4ULL,
		    .tx_tcn=0x8595a5b5c5d5e5f5ULL, },
		  "{transition_blk=9264081114510713072"
		  ", transition_fwd=9336421287348789745"
		  ", rx_bpdu=9408761460186866418, tx_bpdu=9481101633024943091"
		  ", rx_tcn=9553441805863019764, tx_tcn=9625781978701096437}" },
	};
	void *nlh_stp = midtail_alloc(NLMSG_SPACE(hdrlen),
				      2 * NLA_HDRLEN
				      + sizeof(struct bridge_stp_xstats));

	for (size_t i = 0; i < ARRAY_SIZE(br_xst_stp_vecs); i++) {
		TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh_stp, hdrlen,
					      init_ifstats_l2, print_ifstats_l2,
					      BRIDGE_XSTATS_STP,
					      XLAT_KNOWN(0x4,
							 "BRIDGE_XSTATS_STP"),
					      pattern, br_xst_stp_vecs[i].val,
					      print_quoted_hex, 2,
					      printf("%s",
						     br_xst_stp_vecs[i].str));

		char buf[sizeof(br_xst_stp_vecs[0].val) + 33];
		fill_memory(buf, sizeof(buf));
		memcpy(buf, &br_xst_stp_vecs[i].val,
		       sizeof(br_xst_stp_vecs[i].val));
		TEST_NLATTR_(fd, nlh0, hdrlen + 2 * NLA_HDRLEN,
			     init_ifstats_l2, print_ifstats_l2,
			     BRIDGE_XSTATS_STP,
			     XLAT_KNOWN(0x4, "BRIDGE_XSTATS_STP"),
			     sizeof(buf), buf, sizeof(buf),
			     printf("%s", br_xst_stp_vecs[i].str);
			     printf(", ");
			     print_quoted_hex(buf
					      + sizeof(br_xst_stp_vecs[0].val),
					      32);
			     printf("...]]"));
	}



	/* LINK_XSTATS_TYPE_BOND */
	l2_attr = LINK_XSTATS_TYPE_BOND;
	snprintf(l2_attr_str, sizeof(l2_attr_str), XLAT_FMT,
		 XLAT_ARGS(LINK_XSTATS_TYPE_BOND));

	/* LINK_XSTATS_TYPE_BOND: Unknown, unimplemented, no semantics.  */
	static const struct strval32 undec_bd_types[] = {
		{ ARG_STR(BOND_XSTATS_UNSPEC) },
		{ 0x2 },
		{ 0xbad },
	};
	for (size_t i = 0; i < ARRAY_SIZE(undec_bd_types); i++) {
		fmt_str(nla_type_str, sizeof(nla_type_str),
			undec_bd_types[i].val, undec_bd_types[i].str,
			"BOND_XSTATS_???");
		TEST_NLATTR_(fd, nlh0, hdrlen + 2 * NLA_HDRLEN,
			     init_ifstats_l2, print_ifstats_l2,
			     undec_bd_types[i].val, nla_type_str,
			     37, pattern, 37,
			     print_quoted_hex(pattern, 32);
			     printf("...]]"));
	}

	/* LINK_XSTATS_TYPE_BOND: BOND_XSTATS_3AD */
	l3_attr = BOND_XSTATS_3AD;
	snprintf(l3_attr_str, sizeof(l3_attr_str), XLAT_FMT,
		 XLAT_ARGS(BOND_XSTATS_3AD));

	/* BOND_XSTATS_3AD: Unknown, unimplemented, no semantics.  */
	static const struct strval32 undec_3ad_types[] = {
		{ ARG_STR(BOND_3AD_STAT_PAD) },
		{ 0xa },
		{ 0xbad },
	};
	for (size_t i = 0; i < ARRAY_SIZE(undec_3ad_types); i++) {
		fmt_str(nla_type_str, sizeof(nla_type_str),
			undec_3ad_types[i].val, undec_3ad_types[i].str,
			"BOND_XSTATS_???");
		TEST_NLATTR_(fd, nlh0, hdrlen + 3 * NLA_HDRLEN,
			     init_ifstats_l3, print_ifstats_l3,
			     undec_3ad_types[i].val, nla_type_str,
			     37, pattern, 37,
			     print_quoted_hex(pattern, 32);
			     printf("...]]]"));
	}

	/* BOND_XSTATS_3AD: u64 args */
	static const struct strval32 u64_3ad_types[] = {
		{ ARG_STR(BOND_3AD_STAT_LACPDU_RX) },
		{ ARG_STR(BOND_3AD_STAT_LACPDU_TX) },
		{ ARG_STR(BOND_3AD_STAT_LACPDU_UNKNOWN_RX) },
		{ ARG_STR(BOND_3AD_STAT_LACPDU_ILLEGAL_RX) },
		{ ARG_STR(BOND_3AD_STAT_MARKER_RX) },
		{ ARG_STR(BOND_3AD_STAT_MARKER_TX) },
		{ ARG_STR(BOND_3AD_STAT_MARKER_RESP_RX) },
		{ ARG_STR(BOND_3AD_STAT_MARKER_RESP_TX) },
		{ ARG_STR(BOND_3AD_STAT_MARKER_UNKNOWN_RX) },
	};
	void *nlh_3ad_u64 = midtail_alloc(NLMSG_SPACE(hdrlen),
					  3 * NLA_HDRLEN + sizeof(uint64_t));
	for (size_t i = 0; i < ARRAY_SIZE(u64_3ad_types); i++) {
		snprintf(nla_type_str, sizeof(nla_type_str), XLAT_FMT,
			 XLAT_SEL(u64_3ad_types[i].val, u64_3ad_types[i].str));
		check_u64_nlattr(fd, nlh_3ad_u64, hdrlen,
				 init_ifstats_l3, print_ifstats_l3,
				 u64_3ad_types[i].val, nla_type_str,
				 pattern, 3);
	}
}

static void
check_stats_offload(const int fd)
{
	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen) + 3 * NLA_HDRLEN,
				   NLA_HDRLEN + 128);

	/* IFLA_STATS_LINK_OFFLOAD_XSTATS */
	l1_attr = IFLA_STATS_LINK_OFFLOAD_XSTATS;
	snprintf(l1_attr_str, sizeof(l1_attr_str), XLAT_FMT,
		 XLAT_ARGS(IFLA_STATS_LINK_OFFLOAD_XSTATS));

	/* Unknown, unimplemented, no semantics.  */
	static const struct strval32 undec_types[] = {
		{ ARG_STR(IFLA_OFFLOAD_XSTATS_UNSPEC) },
		{ 0x2 },
		{ 0xbad },
	};
	for (size_t i = 0; i < ARRAY_SIZE(undec_types); i++) {
		fmt_str(nla_type_str, sizeof(nla_type_str),
			undec_types[i].val, undec_types[i].str,
			"IFLA_OFFLOAD_XSTATS_???");
		TEST_NLATTR_(fd, nlh0, hdrlen + NLA_HDRLEN,
			     init_ifstats_l1, print_ifstats_l1,
			     undec_types[i].val, nla_type_str,
			     37, pattern, 37,
			     print_quoted_hex(pattern, 32);
			     printf("...]"));
	}

	/* IFLA_OFFLOAD_XSTATS_CPU_HIT */
	check_stats_64(fd, ARG_STR(IFLA_OFFLOAD_XSTATS_CPU_HIT), true);
}

/*
 * skip_af is expected to be sorted
 *
 * [RTM_GETSTATS] -> struct if_stats_msg
 *   [cmd]
 *     [AF_*]
 *       [0] -> u32
 *       [1] -> u64
 */
static void
check_stats_af_generic(const int fd, unsigned int cmd, const char *cmd_str,
		       const uint8_t * const skip_af, const size_t skip_af_cnt)
{
	enum { ATTR_SZ = NLA_HDRLEN + 2 * NLA_HDRLEN + 4 + 8 };

	/*
	 * The payload is designed like this so if a decoder for a new AF_*
	 * is implemented, this check will likely fail.
	 */
	struct {
		struct nlattr hdr;

		struct nlattr nested_hdr1;
		uint32_t nested_data1;

		struct nlattr nested_hdr2;
		uint64_t nested_data2;
	} dummy_data = {
		{ ATTR_SZ, 0 /* AF_* */ },
		{ NLA_HDRLEN + sizeof(uint32_t), 0 }, 0xdeadc0de,
		{ NLA_HDRLEN + sizeof(uint64_t), 1 }, 0xbadda7adeadfacedULL,
	};
	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen), NLA_HDRLEN + ATTR_SZ);
	size_t skip_pos = 0;

	static_assert(ATTR_SZ == sizeof(dummy_data),
		      "Dummy nlattr payload size mismatch");

	snprintf(nla_type_str, sizeof(nla_type_str), XLAT_FMT,
		 XLAT_SEL(cmd, cmd_str));

	for (size_t i = 0; i < 256; i++) {
		if (skip_pos < skip_af_cnt && i == skip_af[skip_pos]) {
			skip_pos++;
			continue;
		}

		dummy_data.hdr.nla_type = i;
		TEST_NLATTR_(fd, nlh0, hdrlen, init_ifstats, print_ifstats,
			     cmd, nla_type_str, ATTR_SZ, &dummy_data, ATTR_SZ,
			     printf("[{nla_len=%u, nla_type=", ATTR_SZ);
			     printxval(addrfams, i, "AF_???");
			     printf("}, ");
			     print_quoted_hex(&dummy_data.nested_hdr1,
					      sizeof(dummy_data) - NLA_HDRLEN);
			     printf("]"));
	}
}

static void
check_stats_af_mpls(const int fd)
{
	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen) + 3 * NLA_HDRLEN,
				   NLA_HDRLEN + 128);

	/* l1: IFLA_STATS_AF_SPEC */
	l1_attr = IFLA_STATS_AF_SPEC;
	snprintf(l1_attr_str, sizeof(l1_attr_str),
		 XLAT_KNOWN(0x5, "IFLA_STATS_AF_SPEC"));

	/* l2: AF_MPLS */
	l2_attr = AF_MPLS;
	snprintf(l2_attr_str, sizeof(l2_attr_str), XLAT_FMT,
		 XLAT_ARGS(AF_MPLS));

	/* Unknown, unimplemented, no semantics.  */
	static const struct strval32 undec_types[] = {
		{ ARG_STR(MPLS_STATS_UNSPEC) },
		{ 0x2 },
		{ 0xbad },
	};
	for (size_t i = 0; i < ARRAY_SIZE(undec_types); i++) {
		fmt_str(nla_type_str, sizeof(nla_type_str),
			undec_types[i].val, undec_types[i].str,
			"MPLS_STATS_???");
		TEST_NLATTR_(fd, nlh0, hdrlen + 2 * NLA_HDRLEN,
			     init_ifstats_l2, print_ifstats_l2,
			     undec_types[i].val, nla_type_str,
			     37, pattern, 37,
			     print_quoted_hex(pattern, 32);
			     printf("...]]"));
	}

	/* MPLS_STATS_LINK */
	struct mpls_link_stats mls;
	void *nlh_mls = midtail_alloc(NLMSG_SPACE(hdrlen),
				      2 * NLA_HDRLEN + sizeof(mls));

	memset(&mls, 0, sizeof(mls));
	TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh_mls, hdrlen,
				      init_ifstats_l2, print_ifstats_l2,
				      MPLS_STATS_LINK,
				      XLAT_KNOWN(0x1, "MPLS_STATS_LINK"),
				      pattern, mls, print_quoted_hex, 2,
				      printf("{rx_packets=0, tx_packets=0"
					     ", rx_bytes=0, tx_bytes=0"
					     ", rx_errors=0, tx_errors=0"
					     ", rx_dropped=0, tx_dropped=0"
					     ", rx_noroute=0}"));

	typedef unsigned long long ullong;
	fill_memory64(&mls, sizeof(mls));
	TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh_mls, hdrlen,
				      init_ifstats_l2, print_ifstats_l2,
				      MPLS_STATS_LINK,
				      XLAT_KNOWN(0x1, "MPLS_STATS_LINK"),
				      pattern, mls, print_quoted_hex, 2,
				      printf("{rx_packets=%llu, tx_packets=%llu"
					     ", rx_bytes=%llu, tx_bytes=%llu"
					     ", rx_errors=%llu, tx_errors=%llu"
					     ", rx_dropped=%llu"
					     ", tx_dropped=%llu"
					     ", rx_noroute=%llu}",
					     (ullong) mls.rx_packets,
					     (ullong) mls.tx_packets,
					     (ullong) mls.rx_bytes,
					     (ullong) mls.tx_bytes,
					     (ullong) mls.rx_errors,
					     (ullong) mls.tx_errors,
					     (ullong) mls.rx_dropped,
					     (ullong) mls.tx_dropped,
					     (ullong) mls.rx_noroute));

	char mls_buf[sizeof(mls) + 32];
	fill_memory(mls_buf, sizeof(mls_buf));
	memcpy(mls_buf, &mls, sizeof(mls));
	TEST_NLATTR_(fd, nlh0, hdrlen + 2 * NLA_HDRLEN,
		     init_ifstats_l2, print_ifstats_l2,
		     MPLS_STATS_LINK, XLAT_KNOWN(0x1, "MPLS_STATS_LINK"),
		     sizeof(mls_buf), mls_buf, sizeof(mls_buf),
		     printf("{rx_packets=9264081114510713072"
			    ", tx_packets=9264081114510713073"
			    ", rx_bytes=9264081114510713074"
			    ", tx_bytes=9264081114510713075"
			    ", rx_errors=9264081114510713076"
			    ", tx_errors=9264081114510713077"
			    ", rx_dropped=9264081114510713078"
			    ", tx_dropped=9264081114510713079"
			    ", rx_noroute=9264081114510713080}, ");
		     print_quoted_hex(mls_buf + sizeof(mls), 32);
		     printf("]]"));
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	const int fd = create_nl_socket(NETLINK_ROUTE);
	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen),
				   NLA_HDRLEN + 256);

	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

	/* Unknown attrs.  */
	static const uint16_t unk_types[] = { 6, 0xffff & NLA_TYPE_MASK };
	for (size_t i = 0; i < ARRAY_SIZE(unk_types); i++) {
		sprintf(nla_type_str, "%#x" NRAW(" /* IFLA_STATS_??? */"),
			unk_types[i]);
		TEST_NLATTR_(fd, nlh0, hdrlen,
			     init_ifstats, print_ifstats,
			     unk_types[i], nla_type_str,
			     4, pattern, 4,
			     print_quoted_hex(pattern, 4));
	}


	/* IFLA_STATS_UNSPEC: unimplemented, no semantics.  */
	static const struct strval32 unimp_types[] = {
		{ ARG_XLAT_KNOWN(0, "IFLA_STATS_UNSPEC") },
	};
	for (size_t i = 0; i < ARRAY_SIZE(unimp_types); i++) {
		TEST_NLATTR_(fd, nlh0, hdrlen,
			     init_ifstats, print_ifstats,
			     unimp_types[i].val, unimp_types[i].str,
			     42, pattern, 42,
			     print_quoted_hex(pattern, 32);
			     printf("..."));
	}


	/* IFLA_STATS_LINK_64 */
	check_stats_64(fd, ARG_STR(IFLA_STATS_LINK_64), false);


	/* IFLA_STATS_LINK_XSTATS, IFLA_STATS_LINK_XSTATS_SLAVE */
	check_xstats(fd, ARG_STR(IFLA_STATS_LINK_XSTATS));
	check_xstats(fd, ARG_STR(IFLA_STATS_LINK_XSTATS_SLAVE));


	/* IFLA_STATS_LINK_OFFLOAD_STATS */
	check_stats_offload(fd);


	/* IFLA_STATS_AF_SPEC */
	static const uint8_t af_spec_fams[] = { AF_MPLS };
	check_stats_af_generic(fd, ARG_STR(IFLA_STATS_AF_SPEC),
			       ARRSZ_PAIR(af_spec_fams));

	/* IFLA_STATS_AF_SPEC: AF_MPLS */
	check_stats_af_mpls(fd);


	puts("+++ exited with 0 +++");
	return 0;
}
