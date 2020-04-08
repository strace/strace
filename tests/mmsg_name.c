/*
 * Check decoding of msg_name* fields of struct msghdr array argument
 * of sendmmsg and recvmmsg syscalls.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>

#include "msghdr.h"

#define IOV_MAX1 (IOV_MAX + 1)

#ifndef TEST_NAME
# define TEST_NAME "mmsg_name"
#endif

static void
print_msghdr(const struct msghdr *const msg, const int user_msg_namelen)
{
	const struct sockaddr_un *const un = msg->msg_name;
	const int offsetof_sun_path = offsetof(struct sockaddr_un, sun_path);

	printf("{msg_name=");
	if (!un)
		printf("NULL");
	else if (user_msg_namelen < offsetof_sun_path) {
		printf("%p", un);
	} else {
		printf("{sa_family=AF_UNIX");
		if (user_msg_namelen > offsetof_sun_path) {
			int len = user_msg_namelen < (int) msg->msg_namelen ?
				  user_msg_namelen : (int) msg->msg_namelen;
			len -= offsetof_sun_path;
			if (len > (int) sizeof(un->sun_path))
				len = sizeof(un->sun_path);
			printf(", sun_path=\"%.*s\"", len, un->sun_path);
		}
		printf("}");
	}
	printf(", msg_namelen=");
	if (user_msg_namelen != (int) msg->msg_namelen) {
		printf("%d->", user_msg_namelen);
	}
	printf("%d, msg_iov=[{iov_base=\"%c\", iov_len=1}]"
	       ", msg_iovlen=1, msg_controllen=0, msg_flags=0}",
	       (int) msg->msg_namelen, *(char *) msg->msg_iov[0].iov_base);
}

static void
test_mmsg_name(const int send_fd, const int recv_fd)
{
	struct sockaddr_un *const send_addr =
		tail_alloc(sizeof(*send_addr) * IOV_MAX1);
	char *const send_buf = tail_alloc(sizeof(*send_buf) * IOV_MAX1);
	struct iovec *const send_iov = tail_alloc(sizeof(*send_iov) * IOV_MAX1);
	struct mmsghdr *const send_mh = tail_alloc(sizeof(*send_mh) * IOV_MAX1);

	int i, rc;

	for (i = 0; i < IOV_MAX1; ++i) {
		int sun_len = i + 1 > (int) sizeof(send_addr[i].sun_path)
				    ? (int) sizeof(send_addr[i].sun_path)
				    : i + 1;

		send_addr[i].sun_family = AF_UNIX;
		memset(send_addr[i].sun_path, 'a' + i % 26, sun_len);

		send_buf[i] = '0' + i % 10;

		send_iov[i].iov_base = &send_buf[i];
		send_iov[i].iov_len = sizeof(*send_buf);

		send_mh[i].msg_hdr.msg_iov = &send_iov[i];
		send_mh[i].msg_hdr.msg_iovlen = 1;
		send_mh[i].msg_hdr.msg_name = &send_addr[i];
		send_mh[i].msg_hdr.msg_namelen = i + 1;
		send_mh[i].msg_hdr.msg_control = 0;
		send_mh[i].msg_hdr.msg_controllen = 0;
		send_mh[i].msg_hdr.msg_flags = 0;
	}

	rc = send_mmsg(send_fd, send_mh, IOV_MAX1, MSG_DONTWAIT);
	int saved_errno = errno;

	printf("sendmmsg(%d, [", send_fd);
	for (i = 0; i < IOV_MAX1; ++i) {
		if (i)
			printf(", ");
		if (i >= IOV_MAX
#if !VERBOSE
			|| i >= DEFAULT_STRLEN
#endif
		   ) {
			printf("...");
			break;
		}
		printf("{msg_hdr=");
		print_msghdr(&send_mh[i].msg_hdr, i + 1);
		printf("}");
	}
	errno = saved_errno;
	printf("], %u, MSG_DONTWAIT) = %d %s (%m)\n",
	       IOV_MAX1, rc, errno2name());

	for (i = 0; i < IOV_MAX1; ++i) {
		send_mh[i].msg_hdr.msg_name = 0;
		send_mh[i].msg_hdr.msg_namelen = 0;
	}

	/*
	 * When recvmmsg is called with a valid descriptor
	 * but inaccessible memory, it causes segfaults on some architectures.
	 * As in these cases we test decoding of failed recvmmsg calls,
	 * it's ok to fail recvmmsg with any reason as long as
	 * it doesn't read that inaccessible memory.
	 */
	rc = send_mmsg(-1, &send_mh[IOV_MAX], 2, MSG_DONTWAIT);
	saved_errno = errno;
	printf("sendmmsg(-1, [{msg_hdr=");
	print_msghdr(&send_mh[IOV_MAX].msg_hdr, 0);
	errno = saved_errno;
	printf("}, ... /* %p */], %u, MSG_DONTWAIT) = %d %s (%m)\n",
	       &send_mh[IOV_MAX1], 2, rc, errno2name());

	rc = send_mmsg(send_fd, send_mh, IOV_MAX1, MSG_DONTWAIT);
	if (rc < 0)
		perror_msg_and_skip("sendmmsg");

	printf("sendmmsg(%d, [", send_fd);
	for (i = 0; i < IOV_MAX1; ++i) {
		if (i)
			printf(", ");
		if (i >= IOV_MAX
#if !VERBOSE
			|| i >= DEFAULT_STRLEN
#endif
		   ) {
			printf("...");
			break;
		}
		printf("{msg_hdr=");
		print_msghdr(&send_mh[i].msg_hdr, 0);
		printf("%s}", i < rc ? ", msg_len=1" : "");
	}
	printf("], %u, MSG_DONTWAIT) = %d\n", IOV_MAX1, rc);

	struct sockaddr_un *const recv_addr =
		tail_alloc(sizeof(*recv_addr) * IOV_MAX1);
	char *const recv_buf = tail_alloc(sizeof(*recv_buf) * IOV_MAX1);
	struct iovec *const recv_iov = tail_alloc(sizeof(*recv_iov) * IOV_MAX1);
	struct mmsghdr *const recv_mh = tail_alloc(sizeof(*recv_mh) * IOV_MAX1);

	for (i = 0; i < IOV_MAX1; ++i) {
		recv_iov[i].iov_base = &recv_buf[i];
		recv_iov[i].iov_len = sizeof(*recv_buf);

		recv_mh[i].msg_hdr.msg_name = &recv_addr[i];
		recv_mh[i].msg_hdr.msg_namelen = i;
		recv_mh[i].msg_hdr.msg_iov = &recv_iov[i];
		recv_mh[i].msg_hdr.msg_iovlen = 1;
		recv_mh[i].msg_hdr.msg_control = 0;
		recv_mh[i].msg_hdr.msg_controllen = 0;
		recv_mh[i].msg_hdr.msg_flags = 0;
	}

	rc = recv_mmsg(recv_fd, recv_mh, IOV_MAX1, MSG_DONTWAIT, 0);
	if (rc < 0)
		perror_msg_and_skip("recvmmsg");

	printf("recvmmsg(%d, [", recv_fd);
	for (i = 0; i < rc; ++i) {
		if (i)
			printf(", ");
#if !VERBOSE
		if (i >= DEFAULT_STRLEN) {
			printf("...");
			break;
		}
#endif
		printf("{msg_hdr=");
		print_msghdr(&recv_mh[i].msg_hdr, i);
		printf(", msg_len=1}");
	}
	printf("], %u, MSG_DONTWAIT, NULL) = %d\n", IOV_MAX1, rc);
}

int
main(void)
{
	int fds[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds))
		perror_msg_and_skip("socketpair");

	const struct sockaddr_un un = {
		.sun_family = AF_UNIX,
		.sun_path = TEST_NAME "-recvmmsg.test.send.socket"
	};

	(void) unlink(un.sun_path);
	if (bind(fds[1], (const void *) &un, sizeof(un)))
		perror_msg_and_skip("bind");
	(void) unlink(un.sun_path);

	test_mmsg_name(fds[1], fds[0]);

	puts("+++ exited with 0 +++");
	return 0;
}
