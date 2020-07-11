/*
 * This file is part of ioctl_block strace test.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "pidns.h"
#include <errno.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <linux/blkpg.h>
#ifdef HAVE_STRUCT_BLK_USER_TRACE_SETUP
# include <linux/blktrace_api.h>
#endif
#include "xlat.h"

static const unsigned int magic = 0xdeadbeef;
static const unsigned long lmagic = (unsigned long) 0xdeadbeefbadc0dedULL;

static struct xlat_data block_argless[] = {
	XLAT(BLKRRPART),
	XLAT(BLKFLSBUF),
#ifdef BLKTRACESTART
	XLAT(BLKTRACESTART),
#endif
#ifdef BLKTRACESTOP
	XLAT(BLKTRACESTOP),
#endif
#ifdef BLKTRACETEARDOWN
	XLAT(BLKTRACETEARDOWN),
#endif
};

#define TEST_NULL_ARG(cmd)						\
	do {								\
		ioctl(-1, cmd, 0);					\
		pidns_print_leader();					\
		printf("ioctl(-1, %s, NULL) = -1 EBADF (%m)\n", #cmd);	\
	} while (0)

int
main(void)
{
	PIDNS_TEST_INIT;

	TEST_NULL_ARG(BLKBSZGET);
	TEST_NULL_ARG(BLKBSZSET);
	TEST_NULL_ARG(BLKFRAGET);
	TEST_NULL_ARG(BLKGETSIZE);
	TEST_NULL_ARG(BLKGETSIZE64);
	TEST_NULL_ARG(BLKPG);
	TEST_NULL_ARG(BLKRAGET);
	TEST_NULL_ARG(BLKROGET);
	TEST_NULL_ARG(BLKROSET);
	TEST_NULL_ARG(BLKSECTGET);
	TEST_NULL_ARG(BLKSECTGET);
	TEST_NULL_ARG(BLKSSZGET);
#ifdef BLKALIGNOFF
	TEST_NULL_ARG(BLKALIGNOFF);
#endif
#ifdef BLKDISCARD
	TEST_NULL_ARG(BLKDISCARD);
#endif
#ifdef BLKDISCARDZEROES
	TEST_NULL_ARG(BLKDISCARDZEROES);
#endif
#ifdef BLKIOMIN
	TEST_NULL_ARG(BLKIOMIN);
#endif
#ifdef BLKIOOPT
	TEST_NULL_ARG(BLKIOOPT);
#endif
#ifdef BLKPBSZGET
	TEST_NULL_ARG(BLKPBSZGET);
#endif
#ifdef BLKROTATIONAL
	TEST_NULL_ARG(BLKROTATIONAL);
#endif
#ifdef BLKSECDISCARD
	TEST_NULL_ARG(BLKSECDISCARD);
#endif
#ifdef BLKZEROOUT
	TEST_NULL_ARG(BLKZEROOUT);
#endif
#if defined BLKTRACESETUP && defined HAVE_STRUCT_BLK_USER_TRACE_SETUP
	TEST_NULL_ARG(BLKTRACESETUP);
#endif

	ioctl(-1, BLKRASET, lmagic);
	pidns_print_leader();
	printf("ioctl(-1, BLKRASET, %lu) = -1 EBADF (%m)\n", lmagic);

	ioctl(-1, BLKFRASET, lmagic);
	pidns_print_leader();
	printf("ioctl(-1, BLKFRASET, %lu) = -1 EBADF (%m)\n", lmagic);

	TAIL_ALLOC_OBJECT_CONST_PTR(int, val_int);
	*val_int = magic;

	ioctl(-1, BLKROSET, val_int);
	pidns_print_leader();
	printf("ioctl(-1, BLKROSET, [%d]) = -1 EBADF (%m)\n", *val_int);

	ioctl(-1, BLKBSZSET, val_int);
	pidns_print_leader();
	printf("ioctl(-1, BLKBSZSET, [%d]) = -1 EBADF (%m)\n", *val_int);

	uint64_t *pair_int64 = tail_alloc(sizeof(*pair_int64) * 2);
	pair_int64[0] = 0xdeadbeefbadc0dedULL;
	pair_int64[1] = 0xfacefeedcafef00dULL;

#ifdef BLKDISCARD
	ioctl(-1, BLKDISCARD, pair_int64);
	pidns_print_leader();
	printf("ioctl(-1, BLKDISCARD, [%" PRIu64 ", %" PRIu64 "])"
	       " = -1 EBADF (%m)\n", pair_int64[0], pair_int64[1]);
#endif

#ifdef BLKSECDISCARD
	ioctl(-1, BLKSECDISCARD, pair_int64);
	pidns_print_leader();
	printf("ioctl(-1, BLKSECDISCARD, [%" PRIu64 ", %" PRIu64 "])"
	       " = -1 EBADF (%m)\n", pair_int64[0], pair_int64[1]);
#endif

#ifdef BLKZEROOUT
	ioctl(-1, BLKZEROOUT, pair_int64);
	pidns_print_leader();
	printf("ioctl(-1, BLKZEROOUT, [%" PRIu64 ", %" PRIu64 "])"
	       " = -1 EBADF (%m)\n", pair_int64[0], pair_int64[1]);
#endif

	TAIL_ALLOC_OBJECT_CONST_PTR(struct blkpg_ioctl_arg, blkpg);
	blkpg->op = 3;
	blkpg->flags = 0xdeadbeef;
	blkpg->datalen = 0xbadc0ded;
	blkpg->data = (void *) (unsigned long) 0xcafef00dfffffeedULL;

	ioctl(-1, BLKPG, blkpg);
	pidns_print_leader();
	printf("ioctl(-1, BLKPG, {op=%s, flags=%d, datalen=%d"
	       ", data=%#lx}) = -1 EBADF (%m)\n",
	       "BLKPG_RESIZE_PARTITION", blkpg->flags, blkpg->datalen,
	       (unsigned long) blkpg->data);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct blkpg_partition, bp);
	bp->start = 0xfac1fed2dad3bef4ULL;
	bp->length = 0xfac5fed6dad7bef8ULL;
	bp->pno = magic;
	memset(bp->devname, 'A', sizeof(bp->devname));
	memset(bp->volname, 'B', sizeof(bp->volname));
	blkpg->op = 1;
	blkpg->data = bp;

	ioctl(-1, BLKPG, blkpg);
	pidns_print_leader();
	printf("ioctl(-1, BLKPG, {op=%s, flags=%d, datalen=%d"
	       ", data={start=%lld, length=%lld, pno=%d"
	       ", devname=\"%.*s\"..., volname=\"%.*s\"...}})"
	       " = -1 EBADF (%m)\n",
	       "BLKPG_ADD_PARTITION",
	       blkpg->flags, blkpg->datalen,
	       bp->start, bp->length, bp->pno,
	       (int) sizeof(bp->devname) - 1, bp->devname,
	       (int) sizeof(bp->volname) - 1, bp->volname);

#if defined BLKTRACESETUP && defined HAVE_STRUCT_BLK_USER_TRACE_SETUP
	TAIL_ALLOC_OBJECT_CONST_PTR(struct blk_user_trace_setup, buts);
	fill_memory(buts, sizeof(*buts));
	buts->pid = getpid();

	ioctl(-1, BLKTRACESETUP, buts);
	pidns_print_leader();
	printf("ioctl(-1, BLKTRACESETUP, {act_mask=%hu, buf_size=%u, buf_nr=%u"
	       ", start_lba=%" PRI__u64 ", end_lba=%" PRI__u64 ", pid=%d%s})"
	       " = -1 EBADF (%m)\n",
	       buts->act_mask, buts->buf_size, buts->buf_nr,
	       buts->start_lba, buts->end_lba, buts->pid,
	       pidns_pid2str(PT_TGID));
#endif

	unsigned int i;
	for (i = 0; i < ARRAY_SIZE(block_argless); ++i) {
		ioctl(-1, (unsigned long) block_argless[i].val, lmagic);
		pidns_print_leader();
		printf("ioctl(-1, %s) = -1 EBADF (%m)\n", block_argless[i].str);
	}

	ioctl(-1, _IOC(_IOC_READ, 0x12, 0xfe, 0xff), lmagic);
	pidns_print_leader();
	printf("ioctl(-1, %s, %#lx) = -1 EBADF (%m)\n",
	       "_IOC(_IOC_READ, 0x12, 0xfe, 0xff)", lmagic);

	pidns_print_leader();
	puts("+++ exited with 0 +++");
	return 0;
}
