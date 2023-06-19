/*
 * Check decoding of struct msghdr ancillary data.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2023 The strace developers.
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
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "kernel_time_types.h"
#include "kernel_timeval.h"
#include "kernel_old_timespec.h"

#include "xlat.h"
#define XLAT_MACROS_ONLY
#include "xlat/ip_cmsg_types.h"
#include "xlat/sock_options.h"
#include "xlat/scmvals.h"
#undef XLAT_MACROS_ONLY

#ifndef SOL_IP
# define SOL_IP 0
#endif
#ifndef SOL_TCP
# define SOL_TCP 6
#endif

static struct cmsghdr *
get_cmsghdr(void *const page, const size_t len)
{
	return page - CMSG_ALIGN(len);
}

static void
print_fds(const struct cmsghdr *const cmsg, const size_t cmsg_len)
{
	size_t nfd = cmsg_len > CMSG_LEN(0)
		     ? (cmsg_len - CMSG_LEN(0)) / sizeof(int) : 0;
	if (!nfd)
		return;

	printf(", cmsg_data=[");
	int *fdp = (int *) CMSG_DATA(cmsg);
	for (size_t i = 0; i < nfd; ++i) {
		if (i)
			printf(", ");
#if !VERBOSE
		if (i >= DEFAULT_STRLEN) {
			printf("...");
			break;
		}
#endif
		printf("%d", fdp[i]);
	}
	printf("]");
}

static void
test_scm_rights1(struct msghdr *const mh,
		 const size_t msg_controllen,
		 void *const page,
		 const void *const src,
		 const size_t cmsg_len)
{
	const size_t aligned_cms_len =
		cmsg_len > CMSG_LEN(0) ? CMSG_ALIGN(cmsg_len) : CMSG_LEN(0);
	if (cmsg_len >= CMSG_LEN(0)
	    && aligned_cms_len + CMSG_LEN(0) <= msg_controllen)
		return;

	struct cmsghdr *cmsg = get_cmsghdr(page, msg_controllen);

	if (msg_controllen >= offsetofend(struct cmsghdr, cmsg_len))
		cmsg->cmsg_len = cmsg_len;
	if (msg_controllen >= offsetofend(struct cmsghdr, cmsg_level))
		cmsg->cmsg_level = SOL_SOCKET;
	if (msg_controllen >= offsetofend(struct cmsghdr, cmsg_type))
		cmsg->cmsg_type = SCM_RIGHTS;

	size_t src_len =
		cmsg_len < msg_controllen ? cmsg_len : msg_controllen;
	if (src_len > CMSG_LEN(0))
		memcpy(CMSG_DATA(cmsg), src, src_len - CMSG_LEN(0));

	mh->msg_control = cmsg;
	mh->msg_controllen = msg_controllen;

	int rc = sendmsg(-1, mh, 0);
	int saved_errno = errno;

	printf("sendmsg(-1, {msg_name=NULL, msg_namelen=0, msg_iov=NULL"
	       ", msg_iovlen=0");
	if (msg_controllen < CMSG_LEN(0)) {
		if (msg_controllen)
			printf(", msg_control=%p", cmsg);
	} else {
		printf(", msg_control=[{cmsg_len=%lu, cmsg_level=SOL_SOCKET"
		       ", cmsg_type=SCM_RIGHTS", (unsigned long) cmsg_len);
		print_fds(cmsg, src_len);
		printf("}");
		if (aligned_cms_len < msg_controllen)
			printf(", ... /* %p */", (void *) cmsg + aligned_cms_len);
		printf("]");
	}

	errno = saved_errno;
	printf(", msg_controllen=%lu, msg_flags=0}, 0) = %d %s (%m)\n",
	       (unsigned long) msg_controllen, rc, errno2name());
}

static void
test_scm_rights2(struct msghdr *const mh,
		 const size_t msg_controllen,
		 void *const page,
		 const int *const *const src,
		 const size_t *const cmsg_len)
{
	const size_t aligned_cms_len[2] = {
		cmsg_len[0] > CMSG_LEN(0) ? CMSG_ALIGN(cmsg_len[0]) : CMSG_LEN(0),
		cmsg_len[1] > CMSG_LEN(0) ? CMSG_ALIGN(cmsg_len[1]) : CMSG_LEN(0)
	};
	if (cmsg_len[0] < CMSG_LEN(0)
	    || aligned_cms_len[0] + CMSG_LEN(0) > msg_controllen
	    || aligned_cms_len[0] + aligned_cms_len[1] + CMSG_LEN(0) <= msg_controllen)
		return;

	struct cmsghdr *const cmsg[2] = {
		get_cmsghdr(page, msg_controllen),
		(void *) get_cmsghdr(page, msg_controllen) + aligned_cms_len[0]
	};
	cmsg[0]->cmsg_len = cmsg_len[0];
	cmsg[0]->cmsg_level = SOL_SOCKET;
	cmsg[0]->cmsg_type = SCM_RIGHTS;
	if (cmsg_len[0] > CMSG_LEN(0))
		memcpy(CMSG_DATA(cmsg[0]), src[0], cmsg_len[0] - CMSG_LEN(0));

	const size_t msg_controllen1 = msg_controllen - aligned_cms_len[0];
	if (msg_controllen1 >= offsetofend(struct cmsghdr, cmsg_len))
		cmsg[1]->cmsg_len = cmsg_len[1];
	if (msg_controllen >= offsetofend(struct cmsghdr, cmsg_level))
		cmsg[1]->cmsg_level = SOL_SOCKET;
	if (msg_controllen >= offsetofend(struct cmsghdr, cmsg_type))
		cmsg[1]->cmsg_type = SCM_RIGHTS;
	size_t src1_len =
		cmsg_len[1] < msg_controllen1 ? cmsg_len[1] : msg_controllen1;
	if (src1_len > CMSG_LEN(0))
		memcpy(CMSG_DATA(cmsg[1]), src[1], src1_len - CMSG_LEN(0));

	mh->msg_control = cmsg[0];
	mh->msg_controllen = msg_controllen;

	int rc = sendmsg(-1, mh, 0);
	int saved_errno = errno;

	printf("sendmsg(-1, {msg_name=NULL, msg_namelen=0, msg_iov=NULL"
	       ", msg_iovlen=0, msg_control=[{cmsg_len=%lu"
	       ", cmsg_level=SOL_SOCKET, cmsg_type=SCM_RIGHTS",
	       (unsigned long) cmsg_len[0]);
	print_fds(cmsg[0], cmsg_len[0]);
	printf("}, {cmsg_len=%lu, cmsg_level=SOL_SOCKET, cmsg_type=SCM_RIGHTS",
	       (unsigned long) cmsg_len[1]);
	print_fds(cmsg[1], src1_len);
	printf("}");
	if (aligned_cms_len[1] < msg_controllen1)
		printf(", ... /* %p */", (void *) cmsg[1] + aligned_cms_len[1]);
	printf("]");

	errno = saved_errno;
	printf(", msg_controllen=%lu, msg_flags=0}, 0) = %d %s (%m)\n",
	       (unsigned long) msg_controllen, rc, errno2name());
}

