/*
 * Check decoding of MTD ioctl commands.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <linux/version.h>
#include <mtd/mtd-abi.h>

static const char *errstr;

static int
do_ioctl(kernel_ulong_t cmd, kernel_ulong_t arg)
{
	int rc = ioctl(-1, cmd, arg);
	errstr = sprintrc(rc);

#ifdef INJECT_RETVAL
	if (rc != INJECT_RETVAL)
		error_msg_and_fail("Return value [%d] does not match"
				   " expectations [%d]", rc, INJECT_RETVAL);

	static char inj_errstr[4096];

	snprintf(inj_errstr, sizeof(inj_errstr), "%s (INJECTED)", errstr);
	errstr = inj_errstr;
#endif

	return rc;
}

static int
do_ioctl_ptr(kernel_ulong_t cmd, const void *arg)
{
	return do_ioctl(cmd, (uintptr_t) arg);
}

#ifdef INJECT_RETVAL
static void
skip_ioctls(int argc, const char *argv[])
{
	if (argc < 2)
		error_msg_and_fail("Usage: %s NUM_SKIP", argv[0]);

	unsigned long num_skip = strtoul(argv[1], NULL, 0);

	for (size_t i = 0; i < num_skip; ++i) {
		int rc = ioctl(-1, MEMWRITE, 0);

		printf("ioctl(-1, MEMWRITE, NULL) = %s%s\n", sprintrc(rc),
		       rc == INJECT_RETVAL ? " (INJECTED)" : "");

		if (rc == INJECT_RETVAL)
			return;
	}

	error_msg_and_fail("Issued %lu ioctl syscalls but failed"
			   " to detect an injected return code %d",
			   num_skip, INJECT_RETVAL);
}
#endif /* INJECT_RETVAL */

