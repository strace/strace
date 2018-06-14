/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "tests.h"

#include <stdio.h>
#include <stddef.h>
#include "test_nlattr.h"

#include <linux/if.h>
#include <linux/if_arp.h>
#ifdef HAVE_LINUX_IF_LINK_H
# include <linux/if_link.h>
#endif
#include <linux/rtnetlink.h>

#ifndef IFLA_LINKINFO
# define IFLA_LINKINFO 18
#endif
#ifndef IFLA_VF_PORTS
# define IFLA_VF_PORTS 24
#endif
#define IFLA_LINK_NETNSID 37
#define IFLA_EVENT 44

#ifndef IFLA_INFO_KIND
# define IFLA_INFO_KIND 1
#endif

#ifndef IFLA_VF_PORT
# define IFLA_VF_PORT 1
#endif

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
	printf("{len=%u, type=RTM_GETLINK, flags=NLM_F_DUMP"
	       ", seq=0, pid=0}, {ifi_family=AF_UNIX"
	       ", ifi_type=ARPHRD_LOOPBACK"
	       ", ifi_index=" IFINDEX_LO_STR
	       ", ifi_flags=IFF_UP, ifi_change=0}",
	       msg_len);
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
	const unsigned int hdrlen = sizeof(struct ifinfomsg);
	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen),
				   NLA_HDRLEN + sizeof(st));

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

	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_ifinfomsg, print_ifinfomsg,
			   IFLA_STATS, pattern, st,
			   PRINT_FIELD_U("{", st, rx_packets);
			   PRINT_FIELD_U(", ", st, tx_packets);
			   PRINT_FIELD_U(", ", st, rx_bytes);
			   PRINT_FIELD_U(", ", st, tx_bytes);
			   PRINT_FIELD_U(", ", st, rx_errors);
			   PRINT_FIELD_U(", ", st, tx_errors);
			   PRINT_FIELD_U(", ", st, rx_dropped);
			   PRINT_FIELD_U(", ", st, tx_dropped);
			   PRINT_FIELD_U(", ", st, multicast);
			   PRINT_FIELD_U(", ", st, collisions);
			   PRINT_FIELD_U(", ", st, rx_length_errors);
			   PRINT_FIELD_U(", ", st, rx_over_errors);
			   PRINT_FIELD_U(", ", st, rx_crc_errors);
			   PRINT_FIELD_U(", ", st, rx_frame_errors);
			   PRINT_FIELD_U(", ", st, rx_fifo_errors);
			   PRINT_FIELD_U(", ", st, rx_missed_errors);
			   PRINT_FIELD_U(", ", st, tx_aborted_errors);
			   PRINT_FIELD_U(", ", st, tx_carrier_errors);
			   PRINT_FIELD_U(", ", st, tx_fifo_errors);
			   PRINT_FIELD_U(", ", st, tx_heartbeat_errors);
			   PRINT_FIELD_U(", ", st, tx_window_errors);
			   PRINT_FIELD_U(", ", st, rx_compressed);
			   PRINT_FIELD_U(", ", st, tx_compressed);
#ifdef HAVE_STRUCT_RTNL_LINK_STATS_RX_NOHANDLER
			   PRINT_FIELD_U(", ", st, rx_nohandler);
#endif
			   printf("}"));

#ifdef HAVE_STRUCT_RTNL_LINK_STATS_RX_NOHANDLER
	const unsigned int sizeof_stats =
		offsetofend(struct rtnl_link_stats, tx_compressed);
	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_ifinfomsg, print_ifinfomsg,
		    IFLA_STATS, sizeof_stats, &st, sizeof_stats,
		    PRINT_FIELD_U("{", st, rx_packets);
		    PRINT_FIELD_U(", ", st, tx_packets);
		    PRINT_FIELD_U(", ", st, rx_bytes);
		    PRINT_FIELD_U(", ", st, tx_bytes);
		    PRINT_FIELD_U(", ", st, rx_errors);
		    PRINT_FIELD_U(", ", st, tx_errors);
		    PRINT_FIELD_U(", ", st, rx_dropped);
		    PRINT_FIELD_U(", ", st, tx_dropped);
		    PRINT_FIELD_U(", ", st, multicast);
		    PRINT_FIELD_U(", ", st, collisions);
		    PRINT_FIELD_U(", ", st, rx_length_errors);
		    PRINT_FIELD_U(", ", st, rx_over_errors);
		    PRINT_FIELD_U(", ", st, rx_crc_errors);
		    PRINT_FIELD_U(", ", st, rx_frame_errors);
		    PRINT_FIELD_U(", ", st, rx_fifo_errors);
		    PRINT_FIELD_U(", ", st, rx_missed_errors);
		    PRINT_FIELD_U(", ", st, tx_aborted_errors);
		    PRINT_FIELD_U(", ", st, tx_carrier_errors);
		    PRINT_FIELD_U(", ", st, tx_fifo_errors);
		    PRINT_FIELD_U(", ", st, tx_heartbeat_errors);
		    PRINT_FIELD_U(", ", st, tx_window_errors);
		    PRINT_FIELD_U(", ", st, rx_compressed);
		    PRINT_FIELD_U(", ", st, tx_compressed);
		    printf("}"));
