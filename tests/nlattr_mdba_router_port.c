/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017 The strace developers.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "tests.h"

#ifdef HAVE_STRUCT_BR_PORT_MSG

# include <stdio.h>
# include "test_nlattr.h"
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

	const int fd = create_nl_socket(NETLINK_ROUTE);

	void *nlh0 = tail_alloc(NLMSG_SPACE(hdrlen));

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

	const uint32_t ifindex = ifindex_lo();
	TEST_NESTED_NLATTR_OBJECT(fd, nlh0, hdrlen,
				  init_br_port_msg, print_br_port_msg,
				  MDBA_ROUTER_PORT, pattern, ifindex,
				  printf(IFINDEX_LO_STR));

	const uint8_t type = MDB_RTR_TYPE_DISABLED;
	static const struct nlattr nla = {
		.nla_len = NLA_HDRLEN + sizeof(type),
		.nla_type = MDBA_ROUTER_PATTR_TYPE
	};
	char buf[NLMSG_ALIGN(ifindex) + NLA_HDRLEN + sizeof(type)];
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