static void
test_scm_rights3(struct msghdr *const mh, void *const page, const size_t nfds)
{
	const size_t len = CMSG_SPACE(sizeof(int) * nfds);
	struct cmsghdr *cmsg = get_cmsghdr(page, len);

	cmsg->cmsg_len = CMSG_LEN(sizeof(int) * nfds);
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	int *fdp = (int *) CMSG_DATA(cmsg);
	for (size_t i = 0; i < nfds; ++i)
		fdp[i] = i;

	mh->msg_control = cmsg;
	mh->msg_controllen = len;

	int rc = sendmsg(-1, mh, 0);
	printf("sendmsg(-1, {msg_name=NULL, msg_namelen=0, msg_iov=NULL"
	       ", msg_iovlen=0, msg_control=[{cmsg_len=%u"
	       ", cmsg_level=SOL_SOCKET, cmsg_type=SCM_RIGHTS",
	       (unsigned) cmsg->cmsg_len);
	print_fds(cmsg, cmsg->cmsg_len);
	printf("}], msg_controllen=%lu, msg_flags=0}, 0) = %d %s (%m)\n",
	       (unsigned long) len, rc, errno2name());
}

static void
test_scm_timestamp_old(struct msghdr *const mh, void *const page)
{
	static const kernel_old_timeval_t tv = {
		.tv_sec = 123456789,
		.tv_usec = 987654
	};
	size_t len = CMSG_SPACE(sizeof(tv));
	struct cmsghdr *cmsg = get_cmsghdr(page, len);

	cmsg->cmsg_len = CMSG_LEN(sizeof(tv));
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SO_TIMESTAMP_OLD;
	memcpy(CMSG_DATA(cmsg), &tv, sizeof(tv));

	mh->msg_control = cmsg;
	mh->msg_controllen = len;

	int rc = sendmsg(-1, mh, 0);
	printf("sendmsg(-1, {msg_name=NULL, msg_namelen=0, msg_iov=NULL"
	       ", msg_iovlen=0, msg_control=[{cmsg_len=%u"
	       ", cmsg_level=SOL_SOCKET, cmsg_type=SO_TIMESTAMP_OLD"
	       ", cmsg_data={tv_sec=%lld, tv_usec=%llu}}]"
	       ", msg_controllen=%lu, msg_flags=0}, 0) = %d %s (%m)\n",
	       (unsigned) cmsg->cmsg_len,
	       (long long) tv.tv_sec, zero_extend_signed_to_ull(tv.tv_usec),
	       (unsigned long) len, rc, errno2name());

	len = CMSG_SPACE(sizeof(tv) - sizeof(long));
	cmsg = get_cmsghdr(page, len);

	cmsg->cmsg_len = CMSG_LEN(sizeof(tv) - sizeof(long));
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SO_TIMESTAMP_OLD;

	mh->msg_control = cmsg;
	mh->msg_controllen = len;

	rc = sendmsg(-1, mh, 0);
	printf("sendmsg(-1, {msg_name=NULL, msg_namelen=0, msg_iov=NULL"
	       ", msg_iovlen=0, msg_control=[{cmsg_len=%u"
	       ", cmsg_level=SOL_SOCKET, cmsg_type=SO_TIMESTAMP_OLD, cmsg_data=???}]"
	       ", msg_controllen=%lu, msg_flags=0}, 0) = %d %s (%m)\n",
	       (unsigned) cmsg->cmsg_len,
	       (unsigned long) len, rc, errno2name());
}

