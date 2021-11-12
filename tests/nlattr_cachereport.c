/*
 * Copyright (c) 2021 Eugene Syromyatnikov <evgsyr@gmail.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "test_netlink.h"
#include "test_nlattr.h"

#include <linux/ip.h>
#include <linux/rtnetlink.h>
#include <linux/mroute.h>
#include <linux/mroute6.h>

#include "xlat.h"
#include "xlat/addrfams.h"

static uint8_t af;
static char af_str[256];

/* uses global "af" variable */
static void
init_rtgen(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = msg_len,
		.nlmsg_type = RTM_NEWCACHEREPORT,
		.nlmsg_flags = NLM_F_EXCL|NLM_F_APPEND,
	);

	struct rtgenmsg *const msg = NLMSG_DATA(nlh);
	SET_STRUCT(struct rtgenmsg, msg,
		.rtgen_family = af,
	);
}

static void
print_rtgen(const unsigned int msg_len)
{
	printf("{nlmsg_len=%u, nlmsg_type=" XLAT_FMT ", nlmsg_flags=" XLAT_FMT
	       ", nlmsg_seq=0, nlmsg_pid=0}, {rtgen_family=%s}",
	       msg_len, XLAT_ARGS(RTM_NEWCACHEREPORT),
	       XLAT_ARGS(NLM_F_EXCL|NLM_F_APPEND), af_str);
}