#endif /* HAVE_STRUCT_RTNL_LINK_STATS_RX_NOHANDLER */

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
		    PRINT_FIELD_X("{", map, mem_start);
		    PRINT_FIELD_X(", ", map, mem_end);
		    PRINT_FIELD_X(", ", map, base_addr);
		    PRINT_FIELD_U(", ", map, irq);
		    PRINT_FIELD_U(", ", map, dma);
		    PRINT_FIELD_U(", ", map, port);
		    printf("}"));

#ifdef HAVE_STRUCT_RTNL_LINK_STATS64
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
		.tx_compressed = 0xbcdadefcdedfea
	};
	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_ifinfomsg, print_ifinfomsg,
			   IFLA_STATS64, pattern, st64,
			   PRINT_FIELD_U("{", st64, rx_packets);
			   PRINT_FIELD_U(", ", st64, tx_packets);
			   PRINT_FIELD_U(", ", st64, rx_bytes);
			   PRINT_FIELD_U(", ", st64, tx_bytes);
			   PRINT_FIELD_U(", ", st64, rx_errors);
			   PRINT_FIELD_U(", ", st64, tx_errors);
			   PRINT_FIELD_U(", ", st64, rx_dropped);
			   PRINT_FIELD_U(", ", st64, tx_dropped);
			   PRINT_FIELD_U(", ", st64, multicast);
			   PRINT_FIELD_U(", ", st64, collisions);
			   PRINT_FIELD_U(", ", st64, rx_length_errors);
			   PRINT_FIELD_U(", ", st64, rx_over_errors);
			   PRINT_FIELD_U(", ", st64, rx_crc_errors);
			   PRINT_FIELD_U(", ", st64, rx_frame_errors);
			   PRINT_FIELD_U(", ", st64, rx_fifo_errors);
			   PRINT_FIELD_U(", ", st64, rx_missed_errors);
			   PRINT_FIELD_U(", ", st64, tx_aborted_errors);
			   PRINT_FIELD_U(", ", st64, tx_carrier_errors);
			   PRINT_FIELD_U(", ", st64, tx_fifo_errors);
			   PRINT_FIELD_U(", ", st64, tx_heartbeat_errors);
			   PRINT_FIELD_U(", ", st64, tx_window_errors);
			   PRINT_FIELD_U(", ", st64, rx_compressed);
			   PRINT_FIELD_U(", ", st64, tx_compressed);
#ifdef HAVE_STRUCT_RTNL_LINK_STATS64_RX_NOHANDLER
			   PRINT_FIELD_U(", ", st64, rx_nohandler);
#endif
			   printf("}"));

#ifdef HAVE_STRUCT_RTNL_LINK_STATS64_RX_NOHANDLER
	const unsigned int sizeof_stats64 =
		offsetofend(struct rtnl_link_stats64, tx_compressed);
	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_ifinfomsg, print_ifinfomsg,
		    IFLA_STATS64, sizeof_stats64, &st64, sizeof_stats64,
		    PRINT_FIELD_U("{", st64, rx_packets);
		    PRINT_FIELD_U(", ", st64, tx_packets);
		    PRINT_FIELD_U(", ", st64, rx_bytes);
		    PRINT_FIELD_U(", ", st64, tx_bytes);
		    PRINT_FIELD_U(", ", st64, rx_errors);
		    PRINT_FIELD_U(", ", st64, tx_errors);
		    PRINT_FIELD_U(", ", st64, rx_dropped);
		    PRINT_FIELD_U(", ", st64, tx_dropped);
		    PRINT_FIELD_U(", ", st64, multicast);
		    PRINT_FIELD_U(", ", st64, collisions);
		    PRINT_FIELD_U(", ", st64, rx_length_errors);
		    PRINT_FIELD_U(", ", st64, rx_over_errors);
		    PRINT_FIELD_U(", ", st64, rx_crc_errors);
		    PRINT_FIELD_U(", ", st64, rx_frame_errors);
		    PRINT_FIELD_U(", ", st64, rx_fifo_errors);
		    PRINT_FIELD_U(", ", st64, rx_missed_errors);
		    PRINT_FIELD_U(", ", st64, tx_aborted_errors);
		    PRINT_FIELD_U(", ", st64, tx_carrier_errors);
		    PRINT_FIELD_U(", ", st64, tx_fifo_errors);
		    PRINT_FIELD_U(", ", st64, tx_heartbeat_errors);
		    PRINT_FIELD_U(", ", st64, tx_window_errors);
		    PRINT_FIELD_U(", ", st64, rx_compressed);
		    PRINT_FIELD_U(", ", st64, tx_compressed);
		    printf("}"));
#endif /* HAVE_STRUCT_RTNL_LINK_STATS64_RX_NOHANDLER */
#endif /* HAVE_STRUCT_RTNL_LINK_STATS64 */

	struct nlattr nla = {
		.nla_len = sizeof(nla),
		.nla_type = IFLA_INFO_KIND,
	};
	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_ifinfomsg, print_ifinfomsg,
		    IFLA_LINKINFO, sizeof(nla), &nla, sizeof(nla),
		    printf("{nla_len=%u, nla_type=IFLA_INFO_KIND}",
			   nla.nla_len));

	nla.nla_type = IFLA_VF_PORT;
	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_ifinfomsg, print_ifinfomsg,
		    IFLA_VF_PORTS, sizeof(nla), &nla, sizeof(nla),
		    printf("{nla_len=%u, nla_type=IFLA_VF_PORT}",
			   nla.nla_len));

	static const struct {
		uint32_t val;
		const char *str;
	} ifla_events[] = {
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

	puts("+++ exited with 0 +++");
	return 0;
}
