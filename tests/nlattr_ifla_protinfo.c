/*
 * Check decoding of IFLA_PROTINFO netlink attribute.
 *
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2021 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2017-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <inttypes.h>
#include <netinet/in.h>
#include <linux/if_arp.h>
#include <linux/if_link.h>
#include <linux/rtnetlink.h>
#include <stdio.h>

#include "test_nlattr.h"

#include "xlat.h"
#include "xlat/addrfams.h"

#include "nlattr_ifla_af_inet6.h"

static const unsigned int hdrlen = sizeof(struct ifinfomsg);

static uint16_t af;
static const char *af_str;

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
		.ifi_family = af,
		.ifi_type = ARPHRD_LOOPBACK,
		.ifi_index = ifindex_lo(),
		.ifi_flags = IFF_UP,
	);
}

static void
init_ifinfomsg_protinfo(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	init_ifinfomsg(nlh, msg_len);

	struct nlattr *const nla = NLMSG_ATTR(nlh, sizeof(struct ifinfomsg));
	SET_STRUCT(struct nlattr, nla,
		.nla_len = msg_len - NLMSG_SPACE(hdrlen),
		.nla_type = IFLA_PROTINFO,
	);
}

static void
print_ifinfomsg(const unsigned int msg_len)
{
	printf("{nlmsg_len=%u, nlmsg_type=" XLAT_FMT ", nlmsg_flags=" XLAT_FMT
	       ", nlmsg_seq=0, nlmsg_pid=0}, {ifi_family=%s"
	       ", ifi_type=" XLAT_FMT ", ifi_index=" XLAT_FMT_U
	       ", ifi_flags=" XLAT_FMT ", ifi_change=0}",
	       msg_len, XLAT_ARGS(RTM_GETLINK), XLAT_ARGS(NLM_F_DUMP),
	       af_str, XLAT_ARGS(ARPHRD_LOOPBACK),
	       XLAT_SEL(ifindex_lo(), IFINDEX_LO_STR), XLAT_ARGS(IFF_UP));
}

static void
print_ifinfomsg_protinfo(const unsigned int msg_len)
{
	print_ifinfomsg(msg_len);
	printf(", [{nla_len=%u, nla_type=" XLAT_FMT "}",
	       msg_len - NLMSG_SPACE(hdrlen), XLAT_ARGS(IFLA_PROTINFO));
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	const int fd = create_nl_socket(NETLINK_ROUTE);
	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen),
				   NLA_HDRLEN * 2 + 42);

	static char buf[256];
	fill_memory_ex(buf, sizeof(buf), 32, 224);

	/* Unsupported address families */
	static const uint8_t skip_af[] = { AF_BRIDGE, AF_INET6 };
	size_t pos = 0;
	for (size_t i = 0; i < 256; i++) {
		if (i == skip_af[pos]) {
			pos += 1;
			continue;
		}

		af = i;
		af_str = sprintxval(addrfams, i, "AF_???");
		TEST_NLATTR_(fd, nlh0, hdrlen, init_ifinfomsg, print_ifinfomsg,
			     IFLA_PROTINFO, XLAT_STR(IFLA_PROTINFO),
			     42, buf, 42,
			     print_quoted_hex(buf, 32);
			     printf("..."));
	}

	/* AF_BRIDGE is handled by nlattr_ifla_brport */

	/* AF_INET6 */
	static char af_inet6_str[20];
	af = AF_INET6;
	snprintf(af_inet6_str, sizeof(af_inet6_str), XLAT_FMT,
		 XLAT_ARGS(AF_INET6));
	af_str = af_inet6_str;
	check_ifla_af_inet6(fd, nlh0, hdrlen,
			    init_ifinfomsg_protinfo, print_ifinfomsg_protinfo,
			    buf, 1);

	puts("+++ exited with 0 +++");
	return 0;
}
