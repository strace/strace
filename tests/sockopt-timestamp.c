/*
 * Check decoding of timestamp control messages.
 *
 * Copyright (c) 2019 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2019-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#if defined HAVE_STRUCT___KERNEL_SOCK_TIMEVAL	\
 || defined HAVE_STRUCT___KERNEL_TIMESPEC
# include <linux/time_types.h>
#endif

#include "kernel_timeval.h"
#include "kernel_old_timespec.h"

#define XLAT_MACROS_ONLY
# include "xlat/sock_options.h"
#undef XLAT_MACROS_ONLY

#undef TEST_OLD_SCM_TIMESTAMPS

/*
 * Sadly, starting with commit
 * glibc-2.33.9000-707-g13c51549e2077f2f3bf84e8fd0b46d8b0c615912, on every
 * 32-bit architecture where 32-bit time_t support is enabled,
 * glibc mangles old scm timestamps.
 */
#if GLIBC_PREREQ_GE(2, 33) && defined __TIMESIZE && __TIMESIZE != 64
# define TEST_OLD_SCM_TIMESTAMPS 0
#endif

#ifndef TEST_OLD_SCM_TIMESTAMPS
# define TEST_OLD_SCM_TIMESTAMPS 1
#endif

#if TEST_OLD_SCM_TIMESTAMPS \
 || defined HAVE_STRUCT___KERNEL_TIMESPEC \
 || defined HAVE_STRUCT___KERNEL_SOCK_TIMEVAL

# if TEST_OLD_SCM_TIMESTAMPS
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
# endif /* TEST_OLD_SCM_TIMESTAMPS */

# ifdef HAVE_STRUCT___KERNEL_SOCK_TIMEVAL
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
# endif /* HAVE_STRUCT___KERNEL_SOCK_TIMEVAL */

# ifdef HAVE_STRUCT___KERNEL_TIMESPEC
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
# endif /* HAVE_STRUCT___KERNEL_TIMESPEC */

static unsigned int
test_sockopt(int so_val, const char *str, void (*fun)(const struct cmsghdr *))
{
	static const char data[] = "socketpair";
	const size_t size = sizeof(data) - 1;

	int sv[2];
	if (socketpair(AF_UNIX, SOCK_DGRAM, 0, sv))
		perror_msg_and_skip(data);

	const int opt_1 = 1;
	if (setsockopt(sv[0], SOL_SOCKET, so_val, &opt_1, sizeof(opt_1))) {
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

	if (recvmsg(sv[0], &mh, 0) != (int) size)
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
# if TEST_OLD_SCM_TIMESTAMPS
		{ SO_TIMESTAMP_OLD, "SO_TIMESTAMP_OLD", print_timestamp_old },
		{ SO_TIMESTAMPNS_OLD, "SO_TIMESTAMPNS_OLD", print_timestampns_old },
# endif
# ifdef HAVE_STRUCT___KERNEL_SOCK_TIMEVAL
		{ SO_TIMESTAMP_NEW, "SO_TIMESTAMP_NEW", print_timestamp_new },
# endif
# ifdef HAVE_STRUCT___KERNEL_TIMESPEC
		{ SO_TIMESTAMPNS_NEW, "SO_TIMESTAMPNS_NEW", print_timestampns_new },
# endif
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

SKIP_MAIN_UNDEFINED("TEST_OLD_SCM_TIMESTAMPS"
		    " || HAVE_STRUCT___KERNEL_TIMESPEC"
		    " || HAVE_STRUCT___KERNEL_SOCK_TIMEVAL")

#endif