static void
test_scm_timestampns_old(struct msghdr *const mh, void *const page)
{
	static const kernel_old_timespec_t ts = {
		.tv_sec = 123456789,
		.tv_nsec = 987654321
	};
	size_t len = CMSG_SPACE(sizeof(ts));
	struct cmsghdr *cmsg = get_cmsghdr(page, len);

	cmsg->cmsg_len = CMSG_LEN(sizeof(ts));
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SO_TIMESTAMPNS_OLD;
	memcpy(CMSG_DATA(cmsg), &ts, sizeof(ts));

	mh->msg_control = cmsg;
	mh->msg_controllen = len;

	int rc = sendmsg(-1, mh, 0);
	printf("sendmsg(-1, {msg_name=NULL, msg_namelen=0, msg_iov=NULL"
	       ", msg_iovlen=0, msg_control=[{cmsg_len=%u"
	       ", cmsg_level=SOL_SOCKET, cmsg_type=SO_TIMESTAMPNS_OLD"
	       ", cmsg_data={tv_sec=%lld, tv_nsec=%llu}}]"
	       ", msg_controllen=%lu, msg_flags=0}, 0) = %d %s (%m)\n",
	       (unsigned) cmsg->cmsg_len,
	       (long long) ts.tv_sec, zero_extend_signed_to_ull(ts.tv_nsec),
	       (unsigned long) len, rc, errno2name());

	len = CMSG_SPACE(sizeof(ts) - sizeof(long));
	cmsg = get_cmsghdr(page, len);

	cmsg->cmsg_len = CMSG_LEN(sizeof(ts) - sizeof(long));
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SO_TIMESTAMPNS_OLD;

	mh->msg_control = cmsg;
	mh->msg_controllen = len;

	rc = sendmsg(-1, mh, 0);
	printf("sendmsg(-1, {msg_name=NULL, msg_namelen=0, msg_iov=NULL"
	       ", msg_iovlen=0, msg_control=[{cmsg_len=%u"
	       ", cmsg_level=SOL_SOCKET, cmsg_type=SO_TIMESTAMPNS_OLD"
	       ", cmsg_data=???}]"
	       ", msg_controllen=%lu, msg_flags=0}, 0) = %d %s (%m)\n",
	       (unsigned) cmsg->cmsg_len,
	       (unsigned long) len, rc, errno2name());
}

static void
test_scm_timestamping_old(struct msghdr *const mh, void *const page)
{
	static const kernel_old_timespec_t ts[] = {
		{ .tv_sec = 123456789, .tv_nsec = 987654321 },
		{ .tv_sec = 123456790, .tv_nsec = 987654320 },
		{ .tv_sec = 123456791, .tv_nsec = 987654319 }
	};
	size_t len = CMSG_SPACE(sizeof(ts));
	struct cmsghdr *cmsg = get_cmsghdr(page, len);

	cmsg->cmsg_len = CMSG_LEN(sizeof(ts));
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SO_TIMESTAMPING_OLD;
	memcpy(CMSG_DATA(cmsg), ts, sizeof(ts));

	mh->msg_control = cmsg;
	mh->msg_controllen = len;

	int rc = sendmsg(-1, mh, 0);
	printf("sendmsg(-1, {msg_name=NULL, msg_namelen=0, msg_iov=NULL"
	       ", msg_iovlen=0, msg_control=[{cmsg_len=%u"
	       ", cmsg_level=SOL_SOCKET, cmsg_type=SO_TIMESTAMPING_OLD"
	       ", cmsg_data=[{tv_sec=%lld, tv_nsec=%llu}"
	       ", {tv_sec=%lld, tv_nsec=%llu}, {tv_sec=%lld, tv_nsec=%llu}]}]"
	       ", msg_controllen=%lu, msg_flags=0}, 0) = %d %s (%m)\n",
	       (unsigned) cmsg->cmsg_len, (long long) ts[0].tv_sec,
	       zero_extend_signed_to_ull(ts[0].tv_nsec),
	       (long long) ts[1].tv_sec,
	       zero_extend_signed_to_ull(ts[1].tv_nsec),
	       (long long) ts[2].tv_sec,
	       zero_extend_signed_to_ull(ts[2].tv_nsec),
	       (unsigned long) len, rc, errno2name());

	len = CMSG_SPACE(sizeof(ts) - sizeof(long));
	cmsg = get_cmsghdr(page, len);

	cmsg->cmsg_len = CMSG_LEN(sizeof(ts) - sizeof(long));
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SO_TIMESTAMPING_OLD;

	mh->msg_control = cmsg;
	mh->msg_controllen = len;

	rc = sendmsg(-1, mh, 0);
	printf("sendmsg(-1, {msg_name=NULL, msg_namelen=0, msg_iov=NULL"
	       ", msg_iovlen=0, msg_control=[{cmsg_len=%u"
	       ", cmsg_level=SOL_SOCKET, cmsg_type=SO_TIMESTAMPING_OLD"
	       ", cmsg_data=???}]"
	       ", msg_controllen=%lu, msg_flags=0}, 0) = %d %s (%m)\n",
	       (unsigned) cmsg->cmsg_len,
	       (unsigned long) len, rc, errno2name());
}

static void
test_scm_timestamp_new(struct msghdr *const mh, void *const page)
{
	static const struct __kernel_sock_timeval tv = {
		.tv_sec = 0xdefaceddeadbeef,
		.tv_usec = 0xdec0dedcafef00d
	};
	size_t len = CMSG_SPACE(sizeof(tv));
	struct cmsghdr *cmsg = get_cmsghdr(page, len);

	cmsg->cmsg_len = CMSG_LEN(sizeof(tv));
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SO_TIMESTAMP_NEW;
	memcpy(CMSG_DATA(cmsg), &tv, sizeof(tv));

	mh->msg_control = cmsg;
	mh->msg_controllen = len;

	int rc = sendmsg(-1, mh, 0);
	printf("sendmsg(-1, {msg_name=NULL, msg_namelen=0, msg_iov=NULL"
	       ", msg_iovlen=0, msg_control=[{cmsg_len=%u"
	       ", cmsg_level=SOL_SOCKET, cmsg_type=SO_TIMESTAMP_NEW"
	       ", cmsg_data={tv_sec=%lld, tv_usec=%llu}}]"
	       ", msg_controllen=%lu, msg_flags=0}, 0) = %s\n",
	       (unsigned) cmsg->cmsg_len,
	       (long long) tv.tv_sec, zero_extend_signed_to_ull(tv.tv_usec),
	       (unsigned long) len, sprintrc(rc));

	len = CMSG_SPACE(sizeof(tv) - sizeof(long));
	cmsg = get_cmsghdr(page, len);

	cmsg->cmsg_len = CMSG_LEN(sizeof(tv) - sizeof(long));
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SO_TIMESTAMP_NEW;

	mh->msg_control = cmsg;
	mh->msg_controllen = len;

	rc = sendmsg(-1, mh, 0);
	printf("sendmsg(-1, {msg_name=NULL, msg_namelen=0, msg_iov=NULL"
	       ", msg_iovlen=0, msg_control=[{cmsg_len=%u"
	       ", cmsg_level=SOL_SOCKET, cmsg_type=SO_TIMESTAMP_NEW, cmsg_data=???}]"
	       ", msg_controllen=%lu, msg_flags=0}, 0) = %s\n",
	       (unsigned) cmsg->cmsg_len,
	       (unsigned long) len, sprintrc(rc));
}

