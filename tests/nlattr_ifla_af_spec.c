/*
 * IFLA_AF_SPEC netlink attribute decoding check.
 *
 * Copyright (c) 2018-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <inttypes.h>
#include <stdio.h>
#include <stddef.h>
#include "test_nlattr.h"

#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/if_link.h>
#include <linux/rtnetlink.h>

#include "xlat.h"
#include "xlat/addrfams.h"

#define XLAT_MACROS_ONLY
#include "xlat/rtnl_ifla_af_spec_inet_attrs.h"
#include "xlat/rtnl_ifla_af_spec_inet6_attrs.h"
#undef XLAT_MACROS_ONLY

#define IFLA_ATTR IFLA_AF_SPEC
#include "nlattr_ifla.h"

#include "nlattr_ifla_af_inet6.h"

#define AF_SPEC_FUNCS(family_)						\
	static void							\
	init_##family_##_msg(struct nlmsghdr *const nlh,		\
			     const unsigned int msg_len)		\
	{								\
		init_ifinfomsg(nlh, msg_len);				\
									\
		struct nlattr *nla = NLMSG_ATTR(nlh, hdrlen);		\
		nla += 1;						\
		SET_STRUCT(struct nlattr, nla,				\
			.nla_len = msg_len - NLMSG_SPACE(hdrlen)	\
				  - NLA_HDRLEN,				\
			.nla_type = family_,				\
		);							\
	}								\
									\
	static void							\
	print_##family_##_msg(const unsigned int msg_len)		\
	{								\
		print_ifinfomsg(msg_len);				\
		printf(", [{nla_len=%u, nla_type=" #family_ "}",	\
		       msg_len - NLMSG_SPACE(hdrlen) - NLA_HDRLEN);	\
	}								\
	/* end of AF_SPEC_FUNCS definition */

AF_SPEC_FUNCS(AF_INET)
AF_SPEC_FUNCS(AF_INET6)
AF_SPEC_FUNCS(AF_MCTP)

static void
print_inet_conf_val(uint32_t *val, size_t idx)
{
	static const char * const strs[] = {
		"IPV4_DEVCONF_FORWARDING-1",
		"IPV4_DEVCONF_MC_FORWARDING-1",
	};

	print_arr_val(val, idx, idx < ARRAY_SIZE(strs) ? strs[idx] : NULL);
}

int
main(void)
{
	static const uint8_t unknown_msg[] = { 0xab, 0xac, 0xdb, 0xcd };

	skip_if_unavailable("/proc/self/fd/");

	const int fd = create_nl_socket(NETLINK_ROUTE);

	const unsigned int hdrlen = sizeof(struct ifinfomsg);
	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen), 3 * NLA_HDRLEN + 256);

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);


	/* unknown AF_* */
	static uint8_t skip_afs[] = { AF_INET, AF_INET6, AF_MCTP };
	size_t pos = 0;
	for (size_t i = 0; i < 256; i++) {
		if (pos < ARRAY_SIZE(skip_afs) && skip_afs[pos] == i) {
			pos += 1;
			continue;
		}

		const char *af_str = sprintxval(addrfams, i, "AF_???");
		TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
					   init_ifinfomsg, print_ifinfomsg,
					   i, af_str, pattern, unknown_msg,
					   print_quoted_hex, 1,
					   printf("\"\\xab\\xac\\xdb\\xcd\""));
	}

	/* AF_INET */
	TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
				      init_AF_INET_msg, print_AF_INET_msg,
				      0, "IFLA_INET_UNSPEC", pattern,
				      unknown_msg, print_quoted_hex, 2,
				      printf("\"\\xab\\xac\\xdb\\xcd\""));
	TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
				      init_AF_INET_msg, print_AF_INET_msg,
				      2, "0x2 /* IFLA_INET_??? */", pattern,
				      unknown_msg, print_quoted_hex, 2,
				      printf("\"\\xab\\xac\\xdb\\xcd\""));

	/* AF_INET: IFLA_INET_CONF */
	uint32_t inet_conf_vals[] = { 0xdeadc0de, 0xda7aface };
	TEST_NESTED_NLATTR_ARRAY_EX(fd, nlh0, hdrlen,
				    init_AF_INET_msg, print_AF_INET_msg,
				    IFLA_INET_CONF, pattern,
				    inet_conf_vals, 2, print_inet_conf_val);

	/* AF_INET6 */
	check_ifla_af_inet6(fd, nlh0, hdrlen,
			    init_AF_INET6_msg, print_AF_INET6_msg, pattern, 2);

	/* AF_MCTP */
	TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
				      init_AF_MCTP_msg, print_AF_MCTP_msg,
				      0, "IFLA_MCTP_UNSPEC", pattern,
				      unknown_msg, print_quoted_hex, 2,
				      printf("\"\\xab\\xac\\xdb\\xcd\""));
	TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
				      init_AF_MCTP_msg, print_AF_MCTP_msg,
				      2, "0x2 /* IFLA_MCTP_??? */", pattern,
				      unknown_msg, print_quoted_hex, 2,
				      printf("\"\\xab\\xac\\xdb\\xcd\""));

	/* AF_MCTP: IFLA_MCTP_NET */
	check_u32_nlattr(fd, nlh0, hdrlen, init_AF_MCTP_msg, print_AF_MCTP_msg,
			 1, "IFLA_MCTP_NET", pattern, 2);

	puts("+++ exited with 0 +++");
	return 0;
}
