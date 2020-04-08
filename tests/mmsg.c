/*
 * Copyright (c) 2014 Masatake YAMATO <yamato@redhat.com>
 * Copyright (c) 2014-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2014-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <assert.h>
#include <unistd.h>

#include "msghdr.h"

int
main(void)
{
	tprintf("%s", "");

	int fds[2];
	if (socketpair(AF_UNIX, SOCK_DGRAM, 0, fds))
		perror_msg_and_skip("socketpair");
	assert(0 == fds[0]);
	assert(1 == fds[1]);

	static const char w0_c[] = "012";
	const char *w0_d = hexdump_strdup(w0_c);
	void *w0 = tail_memdup(w0_c, LENGTH_OF(w0_c));

	static const char w1_c[] = "34567";
	const char *w1_d = hexdump_strdup(w1_c);
	void *w1 = tail_memdup(w1_c, LENGTH_OF(w1_c));

	static const char w2_c[] = "89abcde";
	const char *w2_d = hexdump_strdup(w2_c);
	void *w2 = tail_memdup(w2_c, LENGTH_OF(w2_c));

	const struct iovec w0_iov_[] = {
		{
			.iov_base = w0,
			.iov_len = LENGTH_OF(w0_c)
		}, {
			.iov_base = w1,
			.iov_len = LENGTH_OF(w1_c)
		}
	};
	struct iovec *w0_iov = tail_memdup(w0_iov_, sizeof(w0_iov_));

	const struct iovec w1_iov_[] = {
		{
			.iov_base = w2,
			.iov_len = LENGTH_OF(w2_c)
		}
	};
	struct iovec *w1_iov = tail_memdup(w1_iov_, sizeof(w1_iov_));

	const struct mmsghdr w_mmh_[] = {
		{
			.msg_hdr = {
				.msg_iov = w0_iov,
				.msg_iovlen = ARRAY_SIZE(w0_iov_),
			}
		}, {
			.msg_hdr = {
				.msg_iov = w1_iov,
				.msg_iovlen = ARRAY_SIZE(w1_iov_),
			}
		}
	};
	void *w_mmh = tail_memdup(w_mmh_, sizeof(w_mmh_));
	const unsigned int n_w_mmh = ARRAY_SIZE(w_mmh_);

	int r = send_mmsg(1, w_mmh, n_w_mmh, MSG_DONTROUTE | MSG_NOSIGNAL);
	if (r < 0)
		perror_msg_and_skip("sendmmsg");
	assert(r == (int) n_w_mmh);
	assert(close(1) == 0);
	tprintf("sendmmsg(1, [{msg_hdr={msg_name=NULL, msg_namelen=0"
		", msg_iov=[{iov_base=\"%s\", iov_len=%u}"
		", {iov_base=\"%s\", iov_len=%u}], msg_iovlen=%u"
		", msg_controllen=0, msg_flags=0}, msg_len=%u}"
		", {msg_hdr={msg_name=NULL, msg_namelen=0"
		", msg_iov=[{iov_base=\"%s\", iov_len=%u}], msg_iovlen=%u"
		", msg_controllen=0, msg_flags=0}, msg_len=%u}], %u"
		", MSG_DONTROUTE|MSG_NOSIGNAL) = %d\n"
		" = %u buffers in vector 0\n"
		" * %u bytes in buffer 0\n"
		" | 00000 %-49s  %-16s |\n"
		" * %u bytes in buffer 1\n"
		" | 00000 %-49s  %-16s |\n"
		" = %u buffers in vector 1\n"
		" * %u bytes in buffer 0\n"
		" | 00000 %-49s  %-16s |\n",
		w0_c, LENGTH_OF(w0_c),
		w1_c, LENGTH_OF(w1_c),
		(unsigned int) ARRAY_SIZE(w0_iov_),
		LENGTH_OF(w0_c) + LENGTH_OF(w1_c),
		w2_c, LENGTH_OF(w2_c), (unsigned int) ARRAY_SIZE(w1_iov_),
		LENGTH_OF(w2_c),
		n_w_mmh, r,
		(unsigned int) ARRAY_SIZE(w0_iov_),
		LENGTH_OF(w0_c), w0_d, w0_c,
		LENGTH_OF(w1_c), w1_d, w1_c,
		(unsigned int) ARRAY_SIZE(w1_iov_),
		LENGTH_OF(w2_c), w2_d, w2_c);

	const unsigned int w_len =
		LENGTH_OF(w0_c) + LENGTH_OF(w1_c) + LENGTH_OF(w2_c);
	const unsigned int r_len = (w_len + 1) / 2;
	void *r0 = tail_alloc(r_len);
	void *r1 = tail_alloc(r_len);
	void *r2 = tail_alloc(r_len);
	const struct iovec r0_iov_[] = {
		{
			.iov_base = r0,
			.iov_len = r_len
		}
	};
	struct iovec *r0_iov = tail_memdup(r0_iov_, sizeof(r0_iov_));
	const struct iovec r1_iov_[] = {
		{
			.iov_base = r1,
			.iov_len = r_len
		},
		{
			.iov_base = r2,
			.iov_len = r_len
		}
	};
	struct iovec *r1_iov = tail_memdup(r1_iov_, sizeof(r1_iov_));

	const struct mmsghdr r_mmh_[] = {
		{
			.msg_hdr = {
				.msg_iov = r0_iov,
				.msg_iovlen = ARRAY_SIZE(r0_iov_),
			}
		}, {
			.msg_hdr = {
				.msg_iov = r1_iov,
				.msg_iovlen = ARRAY_SIZE(r1_iov_),
			}
		}
	};
	void *r_mmh = tail_memdup(r_mmh_, sizeof(r_mmh_));
	const unsigned int n_r_mmh = ARRAY_SIZE(r_mmh_);

	static const char r0_c[] = "01234567";
	const char *r0_d = hexdump_strdup(r0_c);
	static const char r1_c[] = "89abcde";
	const char *r1_d = hexdump_strdup(r1_c);

	assert(recv_mmsg(0, r_mmh, n_r_mmh, MSG_DONTWAIT, NULL) == (int) n_r_mmh);
	assert(close(0) == 0);
	tprintf("recvmmsg(0, [{msg_hdr={msg_name=NULL, msg_namelen=0"
		", msg_iov=[{iov_base=\"%s\", iov_len=%u}], msg_iovlen=%u"
		", msg_controllen=0, msg_flags=0}, msg_len=%u}"
		", {msg_hdr={msg_name=NULL, msg_namelen=0"
		", msg_iov=[{iov_base=\"%s\", iov_len=%u}"
		", {iov_base=\"\", iov_len=%u}], msg_iovlen=%u"
		", msg_controllen=0, msg_flags=0}, msg_len=%u}], %u"
		", MSG_DONTWAIT, NULL) = %d\n"
		" = %u buffers in vector 0\n"
		" * %u bytes in buffer 0\n"
		" | 00000 %-49s  %-16s |\n"
		" = %u buffers in vector 1\n"
		" * %u bytes in buffer 0\n"
		" | 00000 %-49s  %-16s |\n",
		r0_c, r_len, (unsigned int) ARRAY_SIZE(r0_iov_),
		LENGTH_OF(r0_c), r1_c, r_len, r_len,
		(unsigned int) ARRAY_SIZE(r1_iov_), LENGTH_OF(r1_c),
		n_r_mmh, r,
		(unsigned int) ARRAY_SIZE(r0_iov_), LENGTH_OF(r0_c),
		r0_d, r0_c,
		(unsigned int) ARRAY_SIZE(r1_iov_), LENGTH_OF(r1_c),
		r1_d, r1_c);

	tprintf("+++ exited with 0 +++\n");
	return 0;
}
