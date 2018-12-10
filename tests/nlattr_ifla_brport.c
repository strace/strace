/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2018 The strace developers.
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
#ifdef HAVE_LINUX_IF_LINK_H
# include <linux/if_link.h>
#endif
#include <linux/rtnetlink.h>

#if !HAVE_DECL_IFLA_PROTINFO
enum { IFLA_PROTINFO = 12 };
#endif

#define IFLA_BRPORT_PRIORITY 2
#define IFLA_BRPORT_MESSAGE_AGE_TIMER 21

#define IFLA_ATTR IFLA_PROTINFO
#include "nlattr_ifla.h"

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	const uint16_t u16 = 0xabcd;
	const uint64_t u64 = 0xabcdedeeefeafeab;
	const int fd = create_nl_socket(NETLINK_ROUTE);
	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen),
				   NLA_HDRLEN * 2 + sizeof(u64));

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

	TEST_NESTED_NLATTR_OBJECT(fd, nlh0, hdrlen,
				  init_ifinfomsg, print_ifinfomsg,
				  IFLA_BRPORT_PRIORITY, pattern, u16,
				  printf("%u", u16));

	TEST_NESTED_NLATTR_OBJECT(fd, nlh0, hdrlen,
				  init_ifinfomsg, print_ifinfomsg,
				  IFLA_BRPORT_MESSAGE_AGE_TIMER, pattern, u64,
				  printf("%" PRIu64, u64));

#ifdef HAVE_STRUCT_IFLA_BRIDGE_ID
	static const struct ifla_bridge_id id = {
		.prio = { 0xab, 0xcd },
		.addr = { 0xab, 0xcd, 0xef, 0xac, 0xbc, 0xcd }
	};
	TEST_NESTED_NLATTR_OBJECT(fd, nlh0, hdrlen,
				  init_ifinfomsg, print_ifinfomsg,
				  IFLA_BRPORT_ROOT_ID, pattern, id,
				  printf("{prio=[%u, %u]"
					 ", addr=%02x:%02x:%02x:%02x:%02x:%02x}",
					 id.prio[0], id.prio[1],
					 id.addr[0], id.addr[1], id.addr[2],
					 id.addr[3], id.addr[4], id.addr[5]));
#endif

	puts("+++ exited with 0 +++");
	return 0;
}