int
main(void)
{
	static const uint8_t unknown_msg[] = { 0xab, 0xac, 0xdb, 0xcd };

	skip_if_unavailable("/proc/self/fd/");

	const int fd = create_nl_socket(NETLINK_ROUTE);

	const unsigned int hdrlen = sizeof(struct rtgenmsg);
	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen), NLA_HDRLEN + 16);

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);


	/* unknown AF_* */
	static uint8_t skip_afs[] = { RTNL_FAMILY_IPMR, RTNL_FAMILY_IP6MR };
	struct {
		struct rtgenmsg ATTRIBUTE_ALIGNED(NLMSG_ALIGNTO) hdr;
		struct {
			char str[sizeof(unknown_msg)];
		} ATTRIBUTE_ALIGNED(NLMSG_ALIGNTO) payload;
	} buf;
	memcpy(buf.payload.str, unknown_msg, sizeof(unknown_msg));
	size_t pos = 0;
	for (size_t i = 0; i < 256; i++) {
		if (pos < ARRAY_SIZE(skip_afs) && skip_afs[pos] == i) {
			pos += 1;
			continue;
		}

		buf.hdr.rtgen_family = i;
		TEST_NETLINK_(fd, nlh0, RTM_NEWCACHEREPORT,
			      XLAT_KNOWN(0x60, "RTM_NEWCACHEREPORT"),
			      NLM_F_REPLACE,
			      XLAT_KNOWN(0x100, "NLM_F_REPLACE"),
			      sizeof(buf), &buf, sizeof(buf),
			      printf("{rtgen_family=%s}"
				     ", \"\\xab\\xac\\xdb\\xcd\"",
				     sprintxval(addrfams, i,
						"RTNL_FAMILY_???")));
	}


	/* RTNL_FAMILY_IPMR */
	af = RTNL_FAMILY_IPMR;
	snprintf(af_str, sizeof(af_str), XLAT_FMT, XLAT_ARGS(RTNL_FAMILY_IPMR));

	/* RTNL_FAMILY_IPMR: unknown, undecoded */
	static const struct strval16 unk_attrs[] = {
		{ ENUM_KNOWN(0, IPMRA_CREPORT_UNSPEC) },
		{ ENUM_KNOWN(0x5, IPMRA_CREPORT_PKT) },
		{ ARG_XLAT_UNKNOWN(0x7, "IPMRA_CREPORT_???") },
		{ ARG_XLAT_UNKNOWN(0x1ead, "IPMRA_CREPORT_???") },
	};

	for (size_t i = 0; i < ARRAY_SIZE(unk_attrs); i++) {
		TEST_NLATTR_(fd, nlh0, hdrlen, init_rtgen, print_rtgen,
			     unk_attrs[i].val, unk_attrs[i].str,
			     16, pattern, 16,
			     print_quoted_hex(pattern, 16));
	}

	/* RTNL_FAMILY_IPMR: IPMRA_CREPORT_MSGTYPE */
	static const struct strval8 mr_msg_types[] = {
		{ ARG_XLAT_UNKNOWN(0, "IGMPMSG_???") },
		{ ARG_XLAT_KNOWN(0x1, "IGMPMSG_NOCACHE") },
		{ ARG_XLAT_KNOWN(0x2, "IGMPMSG_WRONGVIF") },
		{ ARG_XLAT_KNOWN(0x3, "IGMPMSG_WHOLEPKT") },
		{ ARG_XLAT_KNOWN(0x4, "IGMPMSG_WRVIFWHOLE") },
		{ ARG_XLAT_UNKNOWN(0x5, "IGMPMSG_???") },
		{ ARG_XLAT_UNKNOWN(0xca, "IGMPMSG_???") },
	};
	for (size_t i = 0; i < ARRAY_SIZE(mr_msg_types); i++) {
		TEST_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
				       init_rtgen, print_rtgen,
				       IPMRA_CREPORT_MSGTYPE,
				       XLAT_KNOWN(0x1, "IPMRA_CREPORT_MSGTYPE"),
				       pattern, mr_msg_types[i].val, 1,
				       print_quoted_hex,
				       printf("%s", mr_msg_types[i].str));
	}

	/* RTNL_FAMILY_IPMR: u32 */
	static const struct strval16 u32_mr_attrs[] = {
		{ ENUM_KNOWN(0x2, IPMRA_CREPORT_VIF_ID) },
		{ ENUM_KNOWN(0x6, IPMRA_CREPORT_TABLE) },
	};
	void *nlh_u32 = midtail_alloc(NLMSG_SPACE(hdrlen),
				      NLA_HDRLEN + sizeof(uint32_t));
	for (size_t i = 0; i < ARRAY_SIZE(u32_mr_attrs); i++) {
		check_u32_nlattr(fd, nlh_u32, hdrlen, init_rtgen, print_rtgen,
				 u32_mr_attrs[i].val, u32_mr_attrs[i].str,
				 pattern, 0);
	}

	/* RTNL_FAMILY_IPMR: in_addr */
	static const struct strval16 in_addr_attrs[] = {
		{ ENUM_KNOWN(0x3, IPMRA_CREPORT_SRC_ADDR) },
		{ ENUM_KNOWN(0x4, IPMRA_CREPORT_DST_ADDR) },
	};
	static uint32_t ipv4_addr = BE32(0xdeadface);
	for (size_t i = 0; i < ARRAY_SIZE(in_addr_attrs); i++) {
		TEST_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
				       init_rtgen, print_rtgen,
				       in_addr_attrs[i].val,
				       in_addr_attrs[i].str,
				       pattern, ipv4_addr, 4,
				       print_quoted_hex,
				       printf(XLAT_KNOWN_FMT(
					      "\"\\xde\\xad\\xfa\\xce\"",
					      "inet_addr(\"222.173.250.206\")"))
				       );
	}


	/* RTNL_FAMILY_IP6MR */
	af = RTNL_FAMILY_IP6MR;
	snprintf(af_str, sizeof(af_str), XLAT_FMT,
		 XLAT_ARGS(RTNL_FAMILY_IP6MR));

	/* RTNL_FAMILY_IP6MR: unknown, undecoded */
	static const struct strval16 unk6_attrs[] = {
		{ ENUM_KNOWN(0, IP6MRA_CREPORT_UNSPEC) },
		{ ENUM_KNOWN(0x5, IP6MRA_CREPORT_PKT) },
		{ ARG_XLAT_UNKNOWN(0x6, "IP6MRA_CREPORT_???") },
		{ ARG_XLAT_UNKNOWN(0x1ead, "IP6MRA_CREPORT_???") },
	};

	for (size_t i = 0; i < ARRAY_SIZE(unk6_attrs); i++) {
		TEST_NLATTR_(fd, nlh0, hdrlen, init_rtgen, print_rtgen,
			     unk6_attrs[i].val, unk6_attrs[i].str,
			     16, pattern, 16,
			     print_quoted_hex(pattern, 16));
	}

	/* RTNL_FAMILY_IP6MR: IP6MRA_CREPORT_MSGTYPE */
	static const struct strval8 mr6_msg_types[] = {
		{ ARG_XLAT_UNKNOWN(0, "MRT6MSG_???") },
		{ ARG_XLAT_KNOWN(0x1, "MRT6MSG_NOCACHE") },
		{ ARG_XLAT_KNOWN(0x2, "MRT6MSG_WRONGMIF") },
		{ ARG_XLAT_KNOWN(0x3, "MRT6MSG_WHOLEPKT") },
		{ ARG_XLAT_UNKNOWN(0x4, "MRT6MSG_???") },
		{ ARG_XLAT_UNKNOWN(0xca, "MRT6MSG_???") },
	};
	for (size_t i = 0; i < ARRAY_SIZE(mr6_msg_types); i++) {
		TEST_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
				       init_rtgen, print_rtgen,
				       IP6MRA_CREPORT_MSGTYPE,
				       XLAT_KNOWN(0x1,
						  "IP6MRA_CREPORT_MSGTYPE"),
				       pattern, mr6_msg_types[i].val, 1,
				       print_quoted_hex,
				       printf("%s", mr6_msg_types[i].str));
	}

	/* RTNL_FAMILY_IP6MR: u32 */
	static const struct strval16 u32_mr6_attrs[] = {
		{ ENUM_KNOWN(0x2, IP6MRA_CREPORT_MIF_ID) },
	};
	for (size_t i = 0; i < ARRAY_SIZE(u32_mr6_attrs); i++) {
		check_u32_nlattr(fd, nlh_u32, hdrlen, init_rtgen, print_rtgen,
				 u32_mr6_attrs[i].val, u32_mr6_attrs[i].str,
				 pattern, 0);
	}

	/* RTNL_FAMILY_IPMR: in6_addr */
	static const struct strval16 in6_addr_attrs[] = {
		{ ENUM_KNOWN(0x3, IP6MRA_CREPORT_SRC_ADDR) },
		{ ENUM_KNOWN(0x4, IP6MRA_CREPORT_DST_ADDR) },
	};
	uint8_t ipv6_addr[16] = {
		0xba, 0xdc, 0x0d, 0xed, 0xfa, 0xce, 0xbe, 0xef,
		0xde, 0xca, 0xfe, 0xed, 0xde, 0xad, 0xfe, 0xed,
	};
	for (size_t i = 0; i < ARRAY_SIZE(in6_addr_attrs); i++) {
		TEST_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
				       init_rtgen, print_rtgen,
				       in6_addr_attrs[i].val,
				       in6_addr_attrs[i].str,
				       pattern, ipv6_addr, 16,
				       print_quoted_hex,
				       printf(XLAT_KNOWN_FMT(
					      "\"\\xba\\xdc\\x0d\\xed"
					      "\\xfa\\xce\\xbe\\xef"
					      "\\xde\\xca\\xfe\\xed"
					      "\\xde\\xad\\xfe\\xed\"",
					      "inet_pton(AF_INET6"
					      ", \"badc:ded:face:beef"
					      ":deca:feed:dead:feed\")")));
	}

	puts("+++ exited with 0 +++");
	return 0;
}
