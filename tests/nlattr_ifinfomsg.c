/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include <stddef.h>
#include "test_nlattr.h"

#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/if_link.h>
#include <linux/rtnetlink.h>

static const unsigned int hdrlen = sizeof(struct ifinfomsg);

static void
init_ifinfomsg(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = msg_len,
		.nlmsg_type = RTM_GETLINK,
		.nlmsg_flags = NLM_F_DUMP
	);

	struct ifinfomsg *const msg = NLMSG_DATA(nlh);
	SET_STRUCT(struct ifinfomsg, msg,
		.ifi_family = AF_UNIX,
		.ifi_type = ARPHRD_LOOPBACK,
		.ifi_index = ifindex_lo(),
		.ifi_flags = IFF_UP,
	);
}

static void
print_ifinfomsg(const unsigned int msg_len)
{
	printf("{nlmsg_len=%u, nlmsg_type=RTM_GETLINK, nlmsg_flags=NLM_F_DUMP"
	       ", nlmsg_seq=0, nlmsg_pid=0}, {ifi_family=AF_UNIX"
	       ", ifi_type=ARPHRD_LOOPBACK"
	       ", ifi_index=" IFINDEX_LO_STR
	       ", ifi_flags=IFF_UP, ifi_change=0}",
	       msg_len);
}

static void
init_prop_list_msg(struct nlmsghdr *const nlh,
		     const unsigned int msg_len)
{
	init_ifinfomsg(nlh, msg_len);

	struct nlattr *nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = msg_len - NLMSG_SPACE(hdrlen),
		.nla_type = IFLA_PROP_LIST,
	);
}

