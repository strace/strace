/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2024 The strace developers.
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

#define XLAT_MACROS_ONLY
# include "xlat/rtnl_ifla_proto_down_reason_attrs.h"
#undef XLAT_MACROS_ONLY

#define IFLA_ATTR IFLA_PROTO_DOWN_REASON
#include "nlattr_ifla.h"

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	const union {
		uint8_t  bytes[4];
		uint32_t num;
	} num = { .num=0xabacdbcd };
	const int fd = create_nl_socket(NETLINK_ROUTE);
	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen),
				   NLA_HDRLEN + sizeof(num));

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

	static const struct strval8 inv_attrs[] = {
		{ ARG_STR(IFLA_PROTO_DOWN_REASON_UNSPEC) },
		{ ARG_STR(0x3)  " /* IFLA_PROTO_DOWN_REASON_??? */" },
		{ ARG_STR(0xfe) " /* IFLA_PROTO_DOWN_REASON_??? */" },
	};

	for (size_t i = 0; i < ARRAY_SIZE(inv_attrs); i++) {
		TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
					      init_ifinfomsg, print_ifinfomsg,
					      inv_attrs[i].val,
					      inv_attrs[i].str,
					      pattern, num,
					      print_quoted_hex, 1,
					      printf("\"\\x%x\\x%x\\x%x\\x%x\"",
						     num.bytes[0], num.bytes[1],
						     num.bytes[2], num.bytes[3])
					      );
	}

	static const struct strval32 attrs[] = {
		{ ARG_STR(IFLA_PROTO_DOWN_REASON_MASK) },
		{ ARG_STR(IFLA_PROTO_DOWN_REASON_VALUE) },
	};

	for (size_t i = 0; i < ARRAY_SIZE(attrs); i++) {
		TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
					      init_ifinfomsg, print_ifinfomsg,
					      attrs[i].val, attrs[i].str,
					      pattern, num,
					      print_quoted_hex, 1,
					      printf("%#x", num.num));
	}

	puts("+++ exited with 0 +++");
	return 0;
}
