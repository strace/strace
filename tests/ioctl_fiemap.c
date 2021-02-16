/*
 * Check decoding of FS_IOC_FIEMAP ioctl command.
 *
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/fiemap.h>
#include <linux/fs.h>

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
		int rc = ioctl(-1, FS_IOC_FIEMAP, 0);

		printf("ioctl(-1, %s, NULL) = %s%s\n",
		       XLAT_STR(FS_IOC_FIEMAP), sprintrc(rc),
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

	TAIL_ALLOC_OBJECT_VAR_PTR(struct fiemap, fiemap);

	do_ioctl_ptr(FS_IOC_FIEMAP, (char *) fiemap + 1);
	printf("ioctl(-1, %s, %p) = %s\n",
	       XLAT_STR(FS_IOC_FIEMAP), (char *) fiemap + 1, errstr);

#define VALID_FM_FLAGS        0x7
#define INVALID_FM_FLAGS 0xfffffff8

	fiemap->fm_start = (typeof(fiemap->fm_start)) 0xdeadbeefcafef00dULL;
	fiemap->fm_length = (typeof(fiemap->fm_length)) 0xfacefeedbabec0deULL;
	fiemap->fm_flags = VALID_FM_FLAGS;
	fiemap->fm_mapped_extents = 0xbadc0ded;
	fiemap->fm_extent_count = 0xdeadc0de;

	int rc = do_ioctl_ptr(FS_IOC_FIEMAP, fiemap);
	printf("ioctl(-1, %s, {fm_start=%ju, fm_length=%ju"
	       ", fm_flags=%s, fm_extent_count=%u}",
	       XLAT_STR(FS_IOC_FIEMAP),
	       (uintmax_t) fiemap->fm_start, (uintmax_t) fiemap->fm_length,
	       XLAT_KNOWN(VALID_FM_FLAGS,
			  "FIEMAP_FLAG_SYNC|FIEMAP_FLAG_XATTR|"
			  "FIEMAP_FLAG_CACHE"),
	       fiemap->fm_extent_count);
	if (rc < 0) {
		printf(") = %s\n", errstr);
	} else {
		printf(" => {fm_flags=%s, fm_mapped_extents=%u, ",
		       XLAT_KNOWN(VALID_FM_FLAGS,
				  "FIEMAP_FLAG_SYNC|FIEMAP_FLAG_XATTR|"
				  "FIEMAP_FLAG_CACHE"),
		       fiemap->fm_mapped_extents);
#if VERBOSE
		printf("fm_extents=%p", fiemap + 1);
#else
		printf("...");
#endif
		printf("}) = %s\n", errstr);
	}

#define VALID_FE_FLAGS     0x3f8f
#define INVALID_FE_FLAGS 0xffffc070

	fiemap = tail_alloc(sizeof(*fiemap) + 2 * sizeof(fiemap->fm_extents[0]));
	fiemap->fm_start = (typeof(fiemap->fm_start)) 0xdeadbeefcafef00dULL;
	fiemap->fm_length = (typeof(fiemap->fm_length)) 0xfacefeedbabec0deULL;
	fiemap->fm_flags = ~VALID_FM_FLAGS;
	fiemap->fm_mapped_extents = 2;
	fiemap->fm_extent_count = 0xdeadc0de;

	fiemap->fm_extents[0].fe_logical = 0xfacefed1deadbef1;
	fiemap->fm_extents[0].fe_physical = 0xfacefed2deadbef2;
	fiemap->fm_extents[0].fe_length = 0xfacefed3deadbef3;
	fiemap->fm_extents[0].fe_flags = VALID_FE_FLAGS;

	fiemap->fm_extents[1].fe_logical = 0xfacefed1deadbef4;
	fiemap->fm_extents[1].fe_physical = 0xfacefed2deadbef5;
	fiemap->fm_extents[1].fe_length = 0xfacefed3deadbef6;
	fiemap->fm_extents[1].fe_flags = ~VALID_FE_FLAGS;

	rc = do_ioctl_ptr(FS_IOC_FIEMAP, fiemap);
	printf("ioctl(-1, %s, {fm_start=%ju, fm_length=%ju"
	       ", fm_flags=%s, fm_extent_count=%u}",
	       XLAT_STR(FS_IOC_FIEMAP),
	       (uintmax_t) fiemap->fm_start, (uintmax_t) fiemap->fm_length,
	       XLAT_UNKNOWN(INVALID_FM_FLAGS, "FIEMAP_FLAG_???"),
	       fiemap->fm_extent_count);
	if (rc < 0) {
		printf(") = %s\n", errstr);
	} else {
		printf(" => {fm_flags=%s, fm_mapped_extents=%u, ",
		       XLAT_UNKNOWN(INVALID_FM_FLAGS, "FIEMAP_FLAG_???"),
		       fiemap->fm_mapped_extents);
#if VERBOSE
		printf("fm_extents=[{fe_logical=%ju, fe_physical=%ju"
		       ", fe_length=%ju, fe_flags=%s}"
		       ", {fe_logical=%ju, fe_physical=%ju"
		       ", fe_length=%ju, fe_flags=%s}]",
		       (uintmax_t) fiemap->fm_extents[0].fe_logical,
		       (uintmax_t) fiemap->fm_extents[0].fe_physical,
		       (uintmax_t) fiemap->fm_extents[0].fe_length,
		       XLAT_KNOWN(VALID_FE_FLAGS,
			          "FIEMAP_EXTENT_LAST|"
				  "FIEMAP_EXTENT_UNKNOWN|"
				  "FIEMAP_EXTENT_DELALLOC|"
				  "FIEMAP_EXTENT_ENCODED|"
				  "FIEMAP_EXTENT_DATA_ENCRYPTED|"
				  "FIEMAP_EXTENT_NOT_ALIGNED|"
				  "FIEMAP_EXTENT_DATA_INLINE|"
				  "FIEMAP_EXTENT_DATA_TAIL|"
				  "FIEMAP_EXTENT_UNWRITTEN|"
				  "FIEMAP_EXTENT_MERGED|"
				  "FIEMAP_EXTENT_SHARED"),
		       (uintmax_t) fiemap->fm_extents[1].fe_logical,
		       (uintmax_t) fiemap->fm_extents[1].fe_physical,
		       (uintmax_t) fiemap->fm_extents[1].fe_length,
		       XLAT_UNKNOWN(INVALID_FE_FLAGS, "FIEMAP_EXTENT_???"));
#else
		printf("...");
#endif
		printf("}) = %s\n", errstr);
	}

	/* The live version of this test is in btrfs.c */

	puts("+++ exited with 0 +++");
	return 0;
}