static void
print_prop_list_msg(const unsigned int msg_len)
{
	print_ifinfomsg(msg_len);
	printf(", [{nla_len=%u, nla_type=IFLA_PROP_LIST}",
	       msg_len - NLMSG_SPACE(hdrlen));
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	static const struct rtnl_link_stats st = {
		.rx_packets = 0xabcdefac,
		.tx_packets = 0xbcdacdab,
		.rx_bytes = 0xcdbafaab,
		.tx_bytes = 0xdafabadb,
		.rx_errors = 0xeabcdaeb,
		.tx_errors = 0xfefabeab,
		.rx_dropped = 0xadbafafb,
		.tx_dropped = 0xbdffabda,
		.multicast = 0xcdabdfea,
		.collisions = 0xefadbaeb,
		.rx_length_errors = 0xfabffabd,
		.rx_over_errors = 0xafbafabc,
		.rx_crc_errors = 0xbfdabdad,
		.rx_frame_errors = 0xcfdabfad,
		.rx_fifo_errors = 0xddfdebad,
		.rx_missed_errors = 0xefabdcba,
		.tx_aborted_errors = 0xefdadbfa,
		.tx_carrier_errors = 0xfaefbada,
		.tx_fifo_errors = 0xaebdffab,
		.tx_heartbeat_errors = 0xbadebaaf,
		.tx_window_errors = 0xcdafbada,
		.rx_compressed = 0xdeffadbd,
		.tx_compressed = 0xefdadfab
	};
	const int fd = create_nl_socket(NETLINK_ROUTE);
	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen),
				   2 * NLA_HDRLEN + MAX(sizeof(st), 20));

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

	const unsigned int nla_type = 0xffff & NLA_TYPE_MASK;
	char nla_type_str[256];
	sprintf(nla_type_str, "%#x /* IFLA_??? */", nla_type);
	TEST_NLATTR_(fd, nlh0, hdrlen,
		     init_ifinfomsg, print_ifinfomsg,
		     nla_type, nla_type_str,
		     4, pattern, 4,
		     print_quoted_hex(pattern, 4));

	const int32_t netnsid = 0xacbdabda;
	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_ifinfomsg, print_ifinfomsg,
			   IFLA_LINK_NETNSID, pattern, netnsid,
			   printf("%d", netnsid));

	const unsigned int sizeof_stats =
		offsetofend(struct rtnl_link_stats, tx_compressed);
	TEST_NLATTR_OBJECT_MINSZ(fd, nlh0, hdrlen,
				 init_ifinfomsg, print_ifinfomsg,
				 IFLA_STATS, pattern, st, sizeof_stats,
				 printf("{");
				 PRINT_FIELD_U(st, rx_packets);
				 printf(", ");
				 PRINT_FIELD_U(st, tx_packets);
				 printf(", ");
				 PRINT_FIELD_U(st, rx_bytes);
				 printf(", ");
				 PRINT_FIELD_U(st, tx_bytes);
				 printf(", ");
				 PRINT_FIELD_U(st, rx_errors);
				 printf(", ");
				 PRINT_FIELD_U(st, tx_errors);
				 printf(", ");
				 PRINT_FIELD_U(st, rx_dropped);
				 printf(", ");
				 PRINT_FIELD_U(st, tx_dropped);
				 printf(", ");
				 PRINT_FIELD_U(st, multicast);
				 printf(", ");
				 PRINT_FIELD_U(st, collisions);
				 printf(", ");
				 PRINT_FIELD_U(st, rx_length_errors);
				 printf(", ");
				 PRINT_FIELD_U(st, rx_over_errors);
				 printf(", ");
				 PRINT_FIELD_U(st, rx_crc_errors);
				 printf(", ");
				 PRINT_FIELD_U(st, rx_frame_errors);
				 printf(", ");
				 PRINT_FIELD_U(st, rx_fifo_errors);
				 printf(", ");
				 PRINT_FIELD_U(st, rx_missed_errors);
				 printf(", ");
				 PRINT_FIELD_U(st, tx_aborted_errors);
				 printf(", ");
				 PRINT_FIELD_U(st, tx_carrier_errors);
				 printf(", ");
				 PRINT_FIELD_U(st, tx_fifo_errors);
				 printf(", ");
				 PRINT_FIELD_U(st, tx_heartbeat_errors);
				 printf(", ");
				 PRINT_FIELD_U(st, tx_window_errors);
				 printf(", ");
				 PRINT_FIELD_U(st, rx_compressed);
				 printf(", ");
				 PRINT_FIELD_U(st, tx_compressed);
				 printf(", ");
				 PRINT_FIELD_U(st, rx_nohandler);
			   printf("}"));

	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_ifinfomsg, print_ifinfomsg,
		    IFLA_STATS, sizeof_stats, &st, sizeof_stats,
		    printf("{");
		    PRINT_FIELD_U(st, rx_packets);
		    printf(", ");
		    PRINT_FIELD_U(st, tx_packets);
		    printf(", ");
		    PRINT_FIELD_U(st, rx_bytes);
		    printf(", ");
		    PRINT_FIELD_U(st, tx_bytes);
		    printf(", ");
		    PRINT_FIELD_U(st, rx_errors);
		    printf(", ");
		    PRINT_FIELD_U(st, tx_errors);
		    printf(", ");
		    PRINT_FIELD_U(st, rx_dropped);
		    printf(", ");
		    PRINT_FIELD_U(st, tx_dropped);
		    printf(", ");
		    PRINT_FIELD_U(st, multicast);
		    printf(", ");
		    PRINT_FIELD_U(st, collisions);
		    printf(", ");
		    PRINT_FIELD_U(st, rx_length_errors);
		    printf(", ");
		    PRINT_FIELD_U(st, rx_over_errors);
		    printf(", ");
		    PRINT_FIELD_U(st, rx_crc_errors);
		    printf(", ");
		    PRINT_FIELD_U(st, rx_frame_errors);
		    printf(", ");
		    PRINT_FIELD_U(st, rx_fifo_errors);
		    printf(", ");
		    PRINT_FIELD_U(st, rx_missed_errors);
		    printf(", ");
		    PRINT_FIELD_U(st, tx_aborted_errors);
		    printf(", ");
		    PRINT_FIELD_U(st, tx_carrier_errors);
		    printf(", ");
		    PRINT_FIELD_U(st, tx_fifo_errors);
		    printf(", ");
		    PRINT_FIELD_U(st, tx_heartbeat_errors);
		    printf(", ");
		    PRINT_FIELD_U(st, tx_window_errors);
		    printf(", ");
		    PRINT_FIELD_U(st, rx_compressed);
		    printf(", ");
		    PRINT_FIELD_U(st, tx_compressed);
		    printf("}"));

	static const struct rtnl_link_ifmap map = {
		.mem_start = 0xadcbefedefbcdedb,
		.mem_end = 0xefcbeabdecdcdefa,
		.base_addr = 0xaddbeabdfaacdbae,
		.irq = 0xefaf,
		.dma = 0xab,
		.port = 0xcd
	};
	const unsigned int sizeof_ifmap =
		offsetofend(struct rtnl_link_ifmap, port);
	const unsigned int plen = sizeof_ifmap - 1 > DEFAULT_STRLEN
				  ? DEFAULT_STRLEN
				  : (int) sizeof_ifmap - 1;
	/* len < sizeof_ifmap */
	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_ifinfomsg, print_ifinfomsg,
		    IFLA_MAP, plen, pattern, plen,
		    print_quoted_hex(pattern, plen));

	/* short read of sizeof_ifmap */
	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_ifinfomsg, print_ifinfomsg,
		    IFLA_MAP, sizeof_ifmap, &map, sizeof_ifmap - 1,
		    printf("%p", RTA_DATA(TEST_NLATTR_nla)));

	/* sizeof_ifmap */
	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_ifinfomsg, print_ifinfomsg,
		    IFLA_MAP, sizeof_ifmap, &map, sizeof_ifmap,
		    printf("{");
		    PRINT_FIELD_X(map, mem_start);
		    printf(", ");
		    PRINT_FIELD_X(map, mem_end);
		    printf(", ");
		    PRINT_FIELD_X(map, base_addr);
		    printf(", ");
		    PRINT_FIELD_U(map, irq);
		    printf(", ");
		    PRINT_FIELD_U(map, dma);
		    printf(", ");
		    PRINT_FIELD_U(map, port);
		    printf("}"));

	static const struct rtnl_link_stats64 st64 = {
		.rx_packets = 0xadcbefedefbcdedb,
		.tx_packets = 0xbdabdedabdcdeabd,
		.rx_bytes = 0xcdbaefbaeadfabec,
		.tx_bytes = 0xdbaedbafabbeacdb,
		.rx_errors = 0xefabfdaefabaefab,
		.tx_errors = 0xfaebfabfabbaeabf,
		.rx_dropped = 0xacdbaedbadbabeba,
		.tx_dropped = 0xbcdeffebdabeadbe,
		.multicast = 0xeeffbaeabaeffabe,
		.collisions = 0xffbaefcefbafacef,
		.rx_length_errors = 0xaabbdeabceffdecb,
		.rx_over_errors = 0xbbdcdadebadeaeed,
		.rx_crc_errors= 0xccdeabecefaedbef,
		.rx_frame_errors = 0xddbedaedebcedaef,
		.rx_fifo_errors = 0xeffbadefafdaeaab,
		.rx_missed_errors = 0xfefaebccceadeecd,
		.tx_aborted_errors = 0xabcdadefcdadef,
		.tx_carrier_errors = 0xbccdafaeeaaefe,
		.tx_fifo_errors = 0xcddefdbedeadce,
		.tx_heartbeat_errors = 0xedaededdadcdea,
		.tx_window_errors = 0xfdacdeaccedcda,
		.rx_compressed = 0xacdbbcacdbccef,
		.tx_compressed = 0xbcdadefcdedfea,
		.rx_nohandler = 0xcbdbacbfbafffd,
		.rx_otherhost_dropped = 0xbefdafcfeeadcbfb
	};
	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_ifinfomsg, print_ifinfomsg,
			   IFLA_STATS64, pattern, st64,
			   printf("{");
			   PRINT_FIELD_U(st64, rx_packets);
			   printf(", ");
			   PRINT_FIELD_U(st64, tx_packets);
			   printf(", ");
			   PRINT_FIELD_U(st64, rx_bytes);
			   printf(", ");
			   PRINT_FIELD_U(st64, tx_bytes);
			   printf(", ");
			   PRINT_FIELD_U(st64, rx_errors);
			   printf(", ");
			   PRINT_FIELD_U(st64, tx_errors);
			   printf(", ");
			   PRINT_FIELD_U(st64, rx_dropped);
			   printf(", ");
			   PRINT_FIELD_U(st64, tx_dropped);
			   printf(", ");
			   PRINT_FIELD_U(st64, multicast);
			   printf(", ");
			   PRINT_FIELD_U(st64, collisions);
			   printf(", ");
			   PRINT_FIELD_U(st64, rx_length_errors);
			   printf(", ");
			   PRINT_FIELD_U(st64, rx_over_errors);
			   printf(", ");
			   PRINT_FIELD_U(st64, rx_crc_errors);
			   printf(", ");
			   PRINT_FIELD_U(st64, rx_frame_errors);
			   printf(", ");
			   PRINT_FIELD_U(st64, rx_fifo_errors);
			   printf(", ");
			   PRINT_FIELD_U(st64, rx_missed_errors);
			   printf(", ");
			   PRINT_FIELD_U(st64, tx_aborted_errors);
			   printf(", ");
			   PRINT_FIELD_U(st64, tx_carrier_errors);
			   printf(", ");
			   PRINT_FIELD_U(st64, tx_fifo_errors);
			   printf(", ");
			   PRINT_FIELD_U(st64, tx_heartbeat_errors);
			   printf(", ");
			   PRINT_FIELD_U(st64, tx_window_errors);
			   printf(", ");
			   PRINT_FIELD_U(st64, rx_compressed);
			   printf(", ");
			   PRINT_FIELD_U(st64, tx_compressed);
			   printf(", ");
			   PRINT_FIELD_U(st64, rx_nohandler);
			   printf(", ");
			   PRINT_FIELD_U(st64, rx_otherhost_dropped);
			   printf("}"));

	const unsigned int stats64_rx_nohandler_size =
		offsetofend(struct rtnl_link_stats64, rx_nohandler);
	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_ifinfomsg, print_ifinfomsg,
		    IFLA_STATS64, stats64_rx_nohandler_size,
		    &st64, stats64_rx_nohandler_size,
		    printf("{");
		    PRINT_FIELD_U(st64, rx_packets);
		    printf(", ");
		    PRINT_FIELD_U(st64, tx_packets);
		    printf(", ");
		    PRINT_FIELD_U(st64, rx_bytes);
		    printf(", ");
		    PRINT_FIELD_U(st64, tx_bytes);
		    printf(", ");
		    PRINT_FIELD_U(st64, rx_errors);
		    printf(", ");
		    PRINT_FIELD_U(st64, tx_errors);
		    printf(", ");
		    PRINT_FIELD_U(st64, rx_dropped);
		    printf(", ");
		    PRINT_FIELD_U(st64, tx_dropped);
		    printf(", ");
		    PRINT_FIELD_U(st64, multicast);
		    printf(", ");
		    PRINT_FIELD_U(st64, collisions);
		    printf(", ");
		    PRINT_FIELD_U(st64, rx_length_errors);
		    printf(", ");
		    PRINT_FIELD_U(st64, rx_over_errors);
		    printf(", ");
		    PRINT_FIELD_U(st64, rx_crc_errors);
		    printf(", ");
		    PRINT_FIELD_U(st64, rx_frame_errors);
		    printf(", ");
		    PRINT_FIELD_U(st64, rx_fifo_errors);
		    printf(", ");
		    PRINT_FIELD_U(st64, rx_missed_errors);
		    printf(", ");
		    PRINT_FIELD_U(st64, tx_aborted_errors);
		    printf(", ");
		    PRINT_FIELD_U(st64, tx_carrier_errors);
		    printf(", ");
		    PRINT_FIELD_U(st64, tx_fifo_errors);
		    printf(", ");
		    PRINT_FIELD_U(st64, tx_heartbeat_errors);
		    printf(", ");
		    PRINT_FIELD_U(st64, tx_window_errors);
		    printf(", ");
		    PRINT_FIELD_U(st64, rx_compressed);
		    printf(", ");
		    PRINT_FIELD_U(st64, tx_compressed);
		    printf(", ");
		    PRINT_FIELD_U(st64, rx_nohandler);
		    printf("}"));

	const unsigned int stats64_tx_compressed_size =
		offsetofend(struct rtnl_link_stats64, tx_compressed);
	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_ifinfomsg, print_ifinfomsg,
		    IFLA_STATS64, stats64_tx_compressed_size,
		    &st64, stats64_tx_compressed_size,
		    printf("{");
		    PRINT_FIELD_U(st64, rx_packets);
		    printf(", ");
		    PRINT_FIELD_U(st64, tx_packets);
		    printf(", ");
		    PRINT_FIELD_U(st64, rx_bytes);
		    printf(", ");
		    PRINT_FIELD_U(st64, tx_bytes);
		    printf(", ");
		    PRINT_FIELD_U(st64, rx_errors);
		    printf(", ");
		    PRINT_FIELD_U(st64, tx_errors);
		    printf(", ");
		    PRINT_FIELD_U(st64, rx_dropped);
		    printf(", ");
		    PRINT_FIELD_U(st64, tx_dropped);
		    printf(", ");
		    PRINT_FIELD_U(st64, multicast);
		    printf(", ");
		    PRINT_FIELD_U(st64, collisions);
		    printf(", ");
		    PRINT_FIELD_U(st64, rx_length_errors);
		    printf(", ");
		    PRINT_FIELD_U(st64, rx_over_errors);
		    printf(", ");
		    PRINT_FIELD_U(st64, rx_crc_errors);
		    printf(", ");
		    PRINT_FIELD_U(st64, rx_frame_errors);
		    printf(", ");
		    PRINT_FIELD_U(st64, rx_fifo_errors);
		    printf(", ");
		    PRINT_FIELD_U(st64, rx_missed_errors);
		    printf(", ");
		    PRINT_FIELD_U(st64, tx_aborted_errors);
		    printf(", ");
		    PRINT_FIELD_U(st64, tx_carrier_errors);
		    printf(", ");
		    PRINT_FIELD_U(st64, tx_fifo_errors);
		    printf(", ");
		    PRINT_FIELD_U(st64, tx_heartbeat_errors);
		    printf(", ");
		    PRINT_FIELD_U(st64, tx_window_errors);
		    printf(", ");
		    PRINT_FIELD_U(st64, rx_compressed);
		    printf(", ");
		    PRINT_FIELD_U(st64, tx_compressed);
		    printf("}"));

	struct nlattr nla = {
		.nla_len = sizeof(nla),
		.nla_type = IFLA_INFO_KIND,
	};
	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_ifinfomsg, print_ifinfomsg,
		    IFLA_LINKINFO, sizeof(nla), &nla, sizeof(nla),
		    printf("{nla_len=%u, nla_type=IFLA_INFO_KIND}",
			   nla.nla_len));

	/* IFLA_VF_PORTS */
	nla.nla_type = IFLA_VF_PORT;
	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_ifinfomsg, print_ifinfomsg,
		    IFLA_VF_PORTS, sizeof(nla), &nla, sizeof(nla),
		    printf("{nla_len=%u, nla_type=IFLA_VF_PORT}",
			   nla.nla_len));

	/* IFLA_EXT_MASK */
	static const struct strval32 ifla_ext_masks[] = {
		{ ARG_STR(0) },
		{ ARG_STR(RTEXT_FILTER_VF) },
		{ ARG_STR(0xdeface80) " /* RTEXT_FILTER_??? */" },
		{ 0xdeadfeed, "RTEXT_FILTER_VF|RTEXT_FILTER_BRVLAN_COMPRESSED"
			      "|RTEXT_FILTER_SKIP_STATS|RTEXT_FILTER_CFM_CONFIG"
			      "|RTEXT_FILTER_CFM_STATUS|0xdeadfe80" },
	};
	for (size_t i = 0; i < ARRAY_SIZE(ifla_ext_masks); i++) {
		TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
				   init_ifinfomsg, print_ifinfomsg,
				   IFLA_EXT_MASK, pattern,
				   ifla_ext_masks[i].val,
				   printf("%s", ifla_ext_masks[i].str));
	}

	/* IFLA_EVENT */
	static const struct strval32 ifla_events[] = {
		{ 0, "IFLA_EVENT_NONE" },
		{ 6, "IFLA_EVENT_BONDING_OPTIONS" },
		{ ARG_STR(0x7) " /* IFLA_EVENT_??? */" },
		{ ARG_STR(0xdeadfeed) " /* IFLA_EVENT_??? */" },
	};
	for (size_t i = 0; i < ARRAY_SIZE(ifla_events); i++) {
		TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
				   init_ifinfomsg, print_ifinfomsg,
				   IFLA_EVENT, pattern, ifla_events[i].val,
				   printf("%s", ifla_events[i].str));
	}

	/* IFLA_PROP_LIST */
	struct {
		char p1[20];
	} buf;
	fill_memory(&buf, sizeof(buf));
	TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
				      init_prop_list_msg, print_prop_list_msg,
				      IFLA_ALT_IFNAME, "IFLA_ALT_IFNAME",
				      pattern, buf, print_quoted_stringn, 1,
				      print_quoted_memory(&buf, sizeof(buf));
				      printf("..."));

	/* IFLA_ALT_IFNAME, IFLA_PARENT_DEV_NAME, IFLA_PARENT_DEV_BUS_NAME */
	static const char str[] = "OH HAI THAR\r\n\t\377\0\v\x7e";
	static const struct strval32 attrs[] = {
		{ ARG_STR(IFLA_ALT_IFNAME) },
		{ ARG_STR(IFLA_PARENT_DEV_NAME) },
		{ ARG_STR(IFLA_PARENT_DEV_BUS_NAME) },
	};
	for (size_t i = 0; i < ARRAY_SIZE(attrs); i++) {
		TEST_NLATTR_(fd, nlh0, hdrlen,
			     init_ifinfomsg, print_ifinfomsg,
			     attrs[i].val, attrs[i].str,
			     sizeof(str), str, sizeof(str),
			     print_quoted_memory(str, sizeof(str) - 1));

		TEST_NLATTR_(fd, nlh0, hdrlen,
			     init_ifinfomsg, print_ifinfomsg,
			     attrs[i].val, attrs[i].str,
			     sizeof(str) - 1, str, sizeof(str) - 1,
			     print_quoted_memory(str, sizeof(str) - 1);
			     printf("..."));
	}

	puts("+++ exited with 0 +++");
	return 0;
}