static void
test_scm_timestampns_new(struct msghdr *const mh, void *const page)
{
	static const struct __kernel_timespec ts = {
		.tv_sec = 0xdefaceddeadbeef,
		.tv_nsec = 0xdec0dedcafef00d
	};
	size_t len = CMSG_SPACE(sizeof(ts));
	struct cmsghdr *cmsg = get_cmsghdr(page, len);

	cmsg->cmsg_len = CMSG_LEN(sizeof(ts));
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SO_TIMESTAMPNS_NEW;
	memcpy(CMSG_DATA(cmsg), &ts, sizeof(ts));

	mh->msg_control = cmsg;
	mh->msg_controllen = len;

	int rc = sendmsg(-1, mh, 0);
	printf("sendmsg(-1, {msg_name=NULL, msg_namelen=0, msg_iov=NULL"
	       ", msg_iovlen=0, msg_control=[{cmsg_len=%u"
	       ", cmsg_level=SOL_SOCKET, cmsg_type=SO_TIMESTAMPNS_NEW"
	       ", cmsg_data={tv_sec=%lld, tv_nsec=%llu}}]"
	       ", msg_controllen=%lu, msg_flags=0}, 0) = %s\n",
	       (unsigned) cmsg->cmsg_len,
	       (long long) ts.tv_sec, zero_extend_signed_to_ull(ts.tv_nsec),
	       (unsigned long) len, sprintrc(rc));

	len = CMSG_SPACE(sizeof(ts) - sizeof(long));
	cmsg = get_cmsghdr(page, len);

	cmsg->cmsg_len = CMSG_LEN(sizeof(ts) - sizeof(long));
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SO_TIMESTAMPNS_NEW;

	mh->msg_control = cmsg;
	mh->msg_controllen = len;

	rc = sendmsg(-1, mh, 0);
	printf("sendmsg(-1, {msg_name=NULL, msg_namelen=0, msg_iov=NULL"
	       ", msg_iovlen=0, msg_control=[{cmsg_len=%u"
	       ", cmsg_level=SOL_SOCKET, cmsg_type=SO_TIMESTAMPNS_NEW"
	       ", cmsg_data=???}]"
	       ", msg_controllen=%lu, msg_flags=0}, 0) = %s\n",
	       (unsigned) cmsg->cmsg_len,
	       (unsigned long) len, sprintrc(rc));
}

static void
test_scm_timestamping_new(struct msghdr *const mh, void *const page)
{
	static const struct __kernel_timespec ts[] = {
		{ .tv_sec = 0xdeface0deadbef1, .tv_nsec = 0xdec0de2cafef0d3 },
		{ .tv_sec = 0xdeface4deadbef5, .tv_nsec = 0xdec0de6cafef0d7 },
		{ .tv_sec = 0xdeface8deadbef9, .tv_nsec = 0xdec0dedcafef00d }
	};
	size_t len = CMSG_SPACE(sizeof(ts));
	struct cmsghdr *cmsg = get_cmsghdr(page, len);

	cmsg->cmsg_len = CMSG_LEN(sizeof(ts));
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SO_TIMESTAMPING_NEW;
	memcpy(CMSG_DATA(cmsg), ts, sizeof(ts));

	mh->msg_control = cmsg;
	mh->msg_controllen = len;

	int rc = sendmsg(-1, mh, 0);
	printf("sendmsg(-1, {msg_name=NULL, msg_namelen=0, msg_iov=NULL"
	       ", msg_iovlen=0, msg_control=[{cmsg_len=%u"
	       ", cmsg_level=SOL_SOCKET, cmsg_type=SO_TIMESTAMPING_NEW"
	       ", cmsg_data=[{tv_sec=%lld, tv_nsec=%llu}"
	       ", {tv_sec=%lld, tv_nsec=%llu}, {tv_sec=%lld, tv_nsec=%llu}]}]"
	       ", msg_controllen=%lu, msg_flags=0}, 0) = %s\n",
	       (unsigned) cmsg->cmsg_len, (long long) ts[0].tv_sec,
	       zero_extend_signed_to_ull(ts[0].tv_nsec),
	       (long long) ts[1].tv_sec,
	       zero_extend_signed_to_ull(ts[1].tv_nsec),
	       (long long) ts[2].tv_sec,
	       zero_extend_signed_to_ull(ts[2].tv_nsec),
	       (unsigned long) len, sprintrc(rc));

	len = CMSG_SPACE(sizeof(ts) - sizeof(long));
	cmsg = get_cmsghdr(page, len);

	cmsg->cmsg_len = CMSG_LEN(sizeof(ts) - sizeof(long));
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SO_TIMESTAMPING_NEW;

	mh->msg_control = cmsg;
	mh->msg_controllen = len;

	rc = sendmsg(-1, mh, 0);
	printf("sendmsg(-1, {msg_name=NULL, msg_namelen=0, msg_iov=NULL"
	       ", msg_iovlen=0, msg_control=[{cmsg_len=%u"
	       ", cmsg_level=SOL_SOCKET, cmsg_type=SO_TIMESTAMPING_NEW"
	       ", cmsg_data=???}]"
	       ", msg_controllen=%lu, msg_flags=0}, 0) = %s\n",
	       (unsigned) cmsg->cmsg_len,
	       (unsigned long) len, sprintrc(rc));
}

