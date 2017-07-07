/*
 * Copyright (c) 2017 The strace developers.
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

/* This test case is based on netlink_selinux.c */

#include "tests.h"

#ifdef HAVE_LINUX_GENETLINK_H

# include <stdio.h>
# include <string.h>
# include <unistd.h>
# include <sys/socket.h>
# include "netlink.h"
# include <linux/genetlink.h>

static void
test_nlmsg_type(const int fd)
{
	/*
	 * Though GENL_ID_CTRL number is statically fixed in this test case,
	 * strace does not have a builtin knowledge that the corresponding
	 * string is "nlctrl".
	 */
	long rc;
	struct {
		const struct nlmsghdr nlh;
		struct genlmsghdr gnlh;
	} req = {
		.nlh = {
			.nlmsg_len = sizeof(req),
			.nlmsg_type = GENL_ID_CTRL,
			.nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST
		},
		.gnlh = {
			.cmd = CTRL_CMD_GETFAMILY
		}
	};

	rc = sendto(fd, &req, sizeof(req), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {{len=%u, type=nlctrl"
	       ", flags=NLM_F_REQUEST|0x300, seq=0, pid=0}"
	       ", \"\\x03\\x00\\x00\\x00\"}, %u"
	       ", MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, req.nlh.nlmsg_len,
	       (unsigned int) sizeof(req), sprintrc(rc));
}

int main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	int fd = create_nl_socket(NETLINK_GENERIC);

	test_nlmsg_type(fd);

	printf("+++ exited with 0 +++\n");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_LINUX_GENETLINK_H")

#endif
