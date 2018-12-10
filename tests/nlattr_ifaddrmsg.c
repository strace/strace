/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include <arpa/inet.h>
#include "test_nlattr.h"
#ifdef HAVE_LINUX_IF_ADDR_H
# include <linux/if_addr.h>
#endif
#include <linux/rtnetlink.h>

#define IFA_FLAGS 8

#define SET_IFA_FAMILY(af)		\
	do {				\
		ifa_family = af;	\
		ifa_family_str = #af;	\
	}				\
	while (0)

uint8_t ifa_family;
const char *ifa_family_str;

static void
init_ifaddrmsg(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = msg_len,
		.nlmsg_type = RTM_GETADDR,
		.nlmsg_flags = NLM_F_DUMP
	);

	struct ifaddrmsg *const msg = NLMSG_DATA(nlh);
	SET_STRUCT(struct ifaddrmsg, msg,
		.ifa_family = ifa_family,
		.ifa_flags = IFA_F_SECONDARY,
		.ifa_scope = RT_SCOPE_UNIVERSE,
		.ifa_index = ifindex_lo()
	);
}

static void
print_ifaddrmsg(const unsigned int msg_len)
{
	printf("{len=%u, type=RTM_GETADDR, flags=NLM_F_DUMP"
	       ", seq=0, pid=0}, {ifa_family=%s"
	       ", ifa_prefixlen=0"
	       ", ifa_flags=IFA_F_SECONDARY"
	       ", ifa_scope=RT_SCOPE_UNIVERSE"
	       ", ifa_index=" IFINDEX_LO_STR "}",
	       msg_len, ifa_family_str);
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	static const char address4[] = "12.34.56.78";
	static const char address6[] = "12:34:56:78:90:ab:cd:ef";
	static const struct ifa_cacheinfo ci = {
		.ifa_prefered = 0xabcdefac,
		.ifa_valid = 0xbcdadbca,
		.cstamp = 0xcdabedba,
		.tstamp = 0xdebabdac
	};

	struct in_addr a4;
	struct in6_addr a6;
	const uint32_t ifa_flags = IFA_F_SECONDARY | IFA_F_PERMANENT;

	const int fd = create_nl_socket(NETLINK_ROUTE);
	const unsigned int hdrlen = sizeof(struct ifaddrmsg);
	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen),
				   NLA_HDRLEN + MAX(sizeof(ci), sizeof(a6)));

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

	SET_IFA_FAMILY(AF_UNSPEC);
	const unsigned int nla_type = 0xffff & NLA_TYPE_MASK;
	char nla_type_str[256];
	sprintf(nla_type_str, "%#x /* IFA_??? */", nla_type);
	TEST_NLATTR_(fd, nlh0, hdrlen,
		     init_ifaddrmsg, print_ifaddrmsg,
		     nla_type, nla_type_str,
		     4, pattern, 4,
		     print_quoted_hex(pattern, 4));

	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_ifaddrmsg, print_ifaddrmsg,
		    IFA_ADDRESS, 4, pattern, 4,
		    print_quoted_hex(pattern, 4));

	SET_IFA_FAMILY(AF_INET);

	if (!inet_pton(AF_INET, address4, &a4))
		perror_msg_and_skip("inet_pton");

	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_ifaddrmsg, print_ifaddrmsg,
			   IFA_ADDRESS, pattern, a4,
			   printf("inet_addr(\"%s\")", address4));

	SET_IFA_FAMILY(AF_INET6);

	if (!inet_pton(AF_INET6, address6, &a6))
		perror_msg_and_skip("inet_pton");

	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_ifaddrmsg, print_ifaddrmsg,
			   IFA_ADDRESS, pattern, a6,
			   printf("inet_pton(AF_INET6, \"%s\")", address6));

	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_ifaddrmsg, print_ifaddrmsg,
			   IFA_CACHEINFO, pattern, ci,
			   PRINT_FIELD_U("{", ci, ifa_prefered);
			   PRINT_FIELD_U(", ", ci, ifa_valid);
			   PRINT_FIELD_U(", ", ci, cstamp);
			   PRINT_FIELD_U(", ", ci, tstamp);
			   printf("}"));

	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_ifaddrmsg, print_ifaddrmsg,
			   IFA_FLAGS, pattern, ifa_flags,
			   printf("IFA_F_SECONDARY|IFA_F_PERMANENT"));

	puts("+++ exited with 0 +++");
	return 0;
}