static void
print_security(const struct cmsghdr *const cmsg, const size_t cmsg_len)
{
	int n = cmsg_len > CMSG_LEN(0) ? cmsg_len - CMSG_LEN(0) : 0;
	if (!n)
		return;

	printf(", cmsg_data=\"%.*s\"", n, CMSG_DATA(cmsg));
}

static void
test_scm_security(struct msghdr *const mh,
		  const size_t msg_controllen,
		  void *const page,
		  const void *const src,
		  const size_t cmsg_len,
		  const int cmsg_level,
		  const char *const cmsg_level_str)
{
	const size_t aligned_cms_len =
		cmsg_len > CMSG_LEN(0) ? CMSG_ALIGN(cmsg_len) : CMSG_LEN(0);
	if (cmsg_len >= CMSG_LEN(0)
	    && aligned_cms_len + CMSG_LEN(0) <= msg_controllen)
		return;

	struct cmsghdr *cmsg = get_cmsghdr(page, msg_controllen);

	cmsg->cmsg_len = cmsg_len;
	cmsg->cmsg_level = cmsg_level;
	cmsg->cmsg_type = SCM_SECURITY;

	size_t src_len =
		cmsg_len < msg_controllen ? cmsg_len : msg_controllen;
	if (src_len > CMSG_LEN(0))
		memcpy(CMSG_DATA(cmsg), src, src_len - CMSG_LEN(0));

	mh->msg_control = cmsg;
	mh->msg_controllen = msg_controllen;

	int rc = sendmsg(-1, mh, 0);
	int saved_errno = errno;

	printf("sendmsg(-1, {msg_name=NULL, msg_namelen=0, msg_iov=NULL"
	       ", msg_iovlen=0, msg_control=[{cmsg_len=%lu, cmsg_level=%s"
	       ", cmsg_type=SCM_SECURITY",
	       (unsigned long) cmsg_len, cmsg_level_str);
	print_security(cmsg, src_len);
	printf("}");
	if (aligned_cms_len < msg_controllen)
		printf(", ... /* %p */", (void *) cmsg + aligned_cms_len);
	printf("]");

	errno = saved_errno;
	printf(", msg_controllen=%lu, msg_flags=0}, 0) = %d %s (%m)\n",
	       (unsigned long) msg_controllen, rc, errno2name());
}

static void
test_unknown_type(struct msghdr *const mh,
		  void *const page,
		  const int cmsg_level,
		  const char *const cmsg_level_str,
		  const char *const cmsg_type_str)
{
	struct cmsghdr *cmsg = get_cmsghdr(page, CMSG_LEN(0));

	cmsg->cmsg_len = CMSG_LEN(0);
	cmsg->cmsg_level = cmsg_level;
	cmsg->cmsg_type = 0xfacefeed;

	mh->msg_control = cmsg;
	mh->msg_controllen = cmsg->cmsg_len;

	int rc = sendmsg(-1, mh, 0);
	printf("sendmsg(-1, {msg_name=NULL, msg_namelen=0, msg_iov=NULL"
	       ", msg_iovlen=0, msg_control=[{cmsg_len=%u, cmsg_level=%s"
	       ", cmsg_type=%#x /* %s */}], msg_controllen=%u, msg_flags=0}"
	       ", 0) = %d %s (%m)\n",
	       (unsigned) cmsg->cmsg_len, cmsg_level_str, cmsg->cmsg_type,
	       cmsg_type_str, (unsigned) mh->msg_controllen, rc, errno2name());
}

static void
test_sol_socket(struct msghdr *const mh, void *const page)
{
	static const int fds0[] = { -10, -11, -12, -13 };
	static const int fds1[] = { -15, -16, -17, -18 };
	size_t max_msg_controllen;

	max_msg_controllen = CMSG_SPACE(sizeof(fds0)) + sizeof(*fds0) - 1;
	for (size_t msg_controllen = 0;
	     msg_controllen <= max_msg_controllen;
	     msg_controllen++) {
		for (size_t cmsg_len = 0;
		     cmsg_len <= msg_controllen + CMSG_LEN(0);
		     cmsg_len++) {
			test_scm_rights1(mh, msg_controllen,
					 page, fds0, cmsg_len);
		}
	}

	max_msg_controllen =
		CMSG_SPACE(sizeof(fds0)) + CMSG_SPACE(sizeof(fds1)) +
		sizeof(*fds0) - 1;
	for (size_t msg_controllen = CMSG_LEN(0) * 2;
	     msg_controllen <= max_msg_controllen;
	     msg_controllen++) {
		static const int *const fdps[] = { fds0, fds1 };
		size_t cmsg_len[2];

		for (cmsg_len[0] = CMSG_LEN(0);
		     CMSG_ALIGN(cmsg_len[0]) + CMSG_LEN(0) <= msg_controllen
		     && CMSG_ALIGN(cmsg_len[0]) <= CMSG_SPACE(sizeof(fds0));
		     cmsg_len[0]++) {
			const size_t msg_controllen1 =
				msg_controllen - CMSG_ALIGN(cmsg_len[0]);

			for (cmsg_len[1] = 0;
			     cmsg_len[1] <= msg_controllen1 + CMSG_LEN(0);
			     cmsg_len[1]++) {
				test_scm_rights2(mh, msg_controllen,
						 page, fdps, cmsg_len);
			}
		}
	}

	static const char text[16] = "0123456789abcdef";
	max_msg_controllen = CMSG_SPACE(sizeof(text)) + CMSG_LEN(0) - 1;
	for (size_t msg_controllen = CMSG_LEN(0);
	     msg_controllen <= max_msg_controllen;
	     msg_controllen++) {
		for (size_t cmsg_len = 0;
		     cmsg_len <= msg_controllen + CMSG_LEN(0)
		     && cmsg_len <= CMSG_LEN(sizeof(text));
		     cmsg_len++) {
			test_scm_security(mh, msg_controllen,
					  page, text, cmsg_len,
					  ARG_STR(SOL_SOCKET));
		}
	}

	test_scm_rights3(mh, page, DEFAULT_STRLEN - 1);
	test_scm_rights3(mh, page, DEFAULT_STRLEN);
	test_scm_rights3(mh, page, DEFAULT_STRLEN + 1);

	test_scm_timestamp_old(mh, page);
	test_scm_timestampns_old(mh, page);
	test_scm_timestamping_old(mh, page);
	test_scm_timestamp_new(mh, page);
	test_scm_timestampns_new(mh, page);
	test_scm_timestamping_new(mh, page);

	test_unknown_type(mh, page, ARG_STR(SOL_SOCKET), "SCM_???");
}

