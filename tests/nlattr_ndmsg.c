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

#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "test_nlattr.h"
#ifdef HAVE_LINUX_NEIGHBOUR_H
# include <linux/neighbour.h>
#endif
#include <linux/rtnetlink.h>

#define NDA_PORT 6

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
	printf("{len=%u, type=RTM_GETNEIGH, flags=NLM_F_DUMP"
	       ", seq=0, pid=0}, {ndm_family=AF_UNIX"
	       ", ndm_ifindex=" IFINDEX_LO_STR
	       ", ndm_state=NUD_PERMANENT"
	       ", ndm_flags=NTF_PROXY"
	       ", ndm_type=RTN_UNSPEC}",
	       msg_len);
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	const int fd = create_nl_socket(NETLINK_ROUTE);
	const unsigned int hdrlen = sizeof(struct ndmsg);
	void *nlh0 = tail_alloc(NLMSG_SPACE(hdrlen));

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
			   PRINT_FIELD_U("{", ci, ndm_confirmed);
			   PRINT_FIELD_U(", ", ci, ndm_used);
			   PRINT_FIELD_U(", ", ci, ndm_updated);
			   PRINT_FIELD_U(", ", ci, ndm_refcnt);
			   printf("}"));

	const uint16_t port = 0xabcd;
	TEST_NLATTR_OBJECT(fd, nlh0, hdrlen,
			   init_ndmsg, print_ndmsg,
			   NDA_PORT, pattern, port,
			   printf("htons(%u)", ntohs(port)));

	puts("+++ exited with 0 +++");
	return 0;
}
