/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include "test_nlattr.h"
#include <linux/if.h>
#include <linux/if_arp.h>
#ifdef HAVE_LINUX_IF_LINK_H
# include <linux/if_link.h>
#endif
#include <linux/rtnetlink.h>

#if !HAVE_DECL_IFLA_PORT_SELF
enum { IFLA_PORT_SELF = 25 };
#endif
#ifndef IFLA_PORT_VF
# define IFLA_PORT_VF 1
#endif

#define IFLA_ATTR IFLA_PORT_SELF
#include "nlattr_ifla.h"

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	const int fd = create_nl_socket(NETLINK_ROUTE);
	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen), 2 * NLA_HDRLEN + 8);

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

	const uint32_t num = 0xabacdbcd;
	TEST_NESTED_NLATTR_OBJECT(fd, nlh0, hdrlen,
				  init_ifinfomsg, print_ifinfomsg,
				  IFLA_PORT_VF, pattern, num,
				  printf("%u", num));

#ifdef HAVE_STRUCT_IFLA_PORT_VSI
	static const struct ifla_port_vsi vsi = {
		.vsi_mgr_id = 0xab,
		.vsi_type_id = "abc",
		.vsi_type_version = 0xef
	};
	TEST_NESTED_NLATTR_OBJECT(fd, nlh0, hdrlen,
				  init_ifinfomsg, print_ifinfomsg,
				  IFLA_PORT_VSI_TYPE, pattern, vsi,
				  PRINT_FIELD_U("{", vsi, vsi_mgr_id);
				  printf(", vsi_type_id=\"\\x61\\x62\\x63\"");
				  PRINT_FIELD_U(", ", vsi, vsi_type_version);
				  printf("}"));

	static const struct ifla_port_vsi vsi2 = {
		.vsi_mgr_id = 0xab,
		.vsi_type_id = { 10, 0, 255 },
		.vsi_type_version = 0xef,
		.pad = { 0, 1, 2 },
	};
	TEST_NESTED_NLATTR_OBJECT(fd, nlh0, hdrlen,
				  init_ifinfomsg, print_ifinfomsg,
				  IFLA_PORT_VSI_TYPE, pattern, vsi2,
				  PRINT_FIELD_U("{", vsi2, vsi_mgr_id);
				  printf(", vsi_type_id=\"\\x0a\\x00\\xff\"");
				  PRINT_FIELD_U(", ", vsi2, vsi_type_version);
				  printf(", pad=\"\\x00\\x01\\x02\"");
				  printf("}"));
#endif

	puts("+++ exited with 0 +++");
	return 0;
}
