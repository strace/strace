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
#include <stdint.h>
#include "test_nlattr.h"

#define NLMSGERR_ATTR_COOKIE 3

static void
init_nlmsgerr(struct nlmsghdr *const nlh, const unsigned int msg_len)
{
	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = msg_len,
		.nlmsg_type = NLMSG_ERROR,
		.nlmsg_flags = NLM_F_REQUEST | NLM_F_CAPPED
	);

	struct nlmsgerr *const err = NLMSG_DATA(nlh);
	SET_STRUCT(struct nlmsgerr, err,
		.error = -13,
		.msg = {
			.nlmsg_len = NLMSG_HDRLEN + 4,
			.nlmsg_type = NLMSG_NOOP,
			.nlmsg_flags = NLM_F_REQUEST,
		}
	);
}

static void
print_nlmsgerr(const unsigned int msg_len)
{
	printf("{len=%u, type=NLMSG_ERROR"
	       ", flags=NLM_F_REQUEST|NLM_F_CAPPED"
	       ", seq=0, pid=0}, {error=-EACCES"
	       ", msg={len=%u, type=NLMSG_NOOP"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}",
	       msg_len, NLMSG_HDRLEN + 4);
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	const int fd = create_nl_socket(NETLINK_SOCK_DIAG);
	const unsigned int hdrlen = sizeof(struct nlmsgerr);
	void *const nlh0 = tail_alloc(NLMSG_SPACE(hdrlen));

	static const uint8_t cookie[] = { 0xab, 0xfe };
	TEST_NLATTR(fd, nlh0, hdrlen,
		    init_nlmsgerr, print_nlmsgerr,
		    NLMSGERR_ATTR_COOKIE,
		    sizeof(cookie), cookie, sizeof(cookie),
		    printf("[%u, %u]", cookie[0], cookie[1]);
		    printf("}"));

	printf("+++ exited with 0 +++\n");
	return 0;
}
