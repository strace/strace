/*
 * IFLA_VFINFO_LIST netlink attribute decoding check.
 *
 * Copyright (c) 2018-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>

#include <arpa/inet.h>

#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/if_link.h>
#include <linux/rtnetlink.h>

#include "test_nlattr.h"

#include "xlat.h"

#define IFLA_ATTR IFLA_VFINFO_LIST
#include "nlattr_ifla.h"

static void
init_vf_info_msg(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	init_ifinfomsg(nlh, msg_len);

	struct nlattr *nla = NLMSG_ATTR(nlh, hdrlen);
	nla += 1;
	SET_STRUCT(struct nlattr, nla,
		.nla_len = msg_len - NLMSG_SPACE(hdrlen)
			  - NLA_HDRLEN,
		.nla_type = IFLA_VF_INFO,
	);
}

static void
print_vf_info_msg(const unsigned int msg_len)
{
	print_ifinfomsg(msg_len);
	printf(", [{nla_len=%u, nla_type=" XLAT_FMT "}",
	       msg_len - NLMSG_SPACE(hdrlen) - NLA_HDRLEN,
	       XLAT_ARGS(IFLA_VF_INFO));
}

static void
init_vf_stats_msg(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	init_vf_info_msg(nlh, msg_len);

	struct nlattr *nla = NLMSG_ATTR(nlh, hdrlen);
	nla += 2;
	SET_STRUCT(struct nlattr, nla,
		.nla_len = msg_len - NLMSG_SPACE(hdrlen)
			  - NLA_HDRLEN * 2,
		.nla_type = IFLA_VF_STATS,
	);
}

static void
print_vf_stats_msg(const unsigned int msg_len)
{
	print_vf_info_msg(msg_len);
	printf(", [{nla_len=%u, nla_type=" XLAT_FMT "}",
	       msg_len - NLMSG_SPACE(hdrlen) - NLA_HDRLEN * 2,
	       XLAT_ARGS(IFLA_VF_STATS));
}

static void
init_vlan_list_msg(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	init_vf_info_msg(nlh, msg_len);

	struct nlattr *nla = NLMSG_ATTR(nlh, hdrlen);
	nla += 2;
	SET_STRUCT(struct nlattr, nla,
		.nla_len = msg_len - NLMSG_SPACE(hdrlen)
			  - NLA_HDRLEN * 2,
		.nla_type = IFLA_VF_VLAN_LIST,
	);
}

static void
print_vlan_list_msg(const unsigned int msg_len)
{
	print_vf_info_msg(msg_len);
	printf(", [{nla_len=%u, nla_type=" XLAT_FMT "}",
	       msg_len - NLMSG_SPACE(hdrlen) - NLA_HDRLEN * 2,
	       XLAT_ARGS(IFLA_VF_VLAN_LIST));
}

int
main(void)
{
	static const uint8_t dummy[] = { 0xab, 0xac, 0xdb, 0xcd };

	skip_if_unavailable("/proc/self/fd/");

	const int fd = create_nl_socket(NETLINK_ROUTE);

	const unsigned int hdrlen = sizeof(struct ifinfomsg);
	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen), 2 * NLA_HDRLEN + 256);

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);


	/* unknown IFLA_VF_INFO*, IFLA_VF_INFO_UNSPEC */
	static const struct strval16 unk_attrs[] = {
		{ ENUM_KNOWN(0, IFLA_VF_INFO_UNSPEC) },
		{ ARG_XLAT_UNKNOWN(0x2, "IFLA_VF_INFO_???") },
		{ ARG_XLAT_UNKNOWN(0x1ace, "IFLA_VF_INFO_???") },
	};
	for (size_t i = 0; i < ARRAY_SIZE(unk_attrs); i++) {
		TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
					   init_ifinfomsg, print_ifinfomsg,
					   unk_attrs[i].val, unk_attrs[i].str,
					   pattern, dummy, print_quoted_hex, 1,
					   printf("\"\\xab\\xac\\xdb\\xcd\""));
	}

	/* IFLA_VF_INFO: unknown, IFLA_VF_UNSPEC */
	static const struct strval16 unk_vf_attrs[] = {
		{ ENUM_KNOWN(0, IFLA_VF_UNSPEC) },
		{ ARG_XLAT_UNKNOWN(0xe, "IFLA_VF_???") },
		{ ARG_XLAT_UNKNOWN(0x1ace, "IFLA_VF_???") },
	};
	for (size_t i = 0; i < ARRAY_SIZE(unk_vf_attrs); i++) {
		TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
					      init_vf_info_msg,
					      print_vf_info_msg,
					      unk_vf_attrs[i].val,
					      unk_vf_attrs[i].str,
					      pattern, dummy,
					      print_quoted_hex, 2,
					      printf("\"\\xab\\xac\\xdb\\xcd\"")
					      );
	}

	/* IFLA_VF_INFO: IFLA_VF_MAC */
	struct ifla_vf_mac ifla_vm;
	ifla_vm.vf = 0xdeadface;
	fill_memory(&ifla_vm.mac, sizeof(ifla_vm.mac));
	TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
				      init_vf_info_msg, print_vf_info_msg,
				      IFLA_VF_MAC,
				      XLAT_KNOWN(0x1, "IFLA_VF_MAC"),
				      pattern, ifla_vm, print_quoted_hex, 2,
				      printf("{vf=3735943886, mac="
					     XLAT_KNOWN_FMT("\"\\x80\\x81"
					     "\\x82\\x83\\x84\\x85\\x86\\x87"
					     "\\x88\\x89\\x8a\\x8b\\x8c\\x8d"
					     "\\x8e\\x8f\\x90\\x91\\x92\\x93"
					     "\\x94\\x95\\x96\\x97\\x98\\x99"
					     "\\x9a\\x9b\\x9c\\x9d\\x9e\\x9f\"",
					     "80:81:82:83:84:85:86:87:88:89:"
					     "8a:8b:8c:8d:8e:8f:90:91:92:93:"
					     "94:95:96:97:98:99:9a:9b:9c:9d:"
					     "9e:9f") "}"));

	/* IFLA_VF_INFO: IFLA_VF_VLAN */
	struct ifla_vf_vlan ifla_vv;
	ifla_vv.vf	= 0x80a0c0e0;
	ifla_vv.vlan	= 0x81a1c1e1;
	ifla_vv.qos	= 0x82a2c2e2;
	TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
				      init_vf_info_msg, print_vf_info_msg,
				      IFLA_VF_VLAN,
				      XLAT_KNOWN(0x2, "IFLA_VF_VLAN"),
				      pattern, ifla_vv, print_quoted_hex, 2,
				      printf("{vf=2158018784, vlan=2174861793"
					     ", qos=2191704802}"));

	/* IFLA_VF_INFO: IFLA_VF_TX_RATE */
	struct ifla_vf_tx_rate ifla_vtr;
	ifla_vtr.vf	= 0x80a0c0e0;
	ifla_vtr.rate	= 0x81a1c1e1;
	TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
				      init_vf_info_msg, print_vf_info_msg,
				      IFLA_VF_TX_RATE,
				      XLAT_KNOWN(0x3, "IFLA_VF_TX_RATE"),
				      pattern, ifla_vtr, print_quoted_hex, 2,
				      printf("{vf=2158018784"
					     ", rate=2174861793}"));

	/* IFLA_VF_INFO: IFLA_VF_SPOOFCHK */
	struct ifla_vf_spoofchk ifla_vsc;
	ifla_vsc.vf		= 0x80a0c0e0;
	ifla_vsc.setting	= 0x81a1c1e1;
	TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
				      init_vf_info_msg, print_vf_info_msg,
				      IFLA_VF_SPOOFCHK,
				      XLAT_KNOWN(0x4, "IFLA_VF_SPOOFCHK"),
				      pattern, ifla_vsc, print_quoted_hex, 2,
				      printf("{vf=2158018784"
					     ", setting=2174861793}"));

	/* IFLA_VF_INFO: IFLA_VF_LINK_STATE */
	static const struct strval32 states[] = {
		{ ENUM_KNOWN(0, IFLA_VF_LINK_STATE_AUTO) },
		{ ENUM_KNOWN(0x1, IFLA_VF_LINK_STATE_ENABLE) },
		{ ENUM_KNOWN(0x2, IFLA_VF_LINK_STATE_DISABLE) },
		{ ARG_STR(0x3) NRAW(" /* IFLA_VF_LINK_STATE_??? */") },
		{ ARG_STR(0x4) NRAW(" /* IFLA_VF_LINK_STATE_??? */") },
		{ ARG_STR(0xfade) NRAW(" /* IFLA_VF_LINK_STATE_??? */") },
	};
	struct ifla_vf_link_state ifla_vls;
	ifla_vls.vf = 0xbeeffeed;

	for (size_t i = 0; i < ARRAY_SIZE(states); i++) {
		ifla_vls.link_state = states[i].val;
		TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
					      init_vf_info_msg,
					      print_vf_info_msg,
					      IFLA_VF_LINK_STATE,
					      XLAT_KNOWN(0x5,
							 "IFLA_VF_LINK_STATE"),
					      pattern, ifla_vls,
					      print_quoted_hex, 2,
					      printf("{vf=3203399405"
						     ", link_state=%s}",
						     states[i].str));
	}

	/* IFLA_VF_INFO: IFLA_VF_RATE */
	struct ifla_vf_rate ifla_vr;
	ifla_vr.vf		= 0x80a0c0e0;
	ifla_vr.min_tx_rate	= 0x81a1c1e1;
	ifla_vr.max_tx_rate	= 0x82a2c2e2;
	TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
				      init_vf_info_msg, print_vf_info_msg,
				      IFLA_VF_RATE,
				      XLAT_KNOWN(0x6, "IFLA_VF_RATE"),
				      pattern, ifla_vr, print_quoted_hex, 2,
				      printf("{vf=2158018784"
					     ", min_tx_rate=2174861793"
					     ", max_tx_rate=2191704802}"));

	/* IFLA_VF_INFO: IFLA_VF_RSS_QUERY_EN */
	struct ifla_vf_rss_query_en ifla_vrqe;
	ifla_vrqe.vf		= 0x80a0c0e0;
	ifla_vrqe.setting	= 0x81a1c1e1;
	TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
				      init_vf_info_msg, print_vf_info_msg,
				      IFLA_VF_RSS_QUERY_EN,
				      XLAT_KNOWN(0x7, "IFLA_VF_RSS_QUERY_EN"),
				      pattern, ifla_vrqe, print_quoted_hex, 2,
				      printf("{vf=2158018784"
					     ", setting=2174861793}"));

	/* IFLA_VF_INFO: IFLA_VF_STATS: unknown, IFLA_VF_STATS_PAD */
	static const struct strval16 unk_vs_attrs[] = {
		{ ENUM_KNOWN(0x6, IFLA_VF_STATS_PAD) },
		{ ARG_XLAT_UNKNOWN(0x9, "IFLA_VF_STATS_???") },
		{ ARG_XLAT_UNKNOWN(0x1ace, "IFLA_VF_STATS_???") },
	};
	for (size_t i = 0; i < ARRAY_SIZE(unk_vs_attrs); i++) {
		TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
					      init_vf_stats_msg,
					      print_vf_stats_msg,
					      unk_vs_attrs[i].val,
					      unk_vs_attrs[i].str,
					      pattern, dummy,
					      print_quoted_hex, 3,
					      printf("\"\\xab\\xac\\xdb\\xcd\"")
					      );
	}

	/* IFLA_VF_INFO: IFLA_VF_STATS: u64 attrs */
	static const struct strval16 u64_vs_attrs[] = {
		{ ENUM_KNOWN(0, IFLA_VF_STATS_RX_PACKETS) },
		{ ENUM_KNOWN(0x1, IFLA_VF_STATS_TX_PACKETS) },
		{ ENUM_KNOWN(0x2, IFLA_VF_STATS_RX_BYTES) },
		{ ENUM_KNOWN(0x3, IFLA_VF_STATS_TX_BYTES) },
		{ ENUM_KNOWN(0x4, IFLA_VF_STATS_BROADCAST) },
		{ ENUM_KNOWN(0x5, IFLA_VF_STATS_MULTICAST) },
		{ ENUM_KNOWN(0x7, IFLA_VF_STATS_RX_DROPPED) },
		{ ENUM_KNOWN(0x8, IFLA_VF_STATS_TX_DROPPED) },
	};
	void *nlh_vs_u64 = midtail_alloc(NLMSG_SPACE(hdrlen),
					  3 * NLA_HDRLEN + sizeof(uint64_t));

	for (size_t i = 0; i < ARRAY_SIZE(u64_vs_attrs); i++) {
		check_u64_nlattr(fd, nlh_vs_u64, hdrlen,
				 init_vf_stats_msg, print_vf_stats_msg,
				 u64_vs_attrs[i].val, u64_vs_attrs[i].str,
				 pattern, 3);
	}

	/* IFLA_VF_INFO: IFLA_VF_TRUST */
	struct ifla_vf_trust ifla_vt;
	ifla_vt.vf	= 0x80a0c0e0;
	ifla_vt.setting	= 0x81a1c1e1;
	TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
				      init_vf_info_msg, print_vf_info_msg,
				      IFLA_VF_TRUST,
				      XLAT_KNOWN(0x9, "IFLA_VF_TRUST"),
				      pattern, ifla_vt, print_quoted_hex, 2,
				      printf("{vf=2158018784"
					     ", setting=2174861793}"));

	/* IFLA_VF_INFO: IFLA_VF_IB_NODE_GUID, IFLA_VF_IB_PORT_GUID */
	static const struct strval16 guid_attrs[] = {
		{ ENUM_KNOWN(0xa, IFLA_VF_IB_NODE_GUID) },
		{ ENUM_KNOWN(0xb, IFLA_VF_IB_PORT_GUID) },
	};
	struct ifla_vf_guid ifla_vg;
	ifla_vg.vf = 0xfacecafe;
	ifla_vg.guid = 0xbadc0deddeedbabeULL;

	for (size_t i = 0; i < ARRAY_SIZE(guid_attrs); i++) {
		TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
					      init_vf_info_msg,
					      print_vf_info_msg,
					      guid_attrs[i].val,
					      guid_attrs[i].str,
					      pattern, ifla_vg,
					      print_quoted_hex, 2,
					      printf("{vf=4207856382"
						     ", guid=0xbadc0deddeedbabe"
						     "}"));
	}

	/* IFLA_VF_INFO: IFLA_VF_VLAN_LIST: unknown, IFLA_VF_VLAN_INFO_UNSPEC */
	static const struct strval16 unk_vl_attrs[] = {
		{ ENUM_KNOWN(0, IFLA_VF_VLAN_INFO_UNSPEC) },
		{ ARG_XLAT_UNKNOWN(0x2, "IFLA_VF_VLAN_INFO_???") },
		{ ARG_XLAT_UNKNOWN(0x1ace, "IFLA_VF_VLAN_INFO_???") },
	};
	for (size_t i = 0; i < ARRAY_SIZE(unk_vl_attrs); i++) {
		TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
					      init_vlan_list_msg,
					      print_vlan_list_msg,
					      unk_vl_attrs[i].val,
					      unk_vl_attrs[i].str,
					      pattern, dummy,
					      print_quoted_hex, 3,
					      printf("\"\\xab\\xac\\xdb\\xcd\"")
					      );
	}

	/* IFLA_VF_INFO: IFLA_VF_VLAN_LIST: IFLA_VF_VLAN_INFO */
	static const struct strval16 eth_protos[] = {
		{ ARG_STR(0) NRAW(" /* ETH_P_??? */") },
		{ ARG_XLAT_KNOWN(0x8, "ETH_P_PPP_MP") },
		{ ARG_XLAT_KNOWN(0x800, "ETH_P_IP") },
		{ ARG_STR(0xf) NRAW(" /* ETH_P_??? */") },
		{ ARG_STR(0xfb) NRAW(" /* ETH_P_??? */") },
		{ ARG_XLAT_KNOWN(0xfbfb, "ETH_P_AF_IUCV") },
		{ ARG_STR(0xffff) NRAW(" /* ETH_P_??? */") },
	};
	struct ifla_vf_vlan_info ifla_vvi;
	ifla_vvi.vf	= 0x80a0c0e0;
	ifla_vvi.vlan	= 0x81a1c1e1;
	ifla_vvi.qos	= 0x82a2c2e2;

	for (size_t i = 0; i < ARRAY_SIZE(eth_protos); i++) {
		ifla_vvi.vlan_proto = htons(eth_protos[i].val);
		TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
					      init_vlan_list_msg,
					      print_vlan_list_msg,
					      IFLA_VF_VLAN_INFO,
					      XLAT_KNOWN(0x1,
							 "IFLA_VF_VLAN_INFO"),
					      pattern, ifla_vvi,
					      print_quoted_hex, 3,
					      printf("{vf=2158018784"
						    ", vlan=2174861793"
						    ", qos=2191704802"
						    ", vlan_proto=htons(%s)}",
						     eth_protos[i].str));
	}

	/* IFLA_VF_INFO: IFLA_VF_BROADCAST */
	struct ifla_vf_broadcast ifla_vb;
	fill_memory(&ifla_vb.broadcast, sizeof(ifla_vb.broadcast));
	TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
				      init_vf_info_msg, print_vf_info_msg,
				      IFLA_VF_BROADCAST,
				      XLAT_KNOWN(0xd, "IFLA_VF_BROADCAST"),
				      pattern, ifla_vb, print_quoted_hex, 2,
				      printf("{broadcast="
					     XLAT_KNOWN_FMT("\"\\x80\\x81"
					     "\\x82\\x83\\x84\\x85\\x86\\x87"
					     "\\x88\\x89\\x8a\\x8b\\x8c\\x8d"
					     "\\x8e\\x8f\\x90\\x91\\x92\\x93"
					     "\\x94\\x95\\x96\\x97\\x98\\x99"
					     "\\x9a\\x9b\\x9c\\x9d\\x9e\\x9f\"",
					     "80:81:82:83:84:85:86:87:88:89:"
					     "8a:8b:8c:8d:8e:8f:90:91:92:93:"
					     "94:95:96:97:98:99:9a:9b:9c:9d:"
					     "9e:9f") "}"));

	puts("+++ exited with 0 +++");
	return 0;
}
