/*
 * Check decoding of UFFDIO_* ioctl commands.
 *
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c)      2016 Red Hat, Inc.
 * Copyright (c) 2016-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/mman.h>

#include "kernel_fcntl.h"
#include <linux/ioctl.h>
#include <linux/userfaultfd.h>

#include "xlat.h"
#include "xlat/uffd_api_features.h"

static const char *errstr;

static int
sys_ioctl(int fd, unsigned long cmd, void *arg)
{
	int rc = ioctl(fd, cmd, arg);
        errstr = sprintrc(rc);
        return rc;
}

int
main(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(struct uffdio_api, api_struct);
	memset(api_struct, 0, sizeof(*api_struct));
	TAIL_ALLOC_OBJECT_CONST_PTR(struct uffdio_register, register_struct);
	memset(register_struct, 0, sizeof(*register_struct));
	TAIL_ALLOC_OBJECT_CONST_PTR(struct uffdio_copy, copy_struct);
	memset(copy_struct, 0, sizeof(*copy_struct));
	TAIL_ALLOC_OBJECT_CONST_PTR(struct uffdio_zeropage, zero_struct);
	memset(zero_struct, 0, sizeof(*zero_struct));
	TAIL_ALLOC_OBJECT_CONST_PTR(struct uffdio_range, range_struct);
	memset(range_struct, 0, sizeof(*range_struct));
	TAIL_ALLOC_OBJECT_CONST_PTR(struct uffdio_writeprotect, writeprotect_struct);
	memset(writeprotect_struct, 0, sizeof(*writeprotect_struct));
	TAIL_ALLOC_OBJECT_CONST_PTR(struct uffdio_continue, continue_struct);
	memset(continue_struct, 0, sizeof(*continue_struct));
	TAIL_ALLOC_OBJECT_CONST_PTR(struct uffdio_poison, poison_struct);
	memset(poison_struct, 0, sizeof(*poison_struct));

	struct {
		unsigned int val;
		const char *str;
		void *ptr;
		const char *text;
	} requests[] = {
		{ ARG_STR(UFFDIO_API), api_struct,
		  "{api=0, features=0}" },
		{ ARG_STR(UFFDIO_REGISTER), register_struct,
		  "{range={start=0, len=0}, mode=0}" },
		{ ARG_STR(UFFDIO_UNREGISTER), range_struct,
		  "{start=0, len=0}" },
		{ ARG_STR(UFFDIO_WAKE), range_struct,
		  "{start=0, len=0}" },
		{ ARG_STR(UFFDIO_COPY), copy_struct,
		  "{dst=0, src=0, len=0, mode=0}" },
		{ ARG_STR(UFFDIO_ZEROPAGE), zero_struct,
		  "{range={start=0, len=0}, mode=0}" },
		{ ARG_STR(UFFDIO_WRITEPROTECT), writeprotect_struct,
		  "{range={start=0, len=0}, mode=0}" },
		{ ARG_STR(UFFDIO_CONTINUE), continue_struct,
		  "{range={start=0, len=0}, mode=0}" },
		{ ARG_STR(UFFDIO_POISON), poison_struct,
		  "{range={start=0, len=0}, mode=0}" },
	};

	for (unsigned int i = 0; i < ARRAY_SIZE(requests); ++i) {
		sys_ioctl(-1, requests[i].val, NULL);
		printf("ioctl(-1, %s, NULL) = %s\n",
		       requests[i].str, errstr);
		sys_ioctl(-1, requests[i].val, requests[i].ptr);
		printf("ioctl(-1, %s, %s) = %s\n",
		       requests[i].str, requests[i].text, errstr);
	}

	int rc;
	int fd = syscall(__NR_userfaultfd, O_NONBLOCK);

	/* ---- API ---- */
	api_struct->api = UFFD_API;
	api_struct->features = 0;
	rc = sys_ioctl(fd, UFFDIO_API, api_struct);
	printf("ioctl(%d, UFFDIO_API, {api=0xaa, features=0", fd);
	if (rc >= 0) {
		if (api_struct->features) {
			printf(" => features=");
			printflags(uffd_api_features, api_struct->features,
				   "UFFD_FEATURE_???");
		}
		printf(", ioctls=1<<_UFFDIO_REGISTER|"
		       "1<<_UFFDIO_UNREGISTER|1<<_UFFDIO_API");
		api_struct->ioctls &= ~(1ull<<_UFFDIO_REGISTER|
					1ull<<_UFFDIO_UNREGISTER|
					1ull<<_UFFDIO_API);
		if (api_struct->ioctls)
			printf("|%#" PRIx64, (uint64_t)api_struct->ioctls);
	}
	printf("}) = %s\n", errstr);

	/* For the rest of the tests we need some anonymous memory */
	size_t pagesize = getpagesize();
	void *area1 = mmap(NULL, pagesize, PROT_READ|PROT_WRITE,
			   MAP_PRIVATE|MAP_ANONYMOUS,
			   -1, 0);
	if (area1 == MAP_FAILED)
		perror_msg_and_fail("mmap area1");
	void *area2 = mmap(NULL, pagesize, PROT_READ|PROT_WRITE,
			   MAP_PRIVATE|MAP_ANONYMOUS,
			   -1, 0);
	if (area2 == MAP_FAILED)
		perror_msg_and_fail("mmap area2");
	madvise(area2, pagesize, MADV_DONTNEED);
	*(char *)area1 = 42;

	/* ---- REGISTER ---- */
	register_struct->range.start = (uint64_t)(uintptr_t)area2;
	register_struct->range.len = pagesize;
	register_struct->mode = UFFDIO_REGISTER_MODE_MISSING;
	rc = sys_ioctl(fd, UFFDIO_REGISTER, register_struct);
	printf("ioctl(%d, UFFDIO_REGISTER, {range={start=%p, len=%#zx}"
	       ", mode=UFFDIO_REGISTER_MODE_MISSING",
	       fd, area2, pagesize);
	if (rc >= 0) {
		printf(", ioctls=1<<_UFFDIO_WAKE|1<<_UFFDIO_COPY"
		       "|1<<_UFFDIO_ZEROPAGE");
		register_struct->ioctls &= ~(1ull<<_UFFDIO_WAKE|
					    1ull<<_UFFDIO_COPY|
					    1ull<<_UFFDIO_ZEROPAGE);
		if (register_struct->ioctls & (1ull<<_UFFDIO_POISON)) {
			printf("|1<<_UFFDIO_POISON");
			register_struct->ioctls &= ~(1ull<<_UFFDIO_POISON);
		}
		if (register_struct->ioctls)
			printf("|%#" PRIx64, (uint64_t)register_struct->ioctls);
	}
	printf("}) = %s\n", errstr);

	/* With area2 registered we can now do the atomic copies onto it
	 * but be careful not to access it in any other way otherwise
	 * userfaultfd will cause us to stall.
	 */
	/* ---- COPY ---- */
	copy_struct->dst = (uint64_t)(uintptr_t)area2;
	copy_struct->src = (uint64_t)(uintptr_t)area1;
	copy_struct->len = pagesize;
	copy_struct->mode = UFFDIO_COPY_MODE_DONTWAKE;
	rc = sys_ioctl(fd, UFFDIO_COPY, copy_struct);
	printf("ioctl(%d, UFFDIO_COPY, {dst=%p, src=%p, len=%#zx,"
	       " mode=UFFDIO_COPY_MODE_DONTWAKE",
	       fd, area2, area1, pagesize);
	if (rc >= 0)
		printf(", copy=%#zx", pagesize);
	printf("}) = %s\n", errstr);

	copy_struct->mode = 0xdeadbeef;
	sys_ioctl(fd, UFFDIO_COPY, copy_struct);
	printf("ioctl(%d, UFFDIO_COPY, {dst=%p, src=%p, len=%#zx,"
	       " mode=UFFDIO_COPY_MODE_DONTWAKE|UFFDIO_COPY_MODE_WP|0xdeadbeec"
	       "}) = %s\n",
	       fd, area2, area1, pagesize, errstr);

	/* ---- ZEROPAGE ---- */
	madvise(area2, pagesize, MADV_DONTNEED);
	zero_struct->range.start = (uint64_t)(uintptr_t)area2;
	zero_struct->range.len = pagesize;
	zero_struct->mode = UFFDIO_ZEROPAGE_MODE_DONTWAKE;
	rc = sys_ioctl(fd, UFFDIO_ZEROPAGE, zero_struct);
	printf("ioctl(%d, UFFDIO_ZEROPAGE, {range={start=%p, len=%#zx},"
	       " mode=UFFDIO_ZEROPAGE_MODE_DONTWAKE",
	       fd, area2, pagesize);
	if (rc >= 0)
		printf(", zeropage=%#zx", pagesize);
	printf("}) = %s\n", errstr);

	/* ---- WAKE ---- */
	range_struct->start = (uint64_t)(uintptr_t)area2;
	range_struct->len = pagesize;
	sys_ioctl(fd, UFFDIO_WAKE, range_struct);
	printf("ioctl(%d, UFFDIO_WAKE, {start=%p, len=%#zx}) = %s\n",
	       fd, area2, pagesize, errstr);

	/* ---- UNREGISTER ---- */
	memset(range_struct, 0, sizeof(*range_struct));
	range_struct->start = (uint64_t)(uintptr_t)area2;
	range_struct->len = pagesize;
	sys_ioctl(fd, UFFDIO_UNREGISTER, range_struct);
	printf("ioctl(%d, UFFDIO_UNREGISTER, {start=%p, len=%#zx}) = %s\n",
	       fd, area2, pagesize, errstr);

	/* ---- WRITEPROTECT ---- */
	writeprotect_struct->range.start = (uint64_t)(uintptr_t)area2;
	writeprotect_struct->range.len = pagesize;
	writeprotect_struct->mode =
		UFFDIO_WRITEPROTECT_MODE_WP|UFFDIO_WRITEPROTECT_MODE_DONTWAKE;
	sys_ioctl(fd, UFFDIO_WRITEPROTECT, writeprotect_struct);
	printf("ioctl(%d, UFFDIO_WRITEPROTECT, {range={start=%p, len=%#zx}"
	       ", mode=UFFDIO_WRITEPROTECT_MODE_WP"
	       "|UFFDIO_WRITEPROTECT_MODE_DONTWAKE}) = %s\n",
	       fd, area2, pagesize, errstr);

	/* ---- CONTINUE ---- */
	continue_struct->range.start = (uint64_t)(uintptr_t)area2;
	continue_struct->range.len = pagesize;
	continue_struct->mode = UFFDIO_CONTINUE_MODE_DONTWAKE;
	rc = sys_ioctl(fd, UFFDIO_CONTINUE, continue_struct);
	printf("ioctl(%d, UFFDIO_CONTINUE, {range={start=%p, len=%#zx}"
	       ", mode=UFFDIO_CONTINUE_MODE_DONTWAKE",
	       fd, area2, pagesize);
	if (rc >= 0)
		printf(", mapped=%llu",
		       (unsigned long long)(uint64_t) continue_struct->mapped);
	printf("}) = %s\n", errstr);

	/* ---- POISON ---- */
	poison_struct->range.start = (uint64_t)(uintptr_t)area2;
	poison_struct->range.len = pagesize;
	poison_struct->mode = UFFDIO_POISON_MODE_DONTWAKE;
	rc = sys_ioctl(fd, UFFDIO_POISON, poison_struct);
	printf("ioctl(%d, UFFDIO_POISON, {range={start=%p, len=%#zx}"
	       ", mode=UFFDIO_POISON_MODE_DONTWAKE",
	       fd, area2, pagesize);
	if (rc >= 0)
		printf(", updated=%llu",
		       (unsigned long long)(uint64_t) poison_struct->updated);
	printf("}) = %s\n", errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
