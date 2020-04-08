/*
 * Check decoding of io_pgetevents syscall.
 *
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <unistd.h>
#include "scno.h"

#if defined __NR_io_setup && defined __NR_io_pgetevents

# include <fcntl.h>
# include <inttypes.h>
# include <stdio.h>
# include <time.h>

# include "nsig.h"

# include <linux/aio_abi.h>

# if !HAVE_STRUCT___AIO_SIGSET
struct __aio_sigset {
	sigset_t *sigmask;
	size_t sigsetsize;
};
# endif

static const char *errstr;

static long
sys_io_pgetevents(const kernel_ulong_t ctx_id,
		  const kernel_long_t min_nr,
		  const kernel_long_t nr,
		  const kernel_ulong_t events,
		  const kernel_ulong_t timeout,
		  const kernel_ulong_t usig)
{
	long rc = syscall(__NR_io_pgetevents, ctx_id, min_nr, nr,
			  events, timeout, usig);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	static const kernel_ulong_t bogus_ctx =
		(kernel_ulong_t) 0xface1e55deadbeefLL;
	static const kernel_long_t bogus_min_nr =
		(kernel_long_t) 0xca7faceddeadf00dLL;
	static const kernel_long_t bogus_nr =
		(kernel_long_t) 0xba5e1e505ca571e0LL;
	static const size_t bogus_sigsetsize =
		(size_t) 0xdeadbeefbadcaffeULL;

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

	const long proto_cbs[] = {
		(long) &cb[0], (long) &cb[1]
	};
	const long *cbs = tail_memdup(proto_cbs, sizeof(proto_cbs));

	TAIL_ALLOC_OBJECT_CONST_PTR(unsigned long, ctx);
	*ctx = 0;

	const unsigned int nr = ARRAY_SIZE(proto_cb);

	const struct io_event *ev = tail_alloc(nr * sizeof(struct io_event));
	TAIL_ALLOC_OBJECT_CONST_PTR(struct timespec, ts);
	TAIL_ALLOC_OBJECT_CONST_PTR(struct __aio_sigset, ss);
	TAIL_ALLOC_OBJECT_CONST_PTR(sigset_t, sigs);

	(void) close(0);
	if (open("/dev/zero", O_RDONLY))
		perror_msg_and_skip("open: %s", "/dev/zero");

	if (syscall(__NR_io_setup, nr, ctx))
		perror_msg_and_skip("io_setup");

	if (syscall(__NR_io_submit, *ctx, nr, cbs) != (long) nr)
		perror_msg_and_skip("io_submit");

	sys_io_pgetevents(bogus_ctx, bogus_min_nr, bogus_nr,
			  (uintptr_t) (ev + 1), 0, 0);
	printf("io_pgetevents(%#jx, %ld, %ld, %p, NULL, NULL) = %s\n",
	       (uintmax_t) bogus_ctx, (long) bogus_min_nr,
	       (long) bogus_nr, ev + 1, errstr);

	sys_io_pgetevents(bogus_ctx, bogus_min_nr, bogus_nr,
			  0, (uintptr_t) (ts + 1), 0);
	printf("io_pgetevents(%#jx, %ld, %ld, NULL, %p, NULL) = %s\n",
	       (uintmax_t) bogus_ctx, (long) bogus_min_nr,
	       (long) bogus_nr, ts + 1, errstr);

	sys_io_pgetevents(bogus_ctx, bogus_min_nr, bogus_nr,
			  0, 0, (uintptr_t) (ss + 1));
	printf("io_pgetevents(%#jx, %ld, %ld, NULL, NULL, %p) = %s\n",
	       (uintmax_t) bogus_ctx, (long) bogus_min_nr,
	       (long) bogus_nr, ss + 1, errstr);

	ss->sigmask = sigs + 1;
	ss->sigsetsize =  bogus_sigsetsize;
	sys_io_pgetevents(bogus_ctx, bogus_min_nr, bogus_nr,
			  0, 0, (uintptr_t) ss);
	printf("io_pgetevents(%#jx, %ld, %ld, NULL, NULL"
	       ", {sigmask=%p, sigsetsize=%zu}) = %s\n",
	       (uintmax_t) bogus_ctx, (long) bogus_min_nr,
	       (long) bogus_nr, sigs + 1, bogus_sigsetsize, errstr);

	ts->tv_sec = 0xdeadbeefU;
	ts->tv_nsec = 0xfacefeedU;
	ss->sigmask = sigs;
	ss->sigsetsize =  NSIG_BYTES;
	sys_io_pgetevents(bogus_ctx, 0, 0, 0, (uintptr_t) ts, (uintptr_t) ss);
	printf("io_pgetevents(%#jx, 0, 0, NULL"
	       ", {tv_sec=%lld, tv_nsec=%llu}"
	       ", {sigmask=~[], sigsetsize=%u}) = %s\n",
	       (uintmax_t) bogus_ctx, (long long) ts->tv_sec,
	       zero_extend_signed_to_ull(ts->tv_nsec), NSIG_BYTES,
	       errstr);

	sigemptyset(sigs);
	sigaddset(sigs, SIGSYS);

	ts->tv_sec = (time_t) 0xcafef00ddeadbeefLL;
	ts->tv_nsec = (long) 0xbadc0dedfacefeedLL;
	sys_io_pgetevents(bogus_ctx, 0, 0, 0, (uintptr_t) ts, (uintptr_t) ss);
	printf("io_pgetevents(%#jx, 0, 0, NULL"
	       ", {tv_sec=%lld, tv_nsec=%llu}"
	       ", {sigmask=[SYS], sigsetsize=%u}) = %s\n",
	       (uintmax_t) bogus_ctx, (long long) ts->tv_sec,
	       zero_extend_signed_to_ull(ts->tv_nsec), NSIG_BYTES,
	       errstr);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_io_setup && __NR_io_pgetevents")

#endif
