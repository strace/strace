/*
 * Copyright (c) 2017-2023 The strace developers.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/* This test case is based on netlink_selinux.c */

#include "tests.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "netlink.h"
#include <linux/genetlink.h>

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
	printf("sendto(%d, [{nlmsg_len=%u, nlmsg_type=nlctrl"
	       ", nlmsg_flags=NLM_F_REQUEST|0x300, nlmsg_seq=0, nlmsg_pid=0}"
	       ", \"\\x03\\x00\\x00\\x00\"], %u"
	       ", MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, req.nlh.nlmsg_len,
	       (unsigned int) sizeof(req), sprintrc(rc));
}

static void
test_sendmsg_nlmsg_type(const int fd)
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

        struct iovec iov[1] = {
		{ .iov_base = &req, .iov_len = sizeof(req) }
        };
        struct msghdr msg = {
		.msg_iov = iov,
		.msg_iovlen = 1
        };

        rc = sendmsg(fd, &msg, MSG_DONTWAIT);
        printf("sendmsg(%d, {msg_name=NULL, msg_namelen=0"
	       ", msg_iov=[{iov_base=[{nlmsg_len=%u, nlmsg_type=nlctrl"
	       ", nlmsg_flags=NLM_F_REQUEST|0x300, nlmsg_seq=0, nlmsg_pid=0}"
	       ", \"\\x03\\x00\\x00\\x00\"], iov_len=%u}], msg_iovlen=1"
	       ", msg_controllen=0, msg_flags=0}, MSG_DONTWAIT) = %s\n",
	       fd, req.nlh.nlmsg_len, (unsigned int) iov[0].iov_len,
	       sprintrc(rc));
}

int main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	int fd = create_nl_socket(NETLINK_GENERIC);

	test_nlmsg_type(fd);
	test_sendmsg_nlmsg_type(fd);

	printf("+++ exited with 0 +++\n");

	return 0;
}
