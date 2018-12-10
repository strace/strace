/*
 * This file is part of net-yy-netlink strace test.
 *
 * Copyright (c) 2014-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include "netlink.h"
#include <linux/sock_diag.h>
#include <linux/netlink_diag.h>

static void
send_query(const int fd)
{
	struct sockaddr_nl nladdr = {
		.nl_family = AF_NETLINK
	};
	struct {
		struct nlmsghdr nlh;
		struct netlink_diag_req ndr;
	} req = {
		.nlh = {
			.nlmsg_len = sizeof(req),
			.nlmsg_type = SOCK_DIAG_BY_FAMILY,
			.nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST
		},
		.ndr = {
			.sdiag_family = AF_NETLINK,
			.sdiag_protocol = NDIAG_PROTO_ALL
		}
	};
	struct iovec iov = {
		.iov_base = &req,
		.iov_len = sizeof(req)
	};
	struct msghdr msg = {
		.msg_name = (void *) &nladdr,
		.msg_namelen = sizeof(nladdr),
		.msg_iov = &iov,
		.msg_iovlen = 1
	};

	if (sendmsg(fd, &msg, 0) <= 0)
		perror_msg_and_skip("sendmsg");
}

static void
check_responses(const int fd)
{
	static union {
		struct nlmsghdr hdr;
		long buf[8192 / sizeof(long)];
	} hdr_buf;

	struct sockaddr_nl nladdr = {
		.nl_family = AF_NETLINK
	};
	struct iovec iov = {
		.iov_base = hdr_buf.buf,
		.iov_len = sizeof(hdr_buf.buf)
	};
	struct msghdr msg = {
		.msg_name = (void *) &nladdr,
		.msg_namelen = sizeof(nladdr),
		.msg_iov = &iov,
		.msg_iovlen = 1
	};

	ssize_t ret = recvmsg(fd, &msg, 0);
	if (ret <= 0)
		perror_msg_and_skip("recvmsg");

	struct nlmsghdr *h = &hdr_buf.hdr;
	if (!is_nlmsg_ok(h, ret))
		error_msg_and_skip("!is_nlmsg_ok");
	if (h->nlmsg_type == NLMSG_ERROR) {
		const struct nlmsgerr *err = NLMSG_DATA(h);
		if (h->nlmsg_len < NLMSG_LENGTH(sizeof(*err)))
			error_msg_and_skip("NLMSG_ERROR");
		errno = -err->error;
		perror_msg_and_skip("NLMSG_ERROR");
	}
	if (h->nlmsg_type != SOCK_DIAG_BY_FAMILY)
		error_msg_and_skip("unexpected nlmsg_type %u",
				   (unsigned) h->nlmsg_type);

	const struct netlink_diag_msg *diag = NLMSG_DATA(h);
	if (h->nlmsg_len < NLMSG_LENGTH(sizeof(*diag)))
		error_msg_and_skip("short response");
}

int main(void)
{
	struct sockaddr_nl addr;
	socklen_t len = sizeof(addr);

	memset(&addr, 0, sizeof(addr));
	addr.nl_family = AF_NETLINK;

	close(0);
	close(1);

	if (socket(AF_NETLINK, SOCK_RAW, NETLINK_SOCK_DIAG))
		perror_msg_and_skip("socket AF_NETLINK");
	if (bind(0, (struct sockaddr *) &addr, len))
		perror_msg_and_skip("bind");

	if (socket(AF_NETLINK, SOCK_RAW, NETLINK_SOCK_DIAG) != 1)
		perror_msg_and_skip("socket AF_NETLINK");

	send_query(1);
	check_responses(1);
	return 0;
}
