/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#ifdef HAVE_STRUCT_BR_PORT_MSG

# include <stdio.h>
# include "test_nlattr.h"
/* struct br_mdb_entry needs a definition of struct in6_addr.  */
# include <netinet/in.h>
# include <linux/if_bridge.h>
# include <linux/rtnetlink.h>

# ifndef MDBA_ROUTER
#  define MDBA_ROUTER 2
# endif
# ifndef MDBA_ROUTER_PORT
#  define MDBA_ROUTER_PORT 1
# endif
# ifndef MDBA_ROUTER_PATTR_TYPE
#  define MDBA_ROUTER_PATTR_TYPE 2
# endif
# ifndef MDB_RTR_TYPE_DISABLED
#  define MDB_RTR_TYPE_DISABLED 0
# endif

const unsigned int hdrlen = sizeof(struct br_port_msg);

static void
init_br_port_msg(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = msg_len,
		.nlmsg_type = RTM_GETMDB,
		.nlmsg_flags = NLM_F_DUMP
	);

	struct br_port_msg *const msg = NLMSG_DATA(nlh);
	SET_STRUCT(struct br_port_msg, msg,
		.family = AF_UNIX,
		.ifindex = ifindex_lo()
	);

	struct nlattr *nla = NLMSG_ATTR(nlh, sizeof(*msg));
	SET_STRUCT(struct nlattr, nla,
		.nla_len = msg_len - NLMSG_SPACE(hdrlen),
		.nla_type = MDBA_ROUTER
	);
}

static void
print_br_port_msg(const unsigned int msg_len)
{
	printf("{len=%u, type=RTM_GETMDB, flags=NLM_F_DUMP"
	       ", seq=0, pid=0}, {family=AF_UNIX"
	       ", ifindex=" IFINDEX_LO_STR "}"
	       ", {{nla_len=%u, nla_type=MDBA_ROUTER}",
	       msg_len, msg_len - NLMSG_SPACE(hdrlen));
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	const uint32_t ifindex = ifindex_lo();
	const uint8_t type = MDB_RTR_TYPE_DISABLED;
	static const struct nlattr nla = {
		.nla_len = NLA_HDRLEN + sizeof(type),
		.nla_type = MDBA_ROUTER_PATTR_TYPE
	};
	char buf[NLMSG_ALIGN(ifindex) + NLA_HDRLEN + sizeof(type)];

	const int fd = create_nl_socket(NETLINK_ROUTE);

	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen),
				   NLA_HDRLEN + sizeof(buf));

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

	TEST_NESTED_NLATTR_OBJECT(fd, nlh0, hdrlen,
				  init_br_port_msg, print_br_port_msg,
				  MDBA_ROUTER_PORT, pattern, ifindex,
				  printf(IFINDEX_LO_STR));

	memcpy(buf, &ifindex, sizeof(ifindex));
	memcpy(buf + NLMSG_ALIGN(ifindex), &nla, sizeof(nla));
	memcpy(buf + NLMSG_ALIGN(ifindex) + NLA_HDRLEN, &type, sizeof(type));
	TEST_NLATTR(fd, nlh0 - NLA_HDRLEN, hdrlen + NLA_HDRLEN,
		    init_br_port_msg, print_br_port_msg,
		    MDBA_ROUTER_PORT, sizeof(buf), buf, sizeof(buf),
		    printf(IFINDEX_LO_STR
			   ", {{nla_len=%u, nla_type=MDBA_ROUTER_PATTR_TYPE}"
			   ", MDB_RTR_TYPE_DISABLED}}",
			   nla.nla_len));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_STRUCT_BR_PORT_MSG")

#endif
