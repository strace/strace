/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "test_nlattr.h"
#include <linux/neighbour.h>
#include <linux/rtnetlink.h>

static const unsigned int hdrlen = sizeof(struct ndmsg);

static void
init_ndmsg(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = msg_len,
		.nlmsg_type = RTM_GETNEIGH,
		.nlmsg_flags = NLM_F_DUMP
	);

	struct ndmsg *const msg = NLMSG_DATA(nlh);
	SET_STRUCT(struct ndmsg, msg,
		.ndm_family = AF_UNIX,
		.ndm_ifindex = ifindex_lo(),
		.ndm_state = NUD_PERMANENT,
		.ndm_flags = NTF_PROXY,
		.ndm_type = RTN_UNSPEC
	);
}

static void
print_ndmsg(const unsigned int msg_len)
{
	printf("{nlmsg_len=%u, nlmsg_type=RTM_GETNEIGH, nlmsg_flags=NLM_F_DUMP"
	       ", nlmsg_seq=0, nlmsg_pid=0}, {ndm_family=AF_UNIX"
	       ", ndm_ifindex=" IFINDEX_LO_STR
	       ", ndm_state=NUD_PERMANENT"
	       ", ndm_flags=NTF_PROXY"
	       ", ndm_type=RTN_UNSPEC}",
	       msg_len);
}

static void
init_ndmsg_nfea(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	init_ndmsg(nlh, msg_len);

	struct nlattr *nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = msg_len - NLMSG_SPACE(hdrlen),
		.nla_type = NDA_FDB_EXT_ATTRS,
	);
}

