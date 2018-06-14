/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2018 The strace developers.
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
#include "test_nlattr.h"
#include <linux/pkt_sched.h>
#include <linux/rtnetlink.h>

#ifndef TCA_STAB
# define TCA_STAB 8
#endif
#if !HAVE_DECL_TCA_STAB_DATA
enum { TCA_STAB_DATA = 2 };
#endif

const unsigned int hdrlen = sizeof(struct tcmsg);

static void
init_tcmsg(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = msg_len,
		.nlmsg_type = RTM_GETQDISC,
		.nlmsg_flags = NLM_F_DUMP
	);

	struct tcmsg *const msg = NLMSG_DATA(nlh);
	SET_STRUCT(struct tcmsg, msg,
		.tcm_family = AF_UNIX,
		.tcm_ifindex = ifindex_lo()
	);

	struct nlattr *const nla = NLMSG_ATTR(nlh, sizeof(*msg));
	SET_STRUCT(struct nlattr, nla,
		.nla_len = msg_len - NLMSG_SPACE(hdrlen),
		.nla_type = TCA_STAB
	);
}

static void
print_tcmsg(const unsigned int msg_len)
{
	printf("{len=%u, type=RTM_GETQDISC, flags=NLM_F_DUMP"
	       ", seq=0, pid=0}, {tcm_family=AF_UNIX"
	       ", tcm_ifindex=" IFINDEX_LO_STR
	       ", tcm_handle=0, tcm_parent=0, tcm_info=0}"
	       ", {{nla_len=%u, nla_type=TCA_STAB}",
	       msg_len, msg_len - NLMSG_SPACE(hdrlen));
}

static void
print_uint16(const uint16_t *p, size_t idx)
{
	printf("%u", *p);
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	const int fd = create_nl_socket(NETLINK_ROUTE);
	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen), NLA_HDRLEN + 4
#ifdef HAVE_STRUCT_TC_SIZESPEC
			- 4 + sizeof(struct tc_sizespec)
#endif
			);

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

#ifdef HAVE_STRUCT_TC_SIZESPEC
	static const struct tc_sizespec s = {
		.cell_log = 0xab,
		.size_log = 0xcd,
		.cell_align = 0xefab,
		.overhead = 0xcdadeefa,
		.linklayer = 0xefbaafeb,
		.mpu = 0xfebfaefb,
		.mtu = 0xacdbefab,
		.tsize = 0xbdeaabed
	};
	TEST_NESTED_NLATTR_OBJECT(fd, nlh0, hdrlen,
				  init_tcmsg, print_tcmsg,
				  TCA_STAB_BASE, pattern, s,
				  PRINT_FIELD_U("{", s, cell_log);
				  PRINT_FIELD_U(", ", s, size_log);
				  PRINT_FIELD_D(", ", s, cell_align);
				  PRINT_FIELD_D(", ", s, overhead);
				  PRINT_FIELD_U(", ", s, linklayer);
				  PRINT_FIELD_U(", ", s, mpu);
				  PRINT_FIELD_U(", ", s, mtu);
				  PRINT_FIELD_U(", ", s, tsize);
				  printf("}"));
#endif

	uint16_t data[2] = { 0xacbd, 0xefba };
	TEST_NESTED_NLATTR_ARRAY(fd, nlh0, hdrlen,
				 init_tcmsg, print_tcmsg,
				 TCA_STAB_DATA, pattern, data, print_uint16);

	puts("+++ exited with 0 +++");
	return 0;
}