int
main(int argc, const char *argv[])
{
#ifdef INJECT_RETVAL
	skip_ioctls(argc, argv);
#endif

	static const struct {
		uint32_t cmd;
		const char *str;
	} ptr_cmds[] = {
		{ ARG_STR(ECCGETLAYOUT) },
		{ ARG_STR(ECCGETSTATS) },
		{ ARG_STR(MEMERASE) },
		{ ARG_STR(MEMERASE64) },
		{ ARG_STR(MEMGETBADBLOCK) },
		{ ARG_STR(MEMGETINFO) },
		{ ARG_STR(MEMGETOOBSEL) },
		{ ARG_STR(MEMGETREGIONCOUNT) },
		{ ARG_STR(MEMISLOCKED) },
		{ ARG_STR(MEMLOCK) },
		{ ARG_STR(MEMREADOOB) },
		{ ARG_STR(MEMREADOOB64) },
		{ ARG_STR(MEMSETBADBLOCK) },
		{ ARG_STR(MEMUNLOCK) },
		{ ARG_STR(MEMWRITE) },
		{ ARG_STR(MEMWRITEOOB) },
		{ ARG_STR(MEMWRITEOOB64) },
		{ ARG_STR(OTPGETREGIONCOUNT) },
		{ ARG_STR(OTPGETREGIONINFO) },
		{ ARG_STR(OTPLOCK) },
		{ ARG_STR(OTPSELECT) },
	},
	eiu_cmds[] = {
		{ ARG_STR(MEMERASE) },
		{ ARG_STR(MEMLOCK) },
		{ ARG_STR(MEMUNLOCK) },
		{ ARG_STR(MEMISLOCKED) },
	};

	for (size_t i = 0; i < ARRAY_SIZE(ptr_cmds); ++i) {
		do_ioctl(ptr_cmds[i].cmd, 0);
		if (_IOC_DIR(ptr_cmds[i].cmd) == _IOC_WRITE)
			printf("ioctl(-1, MIXER_WRITE(%u) or %s, NULL) = %s\n",
			       (unsigned int) _IOC_NR(ptr_cmds[i].cmd),
			       ptr_cmds[i].str, errstr);
		else if (_IOC_DIR(ptr_cmds[i].cmd) == _IOC_READ)
			printf("ioctl(-1, MIXER_READ(%u) or %s, NULL) = %s\n",
			       (unsigned int) _IOC_NR(ptr_cmds[i].cmd),
			       ptr_cmds[i].str, errstr);
		else
			printf("ioctl(-1, %s, NULL) = %s\n",
			       ptr_cmds[i].str, errstr);
	}

	do_ioctl(MTDFILEMODE, MTD_FILE_MODE_NORMAL);
	printf("ioctl(-1, MTDFILEMODE, MTD_FILE_MODE_NORMAL) = %s\n", errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(int, opt);
	*opt = MTD_OTP_OFF;
	do_ioctl_ptr(OTPSELECT, opt);
	printf("ioctl(-1, MIXER_READ(%u) or OTPSELECT, [MTD_OTP_OFF]) = %s\n",
	       (unsigned int) _IOC_NR(OTPSELECT), errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(uint64_t, v64);
	fill_memory(v64, sizeof(*v64));

	do_ioctl_ptr(MEMGETBADBLOCK, v64);
	printf("ioctl(-1, MIXER_WRITE(%u) or MEMGETBADBLOCK, [%" PRIu64 "])"
	       " = %s\n",
	       (unsigned int) _IOC_NR(MEMGETBADBLOCK), *v64, errstr);

	do_ioctl_ptr(MEMSETBADBLOCK, v64);
	printf("ioctl(-1, MIXER_WRITE(%u) or MEMSETBADBLOCK, [%" PRIu64 "])"
	       " = %s\n",
	       (unsigned int) _IOC_NR(MEMSETBADBLOCK), *v64, errstr);

	if (do_ioctl(MEMGETREGIONINFO, 0) < 0) {
		printf("ioctl(-1, %s, NULL) = %s\n",
		       "MEMGETREGIONINFO"
#ifdef __i386__
		       " or MTRRIOC_GET_PAGE_ENTRY"
#endif
		       , errstr);
	} else {
		printf("ioctl(-1, %s, NULL) = %s\n",
		       "MEMGETREGIONINFO"
#ifdef __i386__
		       " or MTRRIOC_GET_PAGE_ENTRY"
#endif
		       , errstr);
	}

	TAIL_ALLOC_OBJECT_CONST_PTR(struct region_info_user, riu);
	fill_memory(riu, sizeof(*riu));
	if (do_ioctl_ptr(MEMGETREGIONINFO, riu) < 0) {
		printf("ioctl(-1, %s, {regionindex=%#x}) = %s\n",
		       "MEMGETREGIONINFO"
#ifdef __i386__
		       " or MTRRIOC_GET_PAGE_ENTRY"
#endif
		       , riu->regionindex, errstr);
	} else {
		printf("ioctl(-1, %s, {regionindex=%#x, offset=%#x"
		       ", erasesize=%#x, numblocks=%#x}) = %s\n",
		       "MEMGETREGIONINFO"
#ifdef __i386__
		       " or MTRRIOC_GET_PAGE_ENTRY"
#endif
		       , riu->regionindex, riu->offset,
		       riu->erasesize, riu->numblocks, errstr);
	}

	TAIL_ALLOC_OBJECT_CONST_PTR(struct erase_info_user, eiu);
	fill_memory(eiu, sizeof(*eiu));

	for (size_t i = 0; i < ARRAY_SIZE(eiu_cmds); ++i) {
		do_ioctl_ptr(eiu_cmds[i].cmd, eiu);
		printf("ioctl(-1, MIXER_%s(%u) or %s"
		       ", {start=%#x, length=%#x}) = %s\n",
		       (_IOC_DIR(eiu_cmds[i].cmd) == _IOC_READ)
		        ? "READ" : "WRITE",
		       (unsigned int) _IOC_NR(eiu_cmds[i].cmd),
		       eiu_cmds[i].str, eiu->start, eiu->length, errstr);
	}

	TAIL_ALLOC_OBJECT_CONST_PTR(struct erase_info_user64, eiu64);
	fill_memory(eiu64, sizeof(*eiu64));
	do_ioctl_ptr(MEMERASE64, eiu64);
	printf("ioctl(-1, MIXER_WRITE(%u) or %s, {start=%#llx, length=%#llx})"
	       " = %s\n",
	       (unsigned int) _IOC_NR(MEMERASE64), "MEMERASE64",
	       (unsigned long long) eiu64->start,
	       (unsigned long long) eiu64->length, errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct mtd_oob_buf, oob);
	fill_memory(oob, sizeof(*oob));

	do_ioctl_ptr(MEMWRITEOOB, oob);
	printf("ioctl(-1, MEMWRITEOOB, {start=%#x, length=%#x, ptr=%p})"
	       " = %s\n", oob->start, oob->length, oob->ptr, errstr);

	do_ioctl_ptr(MEMREADOOB, oob);
	printf("ioctl(-1, MEMREADOOB, {start=%#x, length=%#x, ptr=%p})"
	       " = %s\n", oob->start, oob->length, oob->ptr, errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct mtd_oob_buf64, oob64);
	fill_memory(oob64, sizeof(*oob64));

	do_ioctl_ptr(MEMWRITEOOB64, oob64);
	printf("ioctl(-1, MEMWRITEOOB64"
	       ", {start=%#llx, length=%#x, usr_ptr=%#llx}) = %s\n",
	       (unsigned long long) oob64->start, oob64->length,
	       (unsigned long long) oob64->usr_ptr, errstr);

	do_ioctl_ptr(MEMREADOOB64, oob64);
	printf("ioctl(-1, MEMREADOOB64"
	       ", {start=%#llx, length=%#x, usr_ptr=%#llx}) = %s\n",
	       (unsigned long long) oob64->start, oob64->length,
	       (unsigned long long) oob64->usr_ptr, errstr);


	TAIL_ALLOC_OBJECT_CONST_PTR(struct otp_info, oi);
	fill_memory(oi, sizeof(*oi));
	do_ioctl_ptr(OTPLOCK, oi);
	printf("ioctl(-1, MIXER_READ(%u) or OTPLOCK"
	       ", {start=%#x, length=%#x, locked=%u}) = %s\n",
	       (unsigned int) _IOC_NR(OTPLOCK),
	       oi->start, oi->length, oi->locked, errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct mtd_write_req, wr);
	fill_memory(wr, sizeof(*wr));
	wr->mode = MTD_OPS_PLACE_OOB;
	do_ioctl_ptr(MEMWRITE, wr);
	printf("ioctl(-1, MEMWRITE, {start=%#llx, len=%#llx, ooblen=%#llx"
	       ", usr_data=%#llx, usr_oob=%#llx, mode=MTD_OPS_PLACE_OOB})"
	       " = %s\n",
	       (unsigned long long) wr->start,
	       (unsigned long long) wr->len,
	       (unsigned long long) wr->ooblen,
	       (unsigned long long) wr->usr_data,
	       (unsigned long long) wr->usr_oob,
	       errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct mtd_info_user, minfo);
	fill_memory(minfo, sizeof(*minfo));
	minfo->type = MTD_ABSENT;
	minfo->flags = MTD_WRITEABLE;
	if (do_ioctl_ptr(MEMGETINFO, minfo) <0 ) {
		printf("ioctl(-1, MIXER_READ(%u) or MEMGETINFO, %p) = %s\n",
		       (unsigned int) _IOC_NR(MEMGETINFO), minfo, errstr);
	} else {
		printf("ioctl(-1, MIXER_READ(%u) or MEMGETINFO"
		       ", {type=MTD_ABSENT, flags=MTD_WRITEABLE, size=%#x"
		       ", erasesize=%#x, writesize=%#x, oobsize=%#x"
		       ", padding=%#jx}) = %s\n",
		       (unsigned int) _IOC_NR(MEMGETINFO),
		       minfo->size, minfo->erasesize,
		       minfo->writesize, minfo->oobsize,
		       (uintmax_t) minfo->padding, errstr);
	}

	TAIL_ALLOC_OBJECT_CONST_PTR(struct nand_oobinfo, ninfo);
	fill_memory(ninfo, sizeof(*ninfo));
	ninfo->useecc = MTD_NANDECC_OFF;
	if (do_ioctl_ptr(MEMGETOOBSEL, ninfo) < 0) {
		printf("ioctl(-1, MIXER_READ(%u) or MEMGETOOBSEL, %p) = %s\n",
		       (unsigned int) _IOC_NR(MEMGETOOBSEL), ninfo, errstr);
	} else {
		printf("ioctl(-1, MIXER_READ(%u) or MEMGETOOBSEL"
		       ", {useecc=MTD_NANDECC_OFF, eccbytes=%#x, oobfree=[",
		       (unsigned int) _IOC_NR(MEMGETOOBSEL), ninfo->eccbytes);
		for (unsigned int i = 0; i < ARRAY_SIZE(ninfo->oobfree); ++i)
			printf("%s[%#x, %#x]", i ? ", " : "",
			       ninfo->oobfree[i][0], ninfo->oobfree[i][1]);
		printf("], eccpos=[");
		for (unsigned int i = 0; i < ARRAY_SIZE(ninfo->eccpos); ++i)
			printf("%s%#x", i ? ", " : "", ninfo->eccpos[i]);
		printf("]}) = %s\n", errstr);
	}

	TAIL_ALLOC_OBJECT_CONST_PTR(struct nand_ecclayout_user, nlay);
	fill_memory(nlay, sizeof(*nlay));
	if (do_ioctl_ptr(ECCGETLAYOUT, nlay) < 0) {
		printf("ioctl(-1, MIXER_READ(%u) or ECCGETLAYOUT, %p) = %s\n",
		       (unsigned int) _IOC_NR(ECCGETLAYOUT), nlay, errstr);
	} else {
		printf("ioctl(-1, MIXER_READ(%u) or ECCGETLAYOUT"
		       ", {eccbytes=%#x, eccpos=[",
		       (unsigned int) _IOC_NR(ECCGETLAYOUT), nlay->eccbytes);
		for (unsigned int i = 0; i < DEFAULT_STRLEN; ++i)
			printf("%s%#x", i ? ", " : "", nlay->eccpos[i]);
		printf(", ...], oobavail=%#x, oobfree=[", nlay->oobavail);
		for (unsigned int i = 0; i < ARRAY_SIZE(nlay->oobfree); ++i)
			printf("%s{offset=%#x, length=%#x}", i ? ", " : "",
			       nlay->oobfree[i].offset,
			       nlay->oobfree[i].length);
		printf("]}) = %s\n", errstr);
	}

	TAIL_ALLOC_OBJECT_CONST_PTR(struct mtd_ecc_stats, es);
	fill_memory(es, sizeof(*es));
	if (do_ioctl_ptr(ECCGETSTATS, es) <0 ) {
		printf("ioctl(-1, MIXER_READ(%u) or ECCGETSTATS, %p) = %s\n",
		       (unsigned int) _IOC_NR(ECCGETSTATS), es, errstr);
	} else {
		printf("ioctl(-1, MIXER_READ(%u) or ECCGETSTATS"
		       ", {corrected=%#x, failed=%#x, badblocks=%#x"
		       ", bbtblocks=%#x}) = %s\n",
		       (unsigned int) _IOC_NR(ECCGETSTATS),
		       es->corrected, es->failed,
		       es->badblocks, es->bbtblocks, errstr);
	}

	static const unsigned long lmagic =
		(unsigned long) 0xdeadbeefbadc0dedULL;

	do_ioctl(_IOC(_IOC_READ|_IOC_WRITE, 0x4d, 0xfe, 0xff), lmagic);
	printf("ioctl(-1, %s, %#lx) = %s\n",
	       "_IOC(_IOC_READ|_IOC_WRITE, 0x4d, 0xfe, 0xff)",
	       lmagic, errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
