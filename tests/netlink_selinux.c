/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
	printf("sendto(%d, {nlmsg_len=%u, nlmsg_type=SELNL_MSG_SETENFORCE"
	       ", nlmsg_flags=NLM_F_REQUEST, nlmsg_seq=0, nlmsg_pid=0}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, nlh.nlmsg_len, (unsigned) sizeof(nlh), sprintrc(rc));
}

static void
test_selnl_msg_unspec(const int fd)
{
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, 4);

	TEST_NETLINK_(fd, nlh0,
		      0xffff, "0xffff /* SELNL_MSG_??? */",
		      NLM_F_REQUEST, "NLM_F_REQUEST",
		      4, "1234", 4,
		      printf("\"\\x31\\x32\\x33\\x34\""));
}

static void
test_selnl_msg_setenforce(const int fd)
{
	static const struct selnl_msg_setenforce msg = {
		.val = 0xfbdcdfab
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(msg));

	TEST_NETLINK_OBJECT(fd, nlh0,
			    SELNL_MSG_SETENFORCE, NLM_F_REQUEST, msg,
			    printf("{");
			    PRINT_FIELD_D(msg, val);
			    printf("}"));
}

static void
test_selnl_msg_policyload(const int fd)
{
	static const struct selnl_msg_policyload msg = {
		.seqno = 0xabdcfabc
	};
	void *const nlh0 = midtail_alloc(NLMSG_HDRLEN, sizeof(msg));

	TEST_NETLINK_OBJECT(fd, nlh0,
			    SELNL_MSG_POLICYLOAD, NLM_F_REQUEST, msg,
			    printf("{");
			    PRINT_FIELD_U(msg, seqno);
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
