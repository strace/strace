/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
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
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "test_netlink.h"
#include <linux/selinux_netlink.h>

static void
test_nlmsg_type(const int fd)
{
	long rc;
	struct nlmsghdr nlh = {
		.nlmsg_len = sizeof(nlh),
		.nlmsg_type = SELNL_MSG_SETENFORCE,
		.nlmsg_flags = NLM_F_REQUEST,
	};

	rc = sendto(fd, &nlh, sizeof(nlh), MSG_DONTWAIT, NULL, 0);
	printf("sendto(%d, {len=%u, type=SELNL_MSG_SETENFORCE"
	       ", flags=NLM_F_REQUEST, seq=0, pid=0}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh.nlmsg_len, (unsigned) sizeof(nlh), sprintrc(rc));
}

static void
test_selnl_msg_unspec(const int fd)
{
	void *const nlh0 = tail_alloc(NLMSG_HDRLEN);

	TEST_NETLINK_(fd, nlh0,
		      0xffff, "0xffff /* SELNL_MSG_??? */",
		      NLM_F_REQUEST, "NLM_F_REQUEST",
		      4, "1234", 4,
		      printf("\"\\x31\\x32\\x33\\x34\""));
}

static void
test_selnl_msg_setenforce(const int fd)
{
	void *const nlh0 = tail_alloc(NLMSG_HDRLEN);

	static const struct selnl_msg_setenforce msg = {
		.val = 0xfbdcdfab
	};
	TEST_NETLINK_OBJECT(fd, nlh0,
			    SELNL_MSG_SETENFORCE, NLM_F_REQUEST, msg,
			    PRINT_FIELD_D("{", msg, val);
			    printf("}"));
}

static void
test_selnl_msg_policyload(const int fd)
{
	void *const nlh0 = tail_alloc(NLMSG_HDRLEN);

	static const struct selnl_msg_policyload msg = {
		.seqno = 0xabdcfabc
	};
	TEST_NETLINK_OBJECT(fd, nlh0,
			    SELNL_MSG_POLICYLOAD, NLM_F_REQUEST, msg,
			    PRINT_FIELD_U("{", msg, seqno);
			    printf("}"));
}

int main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	int fd = create_nl_socket(NETLINK_SELINUX);

	test_nlmsg_type(fd);
	test_selnl_msg_unspec(fd);
	test_selnl_msg_setenforce(fd);
	test_selnl_msg_policyload(fd);

	printf("+++ exited with 0 +++\n");

	return 0;
}
