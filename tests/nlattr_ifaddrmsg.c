/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include <arpa/inet.h>
#include "test_nlattr.h"
#include <linux/if_addr.h>
#include <linux/rtnetlink.h>

#include "xlat.h"
#include "xlat/addrfams.h"

static uint32_t idx;
static uint8_t ifa_family;
static char ifa_family_str[256];

/* The array size of ifa_flags and ifa_scopes are chosen to be coprime */
static const struct strval8 ifa_flags[] = {
	{ ARG_STR(0) },
	{ ARG_XLAT_KNOWN(0x1, "IFA_F_SECONDARY") },
	/* struct ifaddrmsg::ifa_flags is only 8 bits wide */
	{ ARG_XLAT_KNOWN(0xff,
			 "IFA_F_SECONDARY|IFA_F_NODAD|IFA_F_OPTIMISTIC"
			 "|IFA_F_DADFAILED|IFA_F_HOMEADDRESS"
			 "|IFA_F_DEPRECATED|IFA_F_TENTATIVE"
			 "|IFA_F_PERMANENT") },
};
static const struct strval32 ifa_scopes[] = {
	{ ARG_XLAT_KNOWN(0, "RT_SCOPE_UNIVERSE") },
	{ ARG_STR(0x1) },
	{ ARG_STR(0x2) },
	{ ARG_STR(0x7f) },
	{ ARG_STR(0xc7) },
	{ ARG_XLAT_KNOWN(0xc8, "RT_SCOPE_SITE") },
	{ ARG_STR(0xc9) },
	{ ARG_STR(0xfc) },
	{ ARG_XLAT_KNOWN(0xfd, "RT_SCOPE_LINK") },
	{ ARG_XLAT_KNOWN(0xfe, "RT_SCOPE_HOST") },
	{ ARG_XLAT_KNOWN(0xff, "RT_SCOPE_NOWHERE") },
};

static void
init_ifaddrmsg(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	idx++;

	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = msg_len,
		.nlmsg_type = RTM_GETADDR,
		.nlmsg_flags = NLM_F_DUMP
	);

	struct ifaddrmsg *const msg = NLMSG_DATA(nlh);
	SET_STRUCT(struct ifaddrmsg, msg,
		.ifa_family = ifa_family,
		.ifa_flags = ARR_ITEM(ifa_flags, idx).val,
		.ifa_scope = ARR_ITEM(ifa_scopes, idx).val,
		.ifa_index = ifindex_lo()
	);
}

static void
print_ifaddrmsg(const unsigned int msg_len)
{
	printf("{nlmsg_len=%u, nlmsg_type=" XLAT_FMT ", nlmsg_flags=" XLAT_FMT
	       ", nlmsg_seq=0, nlmsg_pid=0}, {ifa_family=%s"
	       ", ifa_prefixlen=0, ifa_flags=%s, ifa_scope=%s"
	       ", ifa_index=" XLAT_FMT_U "}",
	       msg_len, XLAT_ARGS(RTM_GETADDR), XLAT_ARGS(NLM_F_DUMP),
	       ifa_family_str,
	       ARR_ITEM(ifa_flags, idx).str, ARR_ITEM(ifa_scopes, idx).str,
	       XLAT_SEL(ifindex_lo(), IFINDEX_LO_STR));
}

