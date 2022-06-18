/*
 * IFLA_AF_SPEC netlink attribute decoding check.
 *
 * Copyright (c) 2018-2022 The strace developers.
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
#include <linux/if_bridge.h>
#include <linux/if_link.h>
#include <linux/rtnetlink.h>

#include "xlat.h"
#include "xlat/addrfams.h"

#define XLAT_MACROS_ONLY
#include "xlat/rtnl_ifla_af_spec_inet_attrs.h"
#include "xlat/rtnl_ifla_af_spec_inet6_attrs.h"
#undef XLAT_MACROS_ONLY

static uint8_t msg_af = AF_UNIX;
static char msg_af_str[32] = "AF_UNIX";

#define IFLA_AF msg_af
#define IFLA_AF_STR msg_af_str
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

AF_SPEC_FUNCS(IFLA_BRIDGE_VLAN_TUNNEL_INFO)

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
	static uint8_t skip_afs_msg[] = { AF_BRIDGE };
	size_t pos2 = 0;
	for (size_t j = 0; j < 64; j++) {
		if (pos2 < ARRAY_SIZE(skip_afs_msg) && skip_afs_msg[pos2] == j)
		{
			pos2 += 1;
			continue;
		}

		msg_af = j;
		msg_af_str[0] = '\0';
		strncat(msg_af_str, sprintxval(addrfams, j, "AF_???"),
			sizeof(msg_af_str) - 1);
		pos = 0;

		for (size_t i = 0; i < 64; i++) {
			if (pos < ARRAY_SIZE(skip_afs) && skip_afs[pos] == i) {
				pos += 1;
				continue;
			}

			const char *af_str = sprintxval(addrfams, i, "AF_???");
			TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
						   init_ifinfomsg,
						   print_ifinfomsg,
						   i, af_str, pattern,
						   unknown_msg,
						   print_quoted_hex, 1,
						   printf("\"\\xab\\xac\\xdb"
							  "\\xcd\""));
		}
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

	/* AF_BRIDGE */
	msg_af = AF_BRIDGE;
	strcpy(msg_af_str, "AF_BRIDGE");

	/* AF_BRIDGE: unknown, unimplemented */
	static const struct strval16 unk_attrs[] = {
		{ ENUM_KNOWN(0x4, IFLA_BRIDGE_MRP) },
		{ ENUM_KNOWN(0x5, IFLA_BRIDGE_CFM) },
		{ ENUM_KNOWN(0x6, IFLA_BRIDGE_MST) },
		{ ARG_XLAT_UNKNOWN(0x7, "IFLA_BRIDGE_???") },
		{ ARG_XLAT_UNKNOWN(0xbad, "IFLA_BRIDGE_???") },
	};
	for (size_t i = 0; i < ARRAY_SIZE(unk_attrs); i++) {
		TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
					      init_ifinfomsg, print_ifinfomsg,
					      unk_attrs[i].val,
					      unk_attrs[i].str,
					      pattern, unknown_msg,
					      print_quoted_hex, 1,
					      printf("\"\\xab\\xac\\xdb\\xcd\"")
					      );
	}

	/* AF_BRIDGE: IFLA_BRIDGE_FLAGS */
	static const struct strval16 bridge_flags[] = {
		{ ARG_STR(0) },
		{ ARG_STR(BRIDGE_FLAGS_MASTER) },
		{ ARG_STR(BRIDGE_FLAGS_SELF) },
		{ ARG_STR(BRIDGE_FLAGS_MASTER|BRIDGE_FLAGS_SELF) },
		{ ARG_STR(0x4) " /* BRIDGE_FLAGS_??? */" },
		{ 0xcafe, "BRIDGE_FLAGS_SELF|0xcafc" },
		{ ARG_STR(0x7eac) " /* BRIDGE_FLAGS_??? */" },
	};
	for (size_t i = 0; i < ARRAY_SIZE(bridge_flags); i++) {
		TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
					      init_ifinfomsg, print_ifinfomsg,
					      IFLA_BRIDGE_FLAGS,
					      "IFLA_BRIDGE_FLAGS",
					      pattern, bridge_flags[i].val,
					      print_quoted_hex, 1,
					      printf("%s", bridge_flags[i].str)
					      );
	}

	/* AF_BRIDGE: IFLA_BRIDGE_MODE */
	static const struct strval16 bridge_modes[] = {
		{ ARG_STR(BRIDGE_MODE_VEB) },
		{ ARG_STR(BRIDGE_MODE_VEPA) },
		{ ARG_STR(0x2) " /* BRIDGE_MODE_??? */" },
		{ ARG_STR(0x3) " /* BRIDGE_MODE_??? */" },
		{ ARG_STR(0xcafe) " /* BRIDGE_MODE_??? */" },
		{ ARG_STR(0xfffe) " /* BRIDGE_MODE_??? */" },
		{ ARG_STR(BRIDGE_MODE_UNDEF) },
	};
	for (size_t i = 0; i < ARRAY_SIZE(bridge_flags); i++) {
		TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
					      init_ifinfomsg, print_ifinfomsg,
					      IFLA_BRIDGE_MODE,
					      "IFLA_BRIDGE_MODE",
					      pattern, bridge_modes[i].val,
					      print_quoted_hex, 1,
					      printf("%s", bridge_modes[i].str)
					      );
	}

	/* AF_BRIDGE: IFLA_BRIDGE_VLAN_INFO */
	static const struct {
		struct bridge_vlan_info val;
		const char *str;
	} bridge_vis[] = {
		{ { 0, 0 }, "{flags=0, vid=0}" },
		{ { 1, 1 }, "{flags=BRIDGE_VLAN_INFO_MASTER, vid=1}" },
		{ { 0x69, 0xface },
		  "{flags=BRIDGE_VLAN_INFO_MASTER|BRIDGE_VLAN_INFO_RANGE_BEGIN"
		  "|BRIDGE_VLAN_INFO_BRENTRY|BRIDGE_VLAN_INFO_ONLY_OPTS"
		  ", vid=64206}" },
		{ {0xef80, 0xfeed },
		  "{flags=0xef80 /* BRIDGE_VLAN_INFO_??? */, vid=65261}" },
		{ {0xcafe, 0xdead },
		  "{flags=BRIDGE_VLAN_INFO_PVID|BRIDGE_VLAN_INFO_UNTAGGED"
		  "|BRIDGE_VLAN_INFO_RANGE_BEGIN|BRIDGE_VLAN_INFO_RANGE_END"
		  "|BRIDGE_VLAN_INFO_BRENTRY|BRIDGE_VLAN_INFO_ONLY_OPTS|0xca80"
		  ", vid=57005}" },
	};
	char bvi_buf[12];

	fill_memory_ex(bvi_buf, sizeof(bvi_buf), 'z', 0x80);

	for (size_t i = 0; i < ARRAY_SIZE(bridge_vis); i++) {
		TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
					      init_ifinfomsg, print_ifinfomsg,
					      IFLA_BRIDGE_VLAN_INFO,
					      "IFLA_BRIDGE_VLAN_INFO",
					      pattern, bridge_vis[i].val,
					      print_quoted_hex, 1,
					      printf("%s", bridge_vis[i].str));

		memcpy(bvi_buf, &bridge_vis[i].val, sizeof(bridge_vis[i].val));
		TEST_NLATTR_(fd, nlh0 - NLA_HDRLEN, hdrlen + NLA_HDRLEN,
			     init_ifinfomsg, print_ifinfomsg,
			     IFLA_BRIDGE_VLAN_INFO, "IFLA_BRIDGE_VLAN_INFO",
			     sizeof(bvi_buf), bvi_buf, sizeof(bvi_buf),
			     printf("%s, \"\\x7e\\x7f\\x80\\x81\\x82"
				    "\\x83\\x84\\x85\"]",
				    bridge_vis[i].str));
	}

	/* AF_BRIDGE: IFLA_BRIDGE_TUNNEL_INFO: unknown, undecoded */
	static const struct strval16 unk_bti_attrs[] = {
		{ ENUM_KNOWN(0, IFLA_BRIDGE_VLAN_TUNNEL_UNSPEC) },
		{ ARG_XLAT_UNKNOWN(0x4, "IFLA_BRIDGE_VLAN_TUNNEL_???") },
		{ ARG_XLAT_UNKNOWN(0xbad, "IFLA_BRIDGE_VLAN_TUNNEL_???") },
	};
	for (size_t i = 0; i < ARRAY_SIZE(unk_bti_attrs); i++) {
		TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
					      init_IFLA_BRIDGE_VLAN_TUNNEL_INFO_msg,
					      print_IFLA_BRIDGE_VLAN_TUNNEL_INFO_msg,
					      unk_bti_attrs[i].val,
					      unk_bti_attrs[i].str,
					      pattern, unknown_msg,
					      print_quoted_hex, 2,
					      printf("\"\\xab\\xac\\xdb\\xcd\"")
					      );
	}

	/* AF_BRIDGE: IFLA_BRIDGE_TUNNEL_INFO: u32 attrs */
	static const struct strval16 u32_bti_attrs[] = {
		{ ENUM_KNOWN(0x1, IFLA_BRIDGE_VLAN_TUNNEL_ID) },
	};
	void *nlh_u32 = midtail_alloc(NLMSG_SPACE(hdrlen),
				      NLA_HDRLEN + sizeof(uint32_t));
	for (size_t i = 0; i < ARRAY_SIZE(u32_bti_attrs); i++) {
		check_u32_nlattr(fd, nlh_u32, hdrlen,
				 init_IFLA_BRIDGE_VLAN_TUNNEL_INFO_msg,
				 print_IFLA_BRIDGE_VLAN_TUNNEL_INFO_msg,
				 u32_bti_attrs[i].val, u32_bti_attrs[i].str,
				 pattern, 2);
	}

	/* AF_BRIDGE: IFLA_BRIDGE_TUNNEL_INFO: u16 attrs */
	static const struct strval16 u16_bti_attrs[] = {
		{ ENUM_KNOWN(0x1, IFLA_BRIDGE_VLAN_TUNNEL_VID) },
	};
	void *nlh_u16 = midtail_alloc(NLMSG_SPACE(hdrlen),
				      NLA_HDRLEN + sizeof(uint16_t));
	for (size_t i = 0; i < ARRAY_SIZE(u16_bti_attrs); i++) {
		check_u16_nlattr(fd, nlh_u16, hdrlen,
				 init_IFLA_BRIDGE_VLAN_TUNNEL_INFO_msg,
				 print_IFLA_BRIDGE_VLAN_TUNNEL_INFO_msg,
				 u16_bti_attrs[i].val, u16_bti_attrs[i].str,
				 pattern, 2);
	}

	/* AF_BRIDGE: IFLA_BRIDGE_TUNNEL_INFO: IFLA_BRIDGE_VLAN_TUNNEL_FLAGS */
	static const struct strval16 bti_flags[] = {
		{ ARG_STR(0) },
		{ ARG_STR(BRIDGE_VLAN_INFO_MASTER) },
		{ ARG_STR(BRIDGE_VLAN_INFO_PVID) },
		{ ARG_STR(BRIDGE_VLAN_INFO_MASTER|BRIDGE_VLAN_INFO_PVID) },
		{ ARG_STR(0xef80) " /* BRIDGE_VLAN_INFO_??? */" },
		{ 0xcafe, "BRIDGE_VLAN_INFO_PVID|BRIDGE_VLAN_INFO_UNTAGGED"
			  "|BRIDGE_VLAN_INFO_RANGE_BEGIN"
			  "|BRIDGE_VLAN_INFO_RANGE_END|BRIDGE_VLAN_INFO_BRENTRY"
			  "|BRIDGE_VLAN_INFO_ONLY_OPTS|0xca80" },
	};
	for (size_t i = 0; i < ARRAY_SIZE(bti_flags); i++) {
		TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
					      init_IFLA_BRIDGE_VLAN_TUNNEL_INFO_msg,
					      print_IFLA_BRIDGE_VLAN_TUNNEL_INFO_msg,
					      IFLA_BRIDGE_VLAN_TUNNEL_FLAGS,
					      "IFLA_BRIDGE_VLAN_TUNNEL_FLAGS",
					      pattern, bti_flags[i].val,
					      print_quoted_hex, 2,
					      printf("%s", bti_flags[i].str)
					      );
	}

	/* AF_INET6 */
	msg_af = AF_UNIX;
	strcpy(msg_af_str, "AF_UNIX");

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
