/*
 * Check decoding of timestamp control messages.
 *
 * Copyright (c) 2019 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2019-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_recvmsg

# include <errno.h>
# include <stdio.h>
# include <string.h>
# include <unistd.h>
# include <sys/socket.h>

# include "kernel_time_types.h"
# include "kernel_timeval.h"
# include "kernel_old_timespec.h"
# include "k_sockopt.h"

# define XLAT_MACROS_ONLY
#  include "xlat/sock_options.h"
# undef XLAT_MACROS_ONLY

static const char *errstr;

static long
k_recvmsg(const unsigned int fd, const void *const ptr, const unsigned int flags)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const kernel_ulong_t arg1 = fill | fd;
	const kernel_ulong_t arg2 = (uintptr_t) ptr;
	const kernel_ulong_t arg3 = fill | flags;
	const long rc = syscall(__NR_recvmsg, arg1, arg2, arg3, bad, bad, bad);
	if (rc && errno == ENOSYS)
		perror_msg_and_skip("recvmsg");
	errstr = sprintrc(rc);
	return rc;
}

static void
print_timestamp_old(const struct cmsghdr *c)
{
	const void *cmsg_header = c;
	const void *cmsg_data = CMSG_DATA(c);
	kernel_old_timeval_t tv;
	const unsigned int expected_len = sizeof(tv);
	const unsigned int data_len = c->cmsg_len - (cmsg_data - cmsg_header);

	if (expected_len != data_len)
		perror_msg_and_fail("sizeof(struct timeval) = %u"
				    ", data_len = %u\n",
				    expected_len, data_len);

	memcpy(&tv, cmsg_data, sizeof(tv));
	printf("{tv_sec=%lld, tv_usec=%lld}",
	       (long long) tv.tv_sec, (long long) tv.tv_usec);
}

static void
print_timestampns_old(const struct cmsghdr *c)
{
	const void *cmsg_header = c;
	const void *cmsg_data = CMSG_DATA(c);
	kernel_old_timespec_t ts;
	const unsigned int expected_len = sizeof(ts);
	const unsigned int data_len = c->cmsg_len - (cmsg_data - cmsg_header);

	if (expected_len != data_len)
		perror_msg_and_fail("sizeof(struct timespec) = %u"
				    ", data_len = %u\n",
				    expected_len, data_len);

	memcpy(&ts, cmsg_data, sizeof(ts));
	printf("{tv_sec=%lld, tv_nsec=%lld}",
	       (long long) ts.tv_sec, (long long) ts.tv_nsec);
}

static void
print_timestamp_new(const struct cmsghdr *c)
{
	const void *cmsg_header = c;
	const void *cmsg_data = CMSG_DATA(c);
	struct __kernel_sock_timeval tv;
	const unsigned int expected_len = sizeof(tv);
	const unsigned int data_len = c->cmsg_len - (cmsg_data - cmsg_header);

	if (expected_len != data_len)
		perror_msg_and_fail("sizeof(struct __kernel_sock_timeval) = %u"
				    ", data_len = %u\n",
				    expected_len, data_len);

	memcpy(&tv, cmsg_data, sizeof(tv));
	printf("{tv_sec=%lld, tv_usec=%lld}",
	       (long long) tv.tv_sec, (long long) tv.tv_usec);
}

static void
print_timestampns_new(const struct cmsghdr *c)
{
	const void *cmsg_header = c;
	const void *cmsg_data = CMSG_DATA(c);
	struct __kernel_timespec ts;
	const unsigned int expected_len = sizeof(ts);
	const unsigned int data_len = c->cmsg_len - (cmsg_data - cmsg_header);

	if (expected_len != data_len)
		perror_msg_and_fail("sizeof(struct __kernel_timespec) = %u"
				    ", data_len = %u\n",
				    expected_len, data_len);

	memcpy(&ts, cmsg_data, sizeof(ts));
	printf("{tv_sec=%lld, tv_nsec=%lld}",
	       (long long) ts.tv_sec, (long long) ts.tv_nsec);
}

static unsigned int
test_sockopt(int so_val, const char *str, void (*fun)(const struct cmsghdr *))
{
	static const char data[] = "socketpair";
	const size_t size = sizeof(data) - 1;

	int sv[2];
	if (socketpair(AF_UNIX, SOCK_DGRAM, 0, sv))
		perror_msg_and_skip(data);

	const int opt_1 = 1;
	/*
	 * glibc-2.34~294 adds a fallback for SO_TIMESTAMP{,NS}_NEW that calls
	 * SO_TIMESTAMP{,NS}_OLD, so we have to call the setsockopt directly
	 * in order to avoid unexpected recvmsg msg types.
	 */
	if (k_setsockopt(sv[0], SOL_SOCKET, so_val, &opt_1, sizeof(opt_1))) {
		perror(str);
		return 0;
	}

	if (send(sv[1], data, size, 0) != (int) size)
		perror_msg_and_fail("send");
	if (close(sv[1]))
		perror_msg_and_fail("close send");

	char buf[size];
	struct iovec iov = {
		.iov_base = buf,
		.iov_len = sizeof(buf)
	};
	struct cmsghdr control[16];
	struct msghdr mh = {
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_control = control,
		.msg_controllen = sizeof(control)
	};

	if (k_recvmsg(sv[0], &mh, 0) != (int) size)
		perror_msg_and_fail("recvmsg");
	if (close(sv[0]))
		perror_msg_and_fail("close recv");

	printf("recvmsg(%d, {msg_name=NULL, msg_namelen=0"
	       ", msg_iov=[{iov_base=\"%s\", iov_len=%u}], msg_iovlen=1",
	       sv[0], data, (unsigned int) size);

	unsigned int tested = 0;
	if (mh.msg_controllen) {
		printf(", msg_control=[");
		for (struct cmsghdr *c = CMSG_FIRSTHDR(&mh); c;
		     c = CMSG_NXTHDR(&mh, c)) {
			printf("%s{cmsg_len=%lu, cmsg_level=",
			       (c == control ? "" : ", "),
			       (unsigned long) c->cmsg_len);
			if (c->cmsg_level == SOL_SOCKET) {
				printf("SOL_SOCKET");
			} else {
				printf("%d /* expected SOL_SOCKET == %d */",
				       c->cmsg_level, (int) SOL_SOCKET);
			}
			printf(", cmsg_type=");
			if (c->cmsg_type == so_val) {
				printf("%s, cmsg_data=", str);
				fun(c);
				tested = 1;
			} else {
				printf("%d /* expected %d */",
				       c->cmsg_type, so_val);
			}
			printf("}");
		}
		printf("]");
	}
	printf(", msg_controllen=%lu, msg_flags=0}, 0) = %u\n",
	       (unsigned long) mh.msg_controllen, (unsigned int) size);

	return tested;
}

int
main(void)
{
	static const struct {
		int val;
		const char *str;
		void (*fun)(const struct cmsghdr *);
	} tests[] = {
		{ SO_TIMESTAMP_OLD, "SO_TIMESTAMP_OLD", print_timestamp_old },
		{ SO_TIMESTAMPNS_OLD, "SO_TIMESTAMPNS_OLD", print_timestampns_old },
		{ SO_TIMESTAMP_NEW, "SO_TIMESTAMP_NEW", print_timestamp_new },
		{ SO_TIMESTAMPNS_NEW, "SO_TIMESTAMPNS_NEW", print_timestampns_new },
	};
	unsigned int tested = 0;
	for (unsigned int i = 0; i < ARRAY_SIZE(tests); ++i)
		tested |= test_sockopt(tests[i].val,
				       tests[i].str,
				       tests[i].fun);
	if (!tested)
		return 77;

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_recvmsg")

#endif
