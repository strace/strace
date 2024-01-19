/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include <inttypes.h>
#include "test_nlattr.h"
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/if_link.h>
#include <linux/rtnetlink.h>

#define IFLA_ATTR IFLA_PROTINFO
#define IFLA_AF AF_BRIDGE
#define IFLA_AF_STR "AF_BRIDGE"
#include "nlattr_ifla.h"

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	const int fd = create_nl_socket(NETLINK_ROUTE);
	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen),
				   NLA_HDRLEN * 2 + 42);

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);


	/* Unknown, unhandled, unsupported */
	static const struct strval16 un_attrs[] = {
		{ ENUM_KNOWN(0, IFLA_BRPORT_UNSPEC) },
		{ ENUM_KNOWN(0x18, IFLA_BRPORT_FLUSH) },
		{ ENUM_KNOWN(0x1a, IFLA_BRPORT_PAD) },
		{ ARG_XLAT_UNKNOWN(0x2d, "IFLA_BRPORT_???") },
		{ ARG_XLAT_UNKNOWN(0xbad, "IFLA_BRPORT_???") },
	};
	for (size_t i = 0; i < ARRAY_SIZE(un_attrs); i++) {
		TEST_NLATTR_(fd, nlh0 - NLA_HDRLEN, hdrlen + NLA_HDRLEN,
			     init_ifinfomsg, print_ifinfomsg,
			     un_attrs[i].val, un_attrs[i].str,
			     42, pattern, 42,
			     print_quoted_hex(pattern, 32);
			     printf("...]"));
	}

	/* u8 attrs */
	static const struct strval16 u8_attrs[] = {
		{ ENUM_KNOWN(0x1, IFLA_BRPORT_STATE) },
		{ ENUM_KNOWN(0x4, IFLA_BRPORT_MODE) },
		{ ENUM_KNOWN(0x5, IFLA_BRPORT_GUARD) },
		{ ENUM_KNOWN(0x6, IFLA_BRPORT_PROTECT) },
		{ ENUM_KNOWN(0x7, IFLA_BRPORT_FAST_LEAVE) },
		{ ENUM_KNOWN(0x8, IFLA_BRPORT_LEARNING) },
		{ ENUM_KNOWN(0x9, IFLA_BRPORT_UNICAST_FLOOD) },
		{ ENUM_KNOWN(0xa, IFLA_BRPORT_PROXYARP) },
		{ ENUM_KNOWN(0xb, IFLA_BRPORT_LEARNING_SYNC) },
		{ ENUM_KNOWN(0xc, IFLA_BRPORT_PROXYARP_WIFI) },
		{ ENUM_KNOWN(0x13, IFLA_BRPORT_TOPOLOGY_CHANGE_ACK) },
		{ ENUM_KNOWN(0x14, IFLA_BRPORT_CONFIG_PENDING) },
		{ ENUM_KNOWN(0x19, IFLA_BRPORT_MULTICAST_ROUTER) },
		{ ENUM_KNOWN(0x1b, IFLA_BRPORT_MCAST_FLOOD) },
		{ ENUM_KNOWN(0x1c, IFLA_BRPORT_MCAST_TO_UCAST) },
		{ ENUM_KNOWN(0x1d, IFLA_BRPORT_VLAN_TUNNEL) },
		{ ENUM_KNOWN(0x1e, IFLA_BRPORT_BCAST_FLOOD) },
		{ ENUM_KNOWN(0x20, IFLA_BRPORT_NEIGH_SUPPRESS) },
		{ ENUM_KNOWN(0x21, IFLA_BRPORT_ISOLATED) },
		{ ENUM_KNOWN(0x23, IFLA_BRPORT_MRP_RING_OPEN) },
		{ ENUM_KNOWN(0x24, IFLA_BRPORT_MRP_IN_OPEN) },
		{ ENUM_KNOWN(0x27, IFLA_BRPORT_LOCKED) },
		{ ENUM_KNOWN(0x28, IFLA_BRPORT_MAB) },
		{ ENUM_KNOWN(0x2b, IFLA_BRPORT_NEIGH_VLAN_SUPPRESS) },
	};
	void *nlh_u8 = midtail_alloc(NLMSG_SPACE(hdrlen),
				     NLA_HDRLEN * 2 + sizeof(uint8_t));
	for (size_t i = 0; i < ARRAY_SIZE(u8_attrs); i++) {
		check_u8_nlattr(fd, nlh_u8, hdrlen,
				init_ifinfomsg, print_ifinfomsg,
				u8_attrs[i].val, u8_attrs[i].str, pattern, 1);
	}


	/* u16 attrs */
	static const struct strval16 u16_attrs[] = {
		{ ENUM_KNOWN(0x2, IFLA_BRPORT_PRIORITY) },
		{ ENUM_KNOWN(0xf, IFLA_BRPORT_DESIGNATED_PORT) },
		{ ENUM_KNOWN(0x10, IFLA_BRPORT_DESIGNATED_COST) },
		{ ENUM_KNOWN(0x11, IFLA_BRPORT_ID) },
		{ ENUM_KNOWN(0x12, IFLA_BRPORT_NO) },
	};
	void *nlh_u16 = midtail_alloc(NLMSG_SPACE(hdrlen),
				     NLA_HDRLEN * 2 + sizeof(uint16_t));
	for (size_t i = 0; i < ARRAY_SIZE(u16_attrs); i++) {
		check_u16_nlattr(fd, nlh_u16, hdrlen,
				 init_ifinfomsg, print_ifinfomsg,
				 u16_attrs[i].val, u16_attrs[i].str,
				 pattern, 1);
	}


	/* x16 attrs */
	static const struct strval16 x16_attrs[] = {
		{ ENUM_KNOWN(0x1f, IFLA_BRPORT_GROUP_FWD_MASK) },
	};
	void *nlh_x16 = midtail_alloc(NLMSG_SPACE(hdrlen),
				     NLA_HDRLEN * 2 + sizeof(uint16_t));
	for (size_t i = 0; i < ARRAY_SIZE(x16_attrs); i++) {
		check_x16_nlattr(fd, nlh_x16, hdrlen,
				 init_ifinfomsg, print_ifinfomsg,
				 x16_attrs[i].val, x16_attrs[i].str,
				 pattern, 1);
	}


	/* u32 attrs */
	static const struct strval16 u32_attrs[] = {
		{ ENUM_KNOWN(0x3, IFLA_BRPORT_COST) },
		{ ENUM_KNOWN(0x25, IFLA_BRPORT_MCAST_EHT_HOSTS_LIMIT) },
		{ ENUM_KNOWN(0x26, IFLA_BRPORT_MCAST_EHT_HOSTS_CNT) },
		{ ENUM_KNOWN(0x29, IFLA_BRPORT_MCAST_N_GROUPS) },
		{ ENUM_KNOWN(0x2a, IFLA_BRPORT_MCAST_MAX_GROUPS) },
		{ ENUM_KNOWN(0x2c, IFLA_BRPORT_BACKUP_NHID) },
	};
	void *nlh_u32 = midtail_alloc(NLMSG_SPACE(hdrlen),
				     NLA_HDRLEN * 2 + sizeof(uint32_t));
	for (size_t i = 0; i < ARRAY_SIZE(u32_attrs); i++) {
		check_u32_nlattr(fd, nlh_u32, hdrlen,
				 init_ifinfomsg, print_ifinfomsg,
				 u32_attrs[i].val, u32_attrs[i].str,
				 pattern, 1);
	}


	/* clock_t attrs */
	static const struct strval16 c_t_attrs[] = {
		{ ENUM_KNOWN(0x15, IFLA_BRPORT_MESSAGE_AGE_TIMER) },
		{ ENUM_KNOWN(0x16, IFLA_BRPORT_FORWARD_DELAY_TIMER) },
		{ ENUM_KNOWN(0x17, IFLA_BRPORT_HOLD_TIMER) },
	};
	void *nlh_c_t = midtail_alloc(NLMSG_SPACE(hdrlen),
				     NLA_HDRLEN * 2 + sizeof(uint64_t));
	for (size_t i = 0; i < ARRAY_SIZE(c_t_attrs); i++) {
		check_clock_t_nlattr(fd, nlh_c_t, hdrlen,
				     init_ifinfomsg, print_ifinfomsg,
				     c_t_attrs[i].val, c_t_attrs[i].str, 1);
	}


	/* struct ifla_bridge_id attrs */
	static const struct ifla_bridge_id id = {
		.prio = { 0xab, 0xcd },
		.addr = { 0xab, 0xcd, 0xef, 0xac, 0xbc, 0xcd }
	};
	static const struct strval16 id_attrs[] = {
		{ ENUM_KNOWN(0xd, IFLA_BRPORT_ROOT_ID) },
		{ ENUM_KNOWN(0xe, IFLA_BRPORT_BRIDGE_ID) },
	};
	void *nlh_id = midtail_alloc(NLMSG_SPACE(hdrlen),
				     NLA_HDRLEN * 2 + sizeof(id));
	for (size_t i = 0; i < ARRAY_SIZE(id_attrs); i++) {
		TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh_id, hdrlen,
					      init_ifinfomsg, print_ifinfomsg,
					      id_attrs[i].val, id_attrs[i].str,
					      pattern, id, print_quoted_hex, 1,
					      printf("{prio=[%1$u, %2$u], addr="
						     XLAT_KNOWN_FMT("\""
						     "\\x%3$02x\\x%4$02x"
						     "\\x%5$02x\\x%6$02x"
						     "\\x%7$02x\\x%8$02x\"",
						     "%3$02x:%4$02x:%5$02x"
						     ":%6$02x:%7$02x:%8$02x")
						     "}",
						     id.prio[0], id.prio[1],
						     id.addr[0], id.addr[1],
						     id.addr[2], id.addr[3],
						     id.addr[4], id.addr[5]));
	}


	/* ifindex attrs */
	uint32_t ifidx = 0xbadc0ded;
	static const struct strval16 if_attrs[] = {
		{ ENUM_KNOWN(0x22, IFLA_BRPORT_BACKUP_PORT) },
	};
	for (size_t i = 0; i < ARRAY_SIZE(if_attrs); i++) {
		ifidx = 0xbadc0ded;
		TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh_u32, hdrlen,
					      init_ifinfomsg, print_ifinfomsg,
					      if_attrs[i].val, if_attrs[i].str,
					      pattern, ifidx,
					      print_quoted_hex, 1,
					      printf("3134983661"));
		ifidx = ifindex_lo();
		TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh_u32, hdrlen,
					      init_ifinfomsg, print_ifinfomsg,
					      if_attrs[i].val, if_attrs[i].str,
					      pattern, ifidx,
					      print_quoted_hex, 1,
					      printf(XLAT_FMT_U,
						     XLAT_SEL(ifidx,
							      IFINDEX_LO_STR)));
	}


	puts("+++ exited with 0 +++");
	return 0;
}