static void
test_ip_pktinfo(struct msghdr *const mh, void *const page,
		const int cmsg_type, const char *const cmsg_type_str)
{
	const unsigned int len = CMSG_SPACE(sizeof(struct in_pktinfo));
	struct cmsghdr *const cmsg = get_cmsghdr(page, len);

	cmsg->cmsg_len = CMSG_LEN(sizeof(struct in_pktinfo));
	cmsg->cmsg_level = SOL_IP;
	cmsg->cmsg_type = cmsg_type;

	struct in_pktinfo *const info = (struct in_pktinfo *) CMSG_DATA(cmsg);
	info->ipi_ifindex = ifindex_lo();
	info->ipi_spec_dst.s_addr = inet_addr("1.2.3.4");
	info->ipi_addr.s_addr = inet_addr("5.6.7.8");

	mh->msg_control = cmsg;
	mh->msg_controllen = len;

	int rc = sendmsg(-1, mh, 0);
	printf("sendmsg(-1, {msg_name=NULL, msg_namelen=0, msg_iov=NULL"
	       ", msg_iovlen=0, msg_control=[{cmsg_len=%u, cmsg_level=SOL_IP"
	       ", cmsg_type=%s, cmsg_data={ipi_ifindex=%s"
	       ", ipi_spec_dst=inet_addr(\"%s\")"
	       ", ipi_addr=inet_addr(\"%s\")}}]"
	       ", msg_controllen=%u, msg_flags=0}, 0) = %d %s (%m)\n",
	       (unsigned) cmsg->cmsg_len, cmsg_type_str,
	       IFINDEX_LO_STR, "1.2.3.4", "5.6.7.8", len, rc, errno2name());
}

static void
test_ip_uint(struct msghdr *const mh, void *const page,
	     const int cmsg_type, const char *const cmsg_type_str)
{
	const unsigned int len = CMSG_SPACE(sizeof(int));
	struct cmsghdr *const cmsg = get_cmsghdr(page, len);

	cmsg->cmsg_len = CMSG_LEN(sizeof(int));
	cmsg->cmsg_level = SOL_IP;
	cmsg->cmsg_type = cmsg_type;

	unsigned int *u = (void *) CMSG_DATA(cmsg);
	*u = 0xfacefeed;

	mh->msg_control = cmsg;
	mh->msg_controllen = len;

	int rc = sendmsg(-1, mh, 0);
	printf("sendmsg(-1, {msg_name=NULL, msg_namelen=0, msg_iov=NULL"
	       ", msg_iovlen=0, msg_control=[{cmsg_len=%u"
	       ", cmsg_level=SOL_IP, cmsg_type=%s, cmsg_data=[%u]}]"
	       ", msg_controllen=%u, msg_flags=0}, 0) = %d %s (%m)\n",
	       (unsigned) cmsg->cmsg_len, cmsg_type_str, *u, len,
	       rc, errno2name());
}

static void
test_ip_uint8_t(struct msghdr *const mh, void *const page,
		const int cmsg_type, const char *const cmsg_type_str)
{
	const unsigned int len = CMSG_SPACE(1);
	struct cmsghdr *const cmsg = get_cmsghdr(page, len);

	cmsg->cmsg_len = CMSG_LEN(1);
	cmsg->cmsg_level = SOL_IP;
	cmsg->cmsg_type = cmsg_type;
	*CMSG_DATA(cmsg) = 'A';

	mh->msg_control = cmsg;
	mh->msg_controllen = len;

	int rc = sendmsg(-1, mh, 0);
	printf("sendmsg(-1, {msg_name=NULL, msg_namelen=0, msg_iov=NULL"
	       ", msg_iovlen=0, msg_control=[{cmsg_len=%u"
	       ", cmsg_level=SOL_IP, cmsg_type=%s, cmsg_data=[%#x]}]"
	       ", msg_controllen=%u, msg_flags=0}, 0) = %d %s (%m)\n",
	       (unsigned) cmsg->cmsg_len, cmsg_type_str,
	       (unsigned) (uint8_t) 'A', len, rc, errno2name());
}

static void
print_ip_opts(const void *const cmsg_data, const unsigned int data_len)
{
	const unsigned char *const opts = cmsg_data;
	for (unsigned int i = 0; i < data_len; ++i) {
		if (i)
			printf(", ");
#if !VERBOSE
		if (i >= DEFAULT_STRLEN) {
			printf("...");
			break;
		}
#endif
		printf("0x%02x", opts[i]);
	}
}

