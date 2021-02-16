/*
 * This file is part of inet-yy strace test.
 *
 * Copyright (c) 2014-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2014-2021 The strace developers.
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
#include <linux/inet_diag.h>

static void
send_query(const int fd, const int family, const int proto)
{
	struct sockaddr_nl nladdr = {
		.nl_family = AF_NETLINK
	};
	struct {
		struct nlmsghdr nlh;
		struct inet_diag_req_v2 idr;
	} req = {
		.nlh = {
			.nlmsg_len = sizeof(req),
			.nlmsg_type = SOCK_DIAG_BY_FAMILY,
			.nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST
		},
		.idr = {
			.sdiag_family = family,
			.sdiag_protocol = proto,
			.idiag_states = -1
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

	const struct inet_diag_msg *diag = NLMSG_DATA(h);
	if (h->nlmsg_len < NLMSG_LENGTH(sizeof(*diag)))
		error_msg_and_skip("short response");
}

int main(void)
{
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	close(0);
	close(1);

	if (socket(AF_INET, SOCK_STREAM, 0))
		perror_msg_and_skip("socket AF_INET");
	if (bind(0, (struct sockaddr *) &addr, len))
		perror_msg_and_skip("bind");
	if (listen(0, 5))
		perror_msg_and_skip("listen");
	if (socket(AF_NETLINK, SOCK_RAW, NETLINK_INET_DIAG) != 1)
		perror_msg_and_skip("socket AF_NETLINK");

	send_query(1, AF_INET, IPPROTO_TCP);
	check_responses(1);
	return 0;
}
