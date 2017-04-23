/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
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
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <asm/unistd.h>

#if defined __NR_io_setup \
 && defined __NR_io_submit \
 && defined __NR_io_getevents \
 && defined __NR_io_cancel \
 && defined __NR_io_destroy
# include <linux/aio_abi.h>

int
main(void)
{
	static const long bogus_ctx =
		(long) 0xface1e55deadbeefLL;

	static const char data2[] =
		"\0\1\2\3cat test test test 0123456789abcdef";

	const unsigned int sizeof_data0 = 4096;
	const unsigned int sizeof_data1 = 8192;
	void *data0 = tail_alloc(sizeof_data0);
	void *data1 = tail_alloc(sizeof_data1);

	const struct iocb proto_cb[] = {
		{
			.aio_data = (unsigned long) 0xfeedface11111111ULL,
			.aio_reqprio = 11,
			.aio_buf = (unsigned long) data0,
			.aio_offset = (unsigned long) 0xdeface1facefeedULL,
			.aio_nbytes = sizeof_data0
		},
		{
			.aio_data = (unsigned long) 0xfeedface22222222ULL,
			.aio_reqprio = 22,
			.aio_buf = (unsigned long) data1,
			.aio_offset = (unsigned long) 0xdeface2cafef00dULL,
			.aio_nbytes = sizeof_data1
		}
	};
	const struct iocb *cb = tail_memdup(proto_cb, sizeof(proto_cb));

	const struct iovec proto_iov0[] = {
		{
			.iov_base = data0,
			.iov_len = sizeof_data0 / 4
		},
		{
			.iov_base = data0 + sizeof_data0 / 4,
			.iov_len = sizeof_data0 / 4 * 3
		},
	};
	const struct iovec *iov0 = tail_memdup(proto_iov0, sizeof(proto_iov0));

	const struct iovec proto_iov1[] = {
		{
			.iov_base = data1,
			.iov_len = sizeof_data1 / 4
		},
		{
			.iov_base = data1 + sizeof_data1 / 4,
			.iov_len = sizeof_data1 / 4 * 3
		},
	};
	const struct iovec *iov1 = tail_memdup(proto_iov1, sizeof(proto_iov1));

	const struct iocb proto_cbv[] = {
		{
			.aio_data = (unsigned long) 0xfeed11111111faceULL,
			.aio_lio_opcode = 7,
			.aio_reqprio = 111,
			.aio_buf = (unsigned long) iov0,
			.aio_offset = (unsigned long) 0xdeface1facefeedULL,
			.aio_nbytes = ARRAY_SIZE(proto_iov0)
		},
		{
			.aio_data = (unsigned long) 0xfeed22222222faceULL,
			.aio_lio_opcode = 7,
			.aio_reqprio = 222,
			.aio_buf = (unsigned long) iov1,
			.aio_offset = (unsigned long) 0xdeface2cafef00dULL,
			.aio_nbytes = ARRAY_SIZE(proto_iov1)
		}
	};
	const struct iocb *cbv = tail_memdup(proto_cbv, sizeof(proto_cbv));

	/* For additional decoder testing */
	const struct iocb proto_cbv2[] = {
		{
			.aio_data = 0xbadfacedc0ffeeedULL,
			.aio_key = 0xdefaced0,
			.aio_lio_opcode = 0xf00d,
			.aio_reqprio = 0,
			.aio_fildes = 0xdefaced1,
			.aio_buf = 0,
		},
		{
			.aio_data = 0,
			.aio_key = 0xdefaced0,
			.aio_lio_opcode = 1,
			.aio_reqprio = 0xbeef,
			.aio_fildes = 0xdefaced1,
			.aio_buf = 0,
			/* In order to make record valid */
			.aio_nbytes = (size_t) 0x1020304050607080ULL,
			.aio_offset = 0xdeadda7abadc0dedULL,
# ifdef IOCB_FLAG_RESFD
			.aio_flags = 0xfacef157,
			.aio_resfd = 0xded1ca7e,
# endif
		},
		{
			.aio_data = 0,
			.aio_key = 0xdefaced0,
			.aio_lio_opcode = 1,
			.aio_reqprio = 0xbeef,
			.aio_fildes = 0xdefaced1,
			.aio_buf = 0xbadc0ffeedefacedULL,
			.aio_nbytes = 0x8090a0b0c0d0e0f0ULL,
			.aio_offset = 0xdeadda7abadc0dedULL,
		},
		{
			.aio_data = 0,
			.aio_key = 0xdefaced0,
			.aio_lio_opcode = 1,
			.aio_reqprio = 0xbeef,
			.aio_fildes = 0xdefaced1,
			.aio_buf = (unsigned long)data2,
			.aio_nbytes = sizeof(data2),
			.aio_offset = 0xdeadda7abadc0dedULL,
		},
		{
			.aio_data = 0,
			.aio_key = 0xdefaced0,
			.aio_lio_opcode = 8,
			.aio_reqprio = 0xbeef,
			.aio_fildes = 0xdefaced1,
			.aio_buf = 0,
			.aio_nbytes = 0x8090a0b0c0d0e0f0ULL,
			.aio_offset = 0xdeadda7abadc0dedULL,
		},
	};
	const struct iocb *cbv2 = tail_memdup(proto_cbv2, sizeof(proto_cbv2));

	const struct iocb proto_cbc = {
		.aio_data = (unsigned long) 0xdeadbeefbadc0dedULL,
		.aio_reqprio = 99,
		.aio_fildes = -42
	};
	const struct iocb *cbc = tail_memdup(&proto_cbc, sizeof(proto_cbc));

	const long proto_cbs[] = {
		(long) &cb[0], (long) &cb[1]
	};
	const long *cbs = tail_memdup(proto_cbs, sizeof(proto_cbs));

	const long proto_cbvs[] = {
		(long) &cbv[0], (long) &cbv[1],
	};
	const long *cbvs = tail_memdup(proto_cbvs, sizeof(proto_cbvs));

	const long proto_cbvs2[] = {
		(long) &cbv2[0], (long) &cbv2[1], (long) &cbv2[2],
		(long) &cbv2[3], (long) &cbv2[4],
		(long) NULL, (long) 0xffffffffffffffffLL,
	};
	const long *cbvs2 = tail_memdup(proto_cbvs2, sizeof(proto_cbvs2));

	TAIL_ALLOC_OBJECT_CONST_PTR(unsigned long, ctx);
	*ctx = 0;

	const unsigned int nr = ARRAY_SIZE(proto_cb);
	const unsigned long lnr = (unsigned long) (0xdeadbeef00000000ULL | nr);

	const struct io_event *ev = tail_alloc(nr * sizeof(struct io_event));
	TAIL_ALLOC_OBJECT_CONST_PTR(struct timespec, ts);

	(void) close(0);
	if (open("/dev/zero", O_RDONLY))
		perror_msg_and_skip("open: %s", "/dev/zero");

	long rc = syscall(__NR_io_setup, 0xdeadbeef, NULL);
	printf("io_setup(%u, NULL) = %s\n", 0xdeadbeef, sprintrc(rc));

	rc = syscall(__NR_io_setup, lnr, ctx + 1);
	printf("io_setup(%u, %p) = %s\n", nr, ctx + 1, sprintrc(rc));

	if (syscall(__NR_io_setup, lnr, ctx))
		perror_msg_and_skip("io_setup");
	printf("io_setup(%u, [%#lx]) = 0\n", nr, *ctx);

	rc = syscall(__NR_io_submit, bogus_ctx, (long) 0xca7faceddeadf00dLL,
		     NULL);
	printf("io_submit(%#lx, %ld, NULL) = %s\n",
	       bogus_ctx, (long) 0xca7faceddeadf00dLL, sprintrc(rc));

	rc = syscall(__NR_io_submit, *ctx, nr, cbs + nr);
	printf("io_submit(%#lx, %ld, %p) = %s\n",
	       *ctx, (long) nr, cbs + nr, sprintrc(rc));

	rc = syscall(__NR_io_submit, *ctx, -1L, cbs);
	printf("io_submit(%#lx, -1, %p) = %s\n",
	       *ctx, cbs, sprintrc(rc));

	rc = syscall(__NR_io_submit, *ctx, nr, cbs);
	if (rc != (long) nr)
		perror_msg_and_skip("io_submit");
	printf("io_submit(%#lx, %u, ["
	       "{data=%#" PRI__x64 ", pread, reqprio=11, fildes=0, "
	               "buf=%p, nbytes=%u, offset=%" PRI__d64 "}, "
	       "{data=%#" PRI__x64 ", pread, reqprio=22, fildes=0, "
	               "buf=%p, nbytes=%u, offset=%" PRI__d64 "}"
	       "]) = %s\n",
	       *ctx, nr,
	       cb[0].aio_data, data0, sizeof_data0, cb[0].aio_offset,
	       cb[1].aio_data, data1, sizeof_data1, cb[1].aio_offset,
	       sprintrc(rc));

	rc = syscall(__NR_io_getevents, bogus_ctx,
		     (long) 0xca7faceddeadf00dLL, (long) 0xba5e1e505ca571e0LL,
		     ev + 1, NULL);
	printf("io_getevents(%#lx, %ld, %ld, %p, NULL) = %s\n",
	       bogus_ctx, (long) 0xca7faceddeadf00dLL,
	       (long) 0xba5e1e505ca571e0LL, ev + 1, sprintrc(rc));

	rc = syscall(__NR_io_getevents, bogus_ctx,
		     (long) 0xca7faceddeadf00dLL, (long) 0xba5e1e505ca571e0LL,
		     NULL, ts + 1);
	printf("io_getevents(%#lx, %ld, %ld, NULL, %p) = %s\n",
	       bogus_ctx, (long) 0xca7faceddeadf00dLL,
	       (long) 0xba5e1e505ca571e0LL, ts + 1, sprintrc(rc));

	ts->tv_sec = 0xdeadbeefU;
	ts->tv_nsec = 0xfacefeedU;
	rc = syscall(__NR_io_getevents, bogus_ctx, 0, 0, 0, ts);
	printf("io_getevents(%#lx, 0, 0, NULL"
	       ", {tv_sec=%lld, tv_nsec=%llu}) = %s\n",
	       bogus_ctx, (long long) ts->tv_sec,
	       zero_extend_signed_to_ull(ts->tv_nsec), sprintrc(rc));

	ts->tv_sec = (time_t) 0xcafef00ddeadbeefLL;
	ts->tv_nsec = (long) 0xbadc0dedfacefeedLL;
	rc = syscall(__NR_io_getevents, bogus_ctx, 0, 0, 0, ts);
	printf("io_getevents(%#lx, 0, 0, NULL"
	       ", {tv_sec=%lld, tv_nsec=%llu}) = %s\n",
	       bogus_ctx, (long long) ts->tv_sec,
	       zero_extend_signed_to_ull(ts->tv_nsec), sprintrc(rc));

	ts->tv_sec = 0;
	ts->tv_nsec = 123456789;
	rc = syscall(__NR_io_getevents, *ctx, nr, nr + 1, ev, ts);
	printf("io_getevents(%#lx, %ld, %ld, ["
	       "{data=%#" PRI__x64 ", obj=%p, res=%u, res2=0}, "
	       "{data=%#" PRI__x64 ", obj=%p, res=%u, res2=0}"
	       "], {tv_sec=0, tv_nsec=123456789}) = %s\n",
	       *ctx, (long) nr, (long) (nr + 1),
	       cb[0].aio_data, &cb[0], sizeof_data0,
	       cb[1].aio_data, &cb[1], sizeof_data1,
	       sprintrc(rc));

	rc = syscall(__NR_io_cancel, bogus_ctx, NULL, NULL);
	printf("io_cancel(%#lx, NULL, NULL) = %s\n", bogus_ctx, sprintrc(rc));

	rc = syscall(__NR_io_cancel, *ctx, cbc + 1, ev);
	printf("io_cancel(%#lx, %p, %p) = %s\n", *ctx, cbc + 1, ev,
	       sprintrc(rc));

	rc = syscall(__NR_io_cancel, *ctx, cbc, ev);
	printf("io_cancel(%#lx, {data=%#" PRI__x64
	       ", pread, reqprio=99, fildes=-42}, %p) = %s\n",
	       *ctx, cbc->aio_data, ev, sprintrc(rc));

	rc = syscall(__NR_io_submit, (unsigned long) 0xfacef157beeff00dULL,
		     (long) 0xdeadc0defacefeedLL, NULL);
	printf("io_submit(%#lx, %ld, NULL) = %s\n",
	       (long) 0xfacef157beeff00dULL,
	       (long) 0xdeadc0defacefeedLL, sprintrc(rc));

	rc = syscall(__NR_io_submit, *ctx, -1L, cbvs + nr);
	printf("io_submit(%#lx, %ld, %p) = %s\n",
	       *ctx, -1L, cbvs + nr, sprintrc(rc));

	rc = syscall(__NR_io_submit, *ctx, 1057L, cbvs2);
	printf("io_submit(%#lx, %ld, ["
	       "{data=%#" PRI__x64 ", key=%u, %hu /* SUB_??? */, fildes=%d}, "
	       "{key=%u, pwrite, reqprio=%hd, fildes=%d, str=NULL"
	               ", nbytes=%" PRI__u64 ", offset=%" PRI__d64
# ifdef IOCB_FLAG_RESFD
	               ", resfd=%d, flags=%#x"
# endif
	               "}, "
	       "{key=%u, pwrite, reqprio=%hd, fildes=%d, buf=%#" PRI__x64
	               ", nbytes=%" PRI__u64 ", offset=%" PRI__d64 "}, "
	       "{key=%u, pwrite, reqprio=%hd, fildes=%d"
	               ", str=\"\\0\\1\\2\\3%.28s\"..."
	               ", nbytes=%" PRI__u64 ", offset=%" PRI__d64 "}, "
	       "{key=%u, pwritev, reqprio=%hd, fildes=%d, buf=%#" PRI__x64
	               ", nbytes=%" PRI__u64 ", offset=%" PRI__d64 "}"
	       ", {NULL}, {%#lx}, %p]) = %s\n",
	       *ctx, 1057L,
	       cbv2[0].aio_data, cbv2[0].aio_key,
	       cbv2[0].aio_lio_opcode, cbv2[0].aio_fildes,
	       cbv2[1].aio_key, cbv2[1].aio_reqprio, cbv2[1].aio_fildes,
	       cbv2[1].aio_nbytes, cbv2[1].aio_offset,
# ifdef IOCB_FLAG_RESFD
	       cbv2[1].aio_resfd, cbv2[1].aio_flags,
# endif
	       cbv2[2].aio_key, cbv2[2].aio_reqprio, cbv2[2].aio_fildes,
	       cbv2[2].aio_buf, cbv2[2].aio_nbytes, cbv2[2].aio_offset,
	       cbv2[3].aio_key, cbv2[3].aio_reqprio, cbv2[3].aio_fildes,
	       data2 + 4, cbv2[3].aio_nbytes, cbv2[3].aio_offset,
	       cbv2[4].aio_key, cbv2[4].aio_reqprio, cbv2[4].aio_fildes,
	       cbv2[4].aio_buf, cbv2[4].aio_nbytes, cbv2[4].aio_offset,
	       cbvs2[6], cbvs2 + 7, sprintrc(rc));

	rc = syscall(__NR_io_submit, *ctx, nr, cbvs);
	if (rc != (long) nr)
		perror_msg_and_skip("io_submit");
	printf("io_submit(%#lx, %u, ["
	       "{data=%#" PRI__x64 ", preadv, reqprio=%hd, fildes=0, "
	               "iovec=[{iov_base=%p, iov_len=%u}"
	               ", {iov_base=%p, iov_len=%u}], offset=%" PRI__d64 "}, "
	       "{data=%#" PRI__x64 ", preadv, reqprio=%hd, fildes=0, "
	               "iovec=[{iov_base=%p, iov_len=%u}"
	               ", {iov_base=%p, iov_len=%u}], offset=%" PRI__d64 "}"
	       "]) = %s\n",
	       *ctx, nr,
	       cbv[0].aio_data, cbv[0].aio_reqprio,
	       iov0[0].iov_base, (unsigned int) iov0[0].iov_len,
	       iov0[1].iov_base, (unsigned int) iov0[1].iov_len,
	       cbv[0].aio_offset,
	       cbv[1].aio_data, cbv[1].aio_reqprio,
	       iov1[0].iov_base, (unsigned int) iov1[0].iov_len,
	       iov1[1].iov_base, (unsigned int) iov1[1].iov_len,
	       cbv[1].aio_offset,
	       sprintrc(rc));

	rc = syscall(__NR_io_destroy, bogus_ctx);
	printf("io_destroy(%#lx) = %s\n",
	       bogus_ctx, sprintrc(rc));

	rc = syscall(__NR_io_destroy, *ctx);
	printf("io_destroy(%#lx) = %s\n", *ctx, sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_io_*")

#endif