static inline void
pr_addr(const void *addr, size_t addr_sz,
	const char *pfx, const char *str, const char *sfx)
{
#if XLAT_RAW || XLAT_VERBOSE
	print_quoted_hex(addr, addr_sz);
	if (addr_sz > DEFAULT_STRLEN)
		printf("...");
#endif
#if XLAT_VERBOSE
	printf(" /* ");
#endif
#if !XLAT_RAW
	printf("%s%s%s", pfx, str, sfx);
#endif
#if XLAT_VERBOSE
	printf(" */");
#endif
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	static const struct strval32 ifa_flags_ext[] = {
		{ 0, "0" },
		{ ARG_XLAT_KNOWN(0x1, "IFA_F_SECONDARY") },
		{ ARG_XLAT_UNKNOWN(0x1000, "IFA_F_???") },
		{ ARG_XLAT_KNOWN(0xffffffff,
				 "IFA_F_SECONDARY|IFA_F_NODAD|IFA_F_OPTIMISTIC"
				 "|IFA_F_DADFAILED|IFA_F_HOMEADDRESS"
				 "|IFA_F_DEPRECATED|IFA_F_TENTATIVE"
				 "|IFA_F_PERMANENT|IFA_F_MANAGETEMPADDR"
				 "|IFA_F_NOPREFIXROUTE|IFA_F_MCAUTOJOIN"
				 "|IFA_F_STABLE_PRIVACY|0xfffff000") },
	};
	static const char address4[] = "12.34.56.78";
	static const char address6[] = "12:34:56:78:90:ab:cd:ef";
	static const struct ifa_cacheinfo cis[] = {
		{
			.ifa_prefered = 0,
			.ifa_valid = 0,
			.cstamp = 0,
			.tstamp = 0,
		},
		{
			.ifa_prefered = 1,
			.ifa_valid = 2,
			.cstamp = 300,
			.tstamp = 400,
		},
		{
			.ifa_prefered = 0xabcdefac,
			.ifa_valid = 0xbcdadbca,
			.cstamp = 0xcdabedba,
			.tstamp = 0xdebabdac,
		},
		{
			.ifa_prefered = 0xffffffff,
			.ifa_valid = 0xffffffff,
			.cstamp = 0xffffffff,
			.tstamp = 0xffffffff,
		},
	};

	struct in_addr a4;
	struct in6_addr a6;

	const int fd = create_nl_socket(NETLINK_ROUTE);
	const unsigned int hdrlen = sizeof(struct ifaddrmsg);
	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen),
				   NLA_HDRLEN + MAX(sizeof(cis[0]),
						    sizeof(a6)));

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

	const struct {
		uint8_t af;
		const void *addr;
		size_t addr_sz;
		const char *addr_str;
		const char *addr_pfx;
		const char *addr_sfx;
	} address_data[] = {
		/* Should be sorted on af field */
		{ AF_INET,
		  &a4, sizeof(a4), address4, "inet_addr(\"", "\")" },
		{ AF_INET6,
		  &a6, sizeof(a6), address6, "inet_pton(AF_INET6, \"", "\")" },
	};
	size_t pos = 0;

	if (!inet_pton(AF_INET, address4, &a4))
		perror_msg_and_skip("inet_pton(AF_INET, address4)");

	if (!inet_pton(AF_INET6, address6, &a6))
		perror_msg_and_skip("inet_pton(AF_INET6, address6)");

	for (uint8_t af = 0; af < 50; af++) {
		ifa_family = af;
		strncpy(ifa_family_str, sprintxval(addrfams, af, "AF_???"),
			sizeof(ifa_family_str) - 1);
		ifa_family_str[sizeof(ifa_family_str) - 1] = '\0';

		/* Unimplemented message types */
		static const struct strval32 unimp_types[] = {
			{ ARG_XLAT_KNOWN(0, "IFA_UNSPEC") },
		};
		for (size_t i = 0; i < ARRAY_SIZE(unimp_types); i++) {
			for (size_t j = 1; j < DEFAULT_STRLEN + 2; j++) {
				TEST_NLATTR_(fd, nlh0, hdrlen,
					     init_ifaddrmsg, print_ifaddrmsg,
					     unimp_types[i].val,
					     unimp_types[i].str,
					     j, pattern, j,
					     print_quoted_hex(pattern,
							MIN(j, DEFAULT_STRLEN));
					     if (j > DEFAULT_STRLEN)
						     printf("..."));
			}
		}

		/* Unknown message type */
		TEST_NLATTR_(fd, nlh0, hdrlen,
			     init_ifaddrmsg, print_ifaddrmsg,
			     12, "0xc" NRAW(" /* IFA_??? */"),
			     4, pattern, 4,
			     print_quoted_hex(pattern, 4));

		const unsigned int nla_type = 0xffff & NLA_TYPE_MASK;
		char nla_type_str[256];
		sprintf(nla_type_str, "%#x" NRAW(" /* IFA_??? */"), nla_type);
		TEST_NLATTR_(fd, nlh0, hdrlen,
			     init_ifaddrmsg, print_ifaddrmsg,
			     nla_type, nla_type_str,
			     4, pattern, 4,
			     print_quoted_hex(pattern, 4));

		/* Addresses */
		static const struct strval32 addr_attrs[] = {
			{ ARG_XLAT_KNOWN(0x1, "IFA_ADDRESS") },
			{ ARG_XLAT_KNOWN(0x2, "IFA_LOCAL") },
			{ ARG_XLAT_KNOWN(0x4, "IFA_BROADCAST") },
			{ ARG_XLAT_KNOWN(0x5, "IFA_ANYCAST") },
			{ ARG_XLAT_KNOWN(0x7, "IFA_MULTICAST") },
		};
		for (size_t i = 0; i < ARRAY_SIZE(addr_attrs); i++) {
			if (af == address_data[pos].af) {
				TEST_NLATTR_OBJPTR_EX_(fd, nlh0, hdrlen,
						       init_ifaddrmsg,
						       print_ifaddrmsg,
						       addr_attrs[i].val,
						       addr_attrs[i].str,
						       pattern,
						       address_data[pos].addr,
						       address_data[pos].addr_sz,
						       address_data[pos].addr_sz,
						       print_quoted_hex,
						       pr_addr(address_data[pos]
							       .addr,
							       address_data[pos]
							       .addr_sz,
							       address_data[pos]
							       .addr_pfx,
							       address_data[pos]
							       .addr_str,
							       address_data[pos]
							       .addr_sfx));
			} else {
				TEST_NLATTR_(fd, nlh0, hdrlen,
					     init_ifaddrmsg, print_ifaddrmsg,
					     addr_attrs[i].val,
					     addr_attrs[i].str, 4, pattern, 4,
					     print_quoted_hex(pattern, 4));
			}
		}

		/* string */
		static const struct strval32 str_types[] = {
			{ ARG_XLAT_KNOWN(0x3, "IFA_LABEL") },
		};
		for (size_t i = 0; i < ARRAY_SIZE(str_types); i++) {
			TEST_NLATTR_(fd, nlh0, hdrlen,
				     init_ifaddrmsg, print_ifaddrmsg,
				     str_types[i].val, str_types[i].str,
				     8, pattern, 8,
				     print_quoted_stringn(pattern, 8));
			TEST_NLATTR_(fd, nlh0, hdrlen,
				     init_ifaddrmsg, print_ifaddrmsg,
				     str_types[i].val, str_types[i].str,
				     40, pattern, 40,
				     print_quoted_stringn(pattern, 32));
		}

		/* struct ifa_cacheinfo */
		for (size_t i = 0; i < ARRAY_SIZE(cis); i++) {
			TEST_NLATTR_OBJECT_(fd, nlh0, hdrlen,
					    init_ifaddrmsg, print_ifaddrmsg,
					    IFA_CACHEINFO,
					    XLAT_KNOWN(0x6, "IFA_CACHEINFO"),
					    pattern, cis[i],
					    printf("{");
					    PRINT_FIELD_U(cis[i], ifa_prefered);
					    printf(", ");
					    PRINT_FIELD_U(cis[i], ifa_valid);
					    printf(", ");
					    PRINT_FIELD_U(cis[i], cstamp);
					    printf(", ");
					    PRINT_FIELD_U(cis[i], tstamp);
					    printf("}"));
		}

		/* IFA_F_* */
		for (size_t i = 0; i < ARRAY_SIZE(ifa_flags_ext); i++) {
			TEST_NLATTR_OBJECT_(fd, nlh0, hdrlen,
					    init_ifaddrmsg, print_ifaddrmsg,
					    IFA_FLAGS,
					    XLAT_KNOWN(0x8, "IFA_FLAGS"),
					    pattern,
					    ifa_flags_ext[i].val,
					    printf("%s", ifa_flags_ext[i].str));
		}

		/* IFAPROT_* */
		static const struct strval8 protos[] = {
			{ ARG_XLAT_KNOWN(0, "IFAPROT_UNSPEC") },
			{ ARG_XLAT_KNOWN(0x1, "IFAPROT_KERNEL_LO") },
			{ ARG_XLAT_KNOWN(0x3, "IFAPROT_KERNEL_LL") },
			{ ARG_XLAT_UNKNOWN(0x4, "IFAPROT_???") },
			{ ARG_XLAT_UNKNOWN(0x5, "IFAPROT_???") },
			{ ARG_XLAT_UNKNOWN(0xff, "IFAPROT_???") },
		};
		for (size_t i = 0; i < ARRAY_SIZE(protos); i++) {
			TEST_NLATTR_OBJECT_(fd, nlh0, hdrlen,
					    init_ifaddrmsg, print_ifaddrmsg,
					    IFA_PROTO,
					    XLAT_KNOWN(0xb, "IFA_PROTO"),
					    pattern, protos[i].val,
					    printf("%s", protos[i].str));
		}

		/* u32 */
		check_u32_nlattr(fd, nlh0, hdrlen,
				 init_ifaddrmsg, print_ifaddrmsg,
				 ARG_XLAT_KNOWN(0x9, "IFA_RT_PRIORITY"),
				 pattern, 0);

		/* s32 */
		check_s32_nlattr(fd, nlh0, hdrlen,
				 init_ifaddrmsg, print_ifaddrmsg,
				 ARG_XLAT_KNOWN(0xa, "IFA_TARGET_NETNSID"),
				 pattern, 0);

		/* Progressing through the address_data array */
		if (af == address_data[pos].af)
			pos++;
	}

	puts("+++ exited with 0 +++");
	return 0;
}
