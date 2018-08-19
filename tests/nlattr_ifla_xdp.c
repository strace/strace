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
#include <linux/if.h>
#include <linux/if_arp.h>
#ifdef HAVE_LINUX_IF_LINK_H
# include <linux/if_link.h>
#endif
#include <linux/rtnetlink.h>

#if !HAVE_DECL_IFLA_XDP
enum { IFLA_XDP = 43 };
#endif
#ifndef IFLA_XDP_FD
# define IFLA_XDP_FD 1
#endif

#ifndef IFLA_XDP_PROG_ID
# define IFLA_XDP_PROG_ID 4
#endif

#ifndef IFLA_XDP_DRV_PROG_ID
# define IFLA_XDP_DRV_PROG_ID 5
#endif

#ifndef IFLA_XDP_SKB_PROG_ID
# define IFLA_XDP_SKB_PROG_ID 6
#endif

#ifndef IFLA_XDP_HW_PROG_ID
# define IFLA_XDP_HW_PROG_ID 7
#endif

#define IFLA_ATTR IFLA_XDP
#include "nlattr_ifla.h"

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	const int32_t num = 0xabacdbcd;
	const int fd = create_nl_socket(NETLINK_ROUTE);
	void *nlh0 = midtail_alloc(NLMSG_SPACE(hdrlen),
				   NLA_HDRLEN + sizeof(num));

	static char pattern[4096];
	fill_memory_ex(pattern, sizeof(pattern), 'a', 'z' - 'a' + 1);

	TEST_NESTED_NLATTR_OBJECT(fd, nlh0, hdrlen,
				  init_ifinfomsg, print_ifinfomsg,
				  IFLA_XDP_FD, pattern, num,
				  printf("%d", num));

#ifdef XDP_FLAGS_UPDATE_IF_NOEXIST
	const uint32_t flags = XDP_FLAGS_UPDATE_IF_NOEXIST;
	TEST_NESTED_NLATTR_OBJECT(fd, nlh0, hdrlen,
				  init_ifinfomsg, print_ifinfomsg,
				  IFLA_XDP_FLAGS, pattern, flags,
				  printf("XDP_FLAGS_UPDATE_IF_NOEXIST"));
#endif

	static const struct {
		uint32_t val;
		const char *str;
	} attrs[] = {
		{ ARG_STR(IFLA_XDP_PROG_ID) },
		{ ARG_STR(IFLA_XDP_DRV_PROG_ID) },
		{ ARG_STR(IFLA_XDP_SKB_PROG_ID) },
		{ ARG_STR(IFLA_XDP_HW_PROG_ID) },
	};

	for (size_t i = 0; i < ARRAY_SIZE(attrs); i++) {
		TEST_NESTED_NLATTR_OBJECT_EX_(fd, nlh0, hdrlen,
					      init_ifinfomsg, print_ifinfomsg,
					      attrs[i].val, attrs[i].str,
					      pattern, num,
					      print_quoted_hex, 1,
					      printf("%u", num));
	}

	puts("+++ exited with 0 +++");
	return 0;
}
