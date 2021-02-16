/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include "test_nlattr.h"
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/if_link.h>
#include <linux/rtnetlink.h>

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

	static const struct ifla_port_vsi vsi = {
		.vsi_mgr_id = 0xab,
		.vsi_type_id = "abc",
		.vsi_type_version = 0xef
	};
	TEST_NESTED_NLATTR_OBJECT(fd, nlh0, hdrlen,
				  init_ifinfomsg, print_ifinfomsg,
				  IFLA_PORT_VSI_TYPE, pattern, vsi,
				  printf("{");
				  PRINT_FIELD_U(vsi, vsi_mgr_id);
				  printf(", vsi_type_id=\"\\x61\\x62\\x63\"");
				  printf(", ");
				  PRINT_FIELD_U(vsi, vsi_type_version);
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
				  printf("{");
				  PRINT_FIELD_U(vsi2, vsi_mgr_id);
				  printf(", vsi_type_id=\"\\x0a\\x00\\xff\"");
				  printf(", ");
				  PRINT_FIELD_U(vsi2, vsi_type_version);
				  printf(", pad=\"\\x00\\x01\\x02\"");
				  printf("}"));

	puts("+++ exited with 0 +++");
	return 0;
}