static void
test_ip_opts(struct msghdr *const mh, void *const page,
	     const int cmsg_type, const char *const cmsg_type_str,
	     const unsigned int opts_len)
{
	unsigned int len = CMSG_SPACE(opts_len);
	struct cmsghdr *cmsg = get_cmsghdr(page, len);

	cmsg->cmsg_len = CMSG_LEN(opts_len);
	cmsg->cmsg_level = SOL_IP;
	cmsg->cmsg_type = cmsg_type;
	for (unsigned int i = 0; i < opts_len; ++i)
		CMSG_DATA(cmsg)[i] = 'A' + i;

	mh->msg_control = cmsg;
	mh->msg_controllen = len;

	int rc = sendmsg(-1, mh, 0);
	printf("sendmsg(-1, {msg_name=NULL, msg_namelen=0, msg_iov=NULL"
	       ", msg_iovlen=0, msg_control=[{cmsg_len=%u"
	       ", cmsg_level=SOL_IP, cmsg_type=%s, cmsg_data=[",
	       (unsigned) cmsg->cmsg_len, cmsg_type_str);
	print_ip_opts(CMSG_DATA(cmsg), opts_len);
	printf("]}], msg_controllen=%u, msg_flags=0}, 0) = %d %s (%m)\n",
	       len, rc, errno2name());
}

#ifdef IP_RECVERR
struct sock_ee {
	uint32_t ee_errno;
	uint8_t  ee_origin;
	uint8_t  ee_type;
	uint8_t  ee_code;
	uint8_t  ee_pad;
	uint32_t ee_info;
	uint32_t ee_data;
	struct sockaddr_in offender;
};

static void
test_ip_recverr(struct msghdr *const mh, void *const page,
		const int cmsg_type, const char *const cmsg_type_str)
{
	const unsigned int len = CMSG_SPACE(sizeof(struct sock_ee));
	struct cmsghdr *const cmsg = get_cmsghdr(page, len);

	cmsg->cmsg_len = CMSG_LEN(sizeof(struct sock_ee));
	cmsg->cmsg_level = SOL_IP;
	cmsg->cmsg_type = cmsg_type;

	struct sock_ee *const e = (struct sock_ee *) CMSG_DATA(cmsg);
	e->ee_errno = 0xdeadbeef;
	e->ee_origin = 2;
	e->ee_type = 3;
	e->ee_code = 4;
	e->ee_info = 0xfacefeed;
	e->ee_data = 0xbadc0ded;
	e->offender.sin_family = AF_INET,
	e->offender.sin_port = htons(12345),
	e->offender.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	mh->msg_control = cmsg;
	mh->msg_controllen = len;

	int rc = sendmsg(-1, mh, 0);
	printf("sendmsg(-1, {msg_name=NULL, msg_namelen=0, msg_iov=NULL"
	       ", msg_iovlen=0, msg_control=[{cmsg_len=%u, cmsg_level=SOL_IP"
	       ", cmsg_type=%s, cmsg_data={ee_errno=%u, ee_origin=%u"
	       ", ee_type=%u, ee_code=%u, ee_info=%u, ee_data=%u"
	       ", offender={sa_family=AF_INET, sin_port=htons(%hu)"
	       ", sin_addr=inet_addr(\"127.0.0.1\")}}}]"
	       ", msg_controllen=%u, msg_flags=0}, 0) = %d %s (%m)\n",
	       (unsigned) cmsg->cmsg_len, cmsg_type_str,
	       e->ee_errno, e->ee_origin, e->ee_type,
	       e->ee_code, e->ee_info, e->ee_data,
	       ntohs(e->offender.sin_port),
	       len, rc, errno2name());
}
#endif

#ifdef IP_ORIGDSTADDR
static void
test_ip_origdstaddr(struct msghdr *const mh, void *const page,
		    const int cmsg_type, const char *const cmsg_type_str)
{
	const unsigned int len = CMSG_SPACE(sizeof(struct sockaddr_in));
	struct cmsghdr *const cmsg = get_cmsghdr(page, len);

	cmsg->cmsg_len = CMSG_LEN(sizeof(struct sockaddr_in));
	cmsg->cmsg_level = SOL_IP;
	cmsg->cmsg_type = cmsg_type;

	struct sockaddr_in *const sin = (struct sockaddr_in *) CMSG_DATA(cmsg);
	sin->sin_family = AF_INET,
	sin->sin_port = htons(12345),
	sin->sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	mh->msg_control = cmsg;
	mh->msg_controllen = len;

	int rc = sendmsg(-1, mh, 0);
	printf("sendmsg(-1, {msg_name=NULL, msg_namelen=0, msg_iov=NULL"
	       ", msg_iovlen=0, msg_control=[{cmsg_len=%u, cmsg_level=SOL_IP"
	       ", cmsg_type=%s, cmsg_data={sa_family=AF_INET"
	       ", sin_port=htons(%hu), sin_addr=inet_addr(\"127.0.0.1\")}}]"
	       ", msg_controllen=%u, msg_flags=0}, 0) = %d %s (%m)\n",
	       (unsigned) cmsg->cmsg_len, cmsg_type_str,
	       ntohs(sin->sin_port), len, rc, errno2name());
}
#endif

#ifdef IP_PROTOCOL
static void
test_ip_protocol(struct msghdr *const mh, void *const page,
		 const int cmsg_type, const char *const cmsg_type_str)
{
	const unsigned int len = CMSG_SPACE(sizeof(int));
	struct cmsghdr *const cmsg = get_cmsghdr(page, len);

	cmsg->cmsg_len = CMSG_LEN(sizeof(int));
	cmsg->cmsg_level = SOL_IP;
	cmsg->cmsg_type = cmsg_type;

	unsigned int *u = (void *) CMSG_DATA(cmsg);
	*u = IPPROTO_RAW;

	mh->msg_control = cmsg;
	mh->msg_controllen = len;

	int rc = sendmsg(-1, mh, 0);
	printf("sendmsg(-1, {msg_name=NULL, msg_namelen=0, msg_iov=NULL"
	       ", msg_iovlen=0, msg_control=[{cmsg_len=%u"
	       ", cmsg_level=SOL_IP, cmsg_type=%s, cmsg_data=[%s]}]"
	       ", msg_controllen=%u, msg_flags=0}, 0) = %d %s (%m)\n",
	       (unsigned) cmsg->cmsg_len, cmsg_type_str, "IPPROTO_RAW", len,
	       rc, errno2name());
}
#endif