static void
print_ndmsg_nfea(const unsigned int msg_len)
{
	print_ndmsg(msg_len);
	printf(", [{nla_len=%u, nla_type=NDA_FDB_EXT_ATTRS}",
	       msg_len - NLMSG_SPACE(hdrlen));
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	const int fd = create_nl_socket(NETLINK_ROUTE);
	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen),
				   NLA_HDRLEN + sizeof(struct nda_cacheinfo));

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

	const unsigned int nla_type = 0xffff & NLA_TYPE_MASK;
	char nla_type_str[256];
	sprintf(nla_type_str, "%#x /* NDA_??? */", nla_type);
	TEST_NLATTR_(fd, nlh0, hdrlen,
		     init_ndmsg, print_ndmsg,
		     nla_type, nla_type_str,
		     4, pattern, 4,
		     print_quoted_hex(pattern, 4));

	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_ndmsg, print_ndmsg,
		    NDA_DST, 4, pattern, 4,
		    print_quoted_hex(pattern, 4));

	static const struct nda_cacheinfo ci = {
		.ndm_confirmed = 0xabcdedad,
		.ndm_used = 0xbcdaedad,
		.ndm_updated = 0xcdbadeda,
		.ndm_refcnt = 0xdeadbeda
	};

	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_ndmsg, print_ndmsg,
			   NDA_CACHEINFO, pattern, ci,
			   printf("{");
			   PRINT_FIELD_U(ci, ndm_confirmed);
			   printf(", ");
			   PRINT_FIELD_U(ci, ndm_used);
			   printf(", ");
			   PRINT_FIELD_U(ci, ndm_updated);
			   printf(", ");
			   PRINT_FIELD_U(ci, ndm_refcnt);
			   printf("}"));

	const uint16_t port = 0xabcd;
	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_ndmsg, print_ndmsg,
			   NDA_PORT, pattern, port,
			   printf("htons(%u)", ntohs(port)));

	static const uint8_t mac[6] = "\xf8\xc2\x49\x13\x57\xbd";
	TEST_NLATTR(fd, nlh0, hdrlen, init_ndmsg, print_ndmsg,
		    NDA_LLADDR, sizeof(mac) - 1, mac, sizeof(mac) - 1,
		    for (unsigned int i = 0; i < sizeof(mac) - 1; ++i)
			printf("%s%02x", i ? ":" : "", mac[i]));

	TEST_NLATTR(fd, nlh0, hdrlen, init_ndmsg, print_ndmsg,
		    NDA_LLADDR, sizeof(mac), mac, sizeof(mac),
		    for (unsigned int i = 0; i < sizeof(mac); ++i)
			printf("%s%02x", i ? ":" : "", mac[i]));

	/* u32 attrs */
	static const struct strval16 u32_attrs[] = {
		{ ENUM_KNOWN(0x4, NDA_PROBES) },
		{ ENUM_KNOWN(0x7, NDA_VNI) },
		{ ENUM_KNOWN(0xa, NDA_LINK_NETNSID) },
		{ ENUM_KNOWN(0xb, NDA_SRC_VNI) },
		{ ENUM_KNOWN(0xd, NDA_NH_ID) },
	};
	void *nlh_u32 = midtail_alloc(NLMSG_SPACE(hdrlen), sizeof(uint32_t));

	for (size_t i = 0; i < ARRAY_SIZE(u32_attrs); i++) {
		check_u32_nlattr(fd, nlh_u32, hdrlen, init_ndmsg, print_ndmsg,
				u32_attrs[i].val, u32_attrs[i].str, pattern, 0);
	}

	/* NDA_FDB_EXT_ATTRS: unknown, undecoded */
	static const struct strval16 nfea_unk_attrs[] = {
		{ ENUM_KNOWN(0, NFEA_UNSPEC) },
		{ ENUM_KNOWN(0x2, NFEA_DONT_REFRESH) },
		{ ARG_XLAT_UNKNOWN(0x3, "NFEA_???") },
		{ ARG_XLAT_UNKNOWN(0x1ace, "NFEA_???") },
	};
	static const uint32_t dummy = BE_LE(0xbadc0ded, 0xed0ddcba);

	for (size_t i = 0; i < ARRAY_SIZE(nfea_unk_attrs); i++) {
		TEST_NESTED_NLATTR_(fd, nlh0, hdrlen,
				    init_ndmsg_nfea, print_ndmsg_nfea,
				    nfea_unk_attrs[i].val,
				    nfea_unk_attrs[i].str,
				    sizeof(dummy), &dummy, sizeof(dummy), 1,
				    printf("\"\\xba\\xdc\\x0d\\xed\""));
	}

	/* NDA_FDB_EXT_ATTRS: NFEA_ACTIVITY_NOTIFY */
	static const struct strval8 fan_flags[] = {
		{ ARG_STR(0) },
		{ ARG_XLAT_KNOWN(0x1, "FDB_NOTIFY_BIT") },
		{ ARG_XLAT_KNOWN(0xef, "FDB_NOTIFY_BIT"
				       "|FDB_NOTIFY_INACTIVE_BIT|0xec") },
		{ ARG_XLAT_UNKNOWN(0xfc, "FDB_NOTIFY_???") },
	};

	for (size_t i = 0; i < ARRAY_SIZE(fan_flags); i++) {
		TEST_NESTED_NLATTR_(fd, nlh0, hdrlen,
				    init_ndmsg_nfea, print_ndmsg_nfea,
				    NFEA_ACTIVITY_NOTIFY,
				    "NFEA_ACTIVITY_NOTIFY",
				    1, &fan_flags[i].val, 1, 1,
				    printf("%s", fan_flags[i].str));
	}

	/* NDA_FLAGS_EXT */
	static const struct strval32 ntfe_flags[] = {
		{ ARG_STR(0) },
		{ ARG_XLAT_KNOWN(0x1, "NTF_EXT_MANAGED") },
		{ ARG_XLAT_KNOWN(0x2, "NTF_EXT_LOCKED") },
		{ ARG_XLAT_KNOWN(0xdeadbeef, "NTF_EXT_MANAGED|NTF_EXT_LOCKED"
					     "|0xdeadbeec") },
		{ ARG_XLAT_UNKNOWN(0xfeedcafc, "NTF_EXT_???") },
	};

	for (size_t i = 0; i < ARRAY_SIZE(ntfe_flags); i++) {
		TEST_NLATTR(fd, nlh0, hdrlen, init_ndmsg, print_ndmsg,
			    NDA_FLAGS_EXT, 4, &ntfe_flags[i].val, 4,
			    printf("%s", ntfe_flags[i].str));
	}

	/* NDA_NDM_STATE_MASK */
	static const struct strval16 states_flags[] = {
		{ ARG_XLAT_KNOWN(0, "NUD_NONE") },
		{ ARG_XLAT_KNOWN(0x1, "NUD_INCOMPLETE") },
		{ ARG_XLAT_KNOWN(0xabed, "NUD_INCOMPLETE|NUD_STALE|NUD_DELAY"
					 "|NUD_FAILED|NUD_NOARP|NUD_PERMANENT"
					 "|0xab00") },
		{ ARG_XLAT_UNKNOWN(0xff00, "NUD_???") },
	};

	for (size_t i = 0; i < ARRAY_SIZE(states_flags); i++) {
		TEST_NLATTR(fd, nlh0, hdrlen, init_ndmsg, print_ndmsg,
			    NDA_NDM_STATE_MASK, 2, &states_flags[i].val, 2,
			    printf("%s", states_flags[i].str));
	}

	/* NDA_NDM_FLAGS_MASK */
	static const struct strval8 ndm_flags[] = {
		{ ARG_STR(0) },
		{ ARG_XLAT_KNOWN(0x1, "NTF_USE") },
		{ ARG_XLAT_KNOWN(0xbe, "NTF_SELF|NTF_MASTER|NTF_PROXY"
					"|NTF_EXT_LEARNED|NTF_OFFLOADED"
					"|NTF_ROUTER") },
	};

	for (size_t i = 0; i < ARRAY_SIZE(ndm_flags); i++) {
		TEST_NLATTR(fd, nlh0, hdrlen, init_ndmsg, print_ndmsg,
			    NDA_NDM_FLAGS_MASK, 1, &ndm_flags[i].val, 1,
			    printf("%s", ndm_flags[i].str));
	}

	puts("+++ exited with 0 +++");
	return 0;
}