static void
test_sol_ip(struct msghdr *const mh, void *const page)
{
	test_ip_pktinfo(mh, page, ARG_STR(IP_PKTINFO));
	test_ip_uint(mh, page, ARG_STR(IP_TTL));
	test_ip_uint8_t(mh, page, ARG_STR(IP_TOS));
	test_ip_opts(mh, page, ARG_STR(IP_RECVOPTS), 1);
	test_ip_opts(mh, page, ARG_STR(IP_RECVOPTS), 2);
	test_ip_opts(mh, page, ARG_STR(IP_RECVOPTS), 3);
	test_ip_opts(mh, page, ARG_STR(IP_RECVOPTS), 4);
	test_ip_opts(mh, page, ARG_STR(IP_RETOPTS), 5);
	test_ip_opts(mh, page, ARG_STR(IP_RETOPTS), 6);
	test_ip_opts(mh, page, ARG_STR(IP_RETOPTS), 7);
	test_ip_opts(mh, page, ARG_STR(IP_RETOPTS), 8);
	test_ip_opts(mh, page, ARG_STR(IP_RETOPTS), DEFAULT_STRLEN - 1);
	test_ip_opts(mh, page, ARG_STR(IP_RETOPTS), DEFAULT_STRLEN);
	test_ip_opts(mh, page, ARG_STR(IP_RETOPTS), DEFAULT_STRLEN + 1);
#ifdef IP_RECVERR
	test_ip_recverr(mh, page, ARG_STR(IP_RECVERR));
#endif
#ifdef IP_ORIGDSTADDR
	test_ip_origdstaddr(mh, page, ARG_STR(IP_ORIGDSTADDR));
#endif
#ifdef IP_CHECKSUM
	test_ip_uint(mh, page, ARG_STR(IP_CHECKSUM));
#endif
#ifdef IP_PROTOCOL
	test_ip_protocol(mh, page, ARG_STR(IP_PROTOCOL));
#endif
	test_scm_security(mh, CMSG_LEN(0), page, 0, CMSG_LEN(0),
			  ARG_STR(SOL_IP));
	test_unknown_type(mh, page, ARG_STR(SOL_IP), "IP_???");
}

static void
test_unknown_level(struct msghdr *const mh, void *const page)
{
	struct cmsghdr *cmsg = get_cmsghdr(page, CMSG_LEN(0));

	cmsg->cmsg_len = CMSG_LEN(0);
	cmsg->cmsg_level = SOL_TCP;
	cmsg->cmsg_type = 0xdeadbeef;

	mh->msg_control = cmsg;
	mh->msg_controllen = cmsg->cmsg_len;

	int rc = sendmsg(-1, mh, 0);
	printf("sendmsg(-1, {msg_name=NULL, msg_namelen=0, msg_iov=NULL"
	       ", msg_iovlen=0, msg_control=[{cmsg_len=%u, cmsg_level=%s"
	       ", cmsg_type=%#x}], msg_controllen=%u, msg_flags=0}"
	       ", 0) = %d %s (%m)\n",
	       (unsigned) cmsg->cmsg_len, "SOL_TCP", cmsg->cmsg_type,
	       (unsigned) mh->msg_controllen, rc, errno2name());
}

static void
test_big_len(struct msghdr *const mh)
{
	int optmem_max;

	if (read_int_from_file("/proc/sys/net/core/optmem_max", &optmem_max)
	    || optmem_max <= 0 || optmem_max > 0x100000)
		optmem_max = sizeof(long long) * (2 * IOV_MAX + 512);
	optmem_max = (optmem_max + sizeof(long long) - 1)
		     & ~(sizeof(long long) - 1);

	const size_t len = optmem_max * 2;
	struct cmsghdr *const cmsg = tail_alloc(len);
	cmsg->cmsg_len = len;
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;

	mh->msg_control = cmsg;
	mh->msg_controllen = len;

	int rc = sendmsg(-1, mh, 0);
	if (EBADF != errno)
		perror_msg_and_skip("sendmsg");

	printf("sendmsg(-1, {msg_name=NULL, msg_namelen=0, msg_iov=NULL"
	       ", msg_iovlen=0, msg_control=[{cmsg_len=%u"
	       ", cmsg_level=SOL_SOCKET, cmsg_type=SCM_RIGHTS",
	       (unsigned) cmsg->cmsg_len);
	print_fds(cmsg, optmem_max);
	printf("}, ...], msg_controllen=%lu, msg_flags=0}, 0) = %d %s (%m)\n",
	       (unsigned long) len, rc, errno2name());
}

int main(int ac, const char **av)
{
	int rc = sendmsg(-1, 0, 0);
	printf("sendmsg(-1, NULL, 0) = %d %s (%m)\n", rc, errno2name());

	TAIL_ALLOC_OBJECT_CONST_PTR(struct msghdr, mh);
	memset(mh, 0, sizeof(*mh));
	test_big_len(mh);

	rc = sendmsg(-1, mh + 1, 0);
	printf("sendmsg(-1, %p, 0) = %d %s (%m)\n",
	       mh + 1, rc, errno2name());

	void *page = tail_alloc(1024) + 1024;
	mh->msg_control = page;
	mh->msg_controllen = CMSG_LEN(0);
	rc = sendmsg(-1, mh, 0);
	printf("sendmsg(-1, {msg_name=NULL, msg_namelen=0, msg_iov=NULL"
	       ", msg_iovlen=0, msg_control=%p, msg_controllen=%u"
	       ", msg_flags=0}, 0) = %d %s (%m)\n",
	       page, (unsigned) CMSG_LEN(0), rc, errno2name());

	test_sol_socket(mh, page);
	test_sol_ip(mh, page);
	test_unknown_level(mh, page);

	puts("+++ exited with 0 +++");
	return 0;
}
