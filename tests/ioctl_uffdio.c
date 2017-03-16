/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c)      2016 Red Hat, Inc.
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
#include <asm/unistd.h>

#if defined __NR_userfaultfd && defined HAVE_LINUX_USERFAULTFD_H

# include <fcntl.h>
# include <inttypes.h>
# include <stdint.h>
# include <stdio.h>
# include <string.h>
# include <unistd.h>

# include <sys/ioctl.h>
# include <sys/mman.h>
# include <linux/ioctl.h>
# include <linux/userfaultfd.h>

int
main(void)
{
	int rc;
	int fd = syscall(__NR_userfaultfd, O_NONBLOCK);
	size_t pagesize = getpagesize();

	if (fd < 0)
		perror_msg_and_skip("userfaultfd");

	/* ---- API ---- */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct uffdio_api, api_struct);

	/* With a bad fd */
	memset(api_struct, 0, sizeof(*api_struct));
	rc = ioctl(-1, UFFDIO_API, api_struct);
	printf("ioctl(-1, UFFDIO_API, {api=0, features=0}) = %d %s (%m)\n",
	       rc, errno2name());
	/* With a bad pointer */
	rc = ioctl(fd, UFFDIO_API, NULL);
	printf("ioctl(%d, UFFDIO_API, NULL) = %d %s (%m)\n",
		fd, rc, errno2name());
	/* Normal call */
	api_struct->api = UFFD_API;
	api_struct->features = 0;
	rc = ioctl(fd, UFFDIO_API, api_struct);
	printf("ioctl(%d, UFFDIO_API, {api=0xaa, features=0, "
	       "features.out=%#" PRIx64 ", " "ioctls=1<<_UFFDIO_REGISTER|"
	       "1<<_UFFDIO_UNREGISTER|1<<_UFFDIO_API",
	       fd, (uint64_t)api_struct->features);
	api_struct->ioctls &= ~(1ull<<_UFFDIO_REGISTER|
				1ull<<_UFFDIO_UNREGISTER|
				1ull<<_UFFDIO_API);
	if (api_struct->ioctls)
		printf("|%#" PRIx64, (uint64_t)api_struct->ioctls);
	printf("}) = %d\n", rc);

	/* For the rest of the tests we need some anonymous memory */
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
	struct uffdio_register *register_struct =
					 tail_alloc(sizeof(*register_struct));
	memset(register_struct, 0, sizeof(*register_struct));

	rc = ioctl(-1, UFFDIO_REGISTER, register_struct);
	printf("ioctl(-1, UFFDIO_REGISTER, {range={start=0, len=0}, "
	       "mode=0}) = %d %s (%m)\n", rc, errno2name());

	rc = ioctl(fd, UFFDIO_REGISTER, NULL);
	printf("ioctl(%d, UFFDIO_REGISTER, NULL) = %d %s (%m)\n",
	       fd, rc, errno2name());

	register_struct->range.start = (uint64_t)(uintptr_t)area2;
	register_struct->range.len = pagesize;
	register_struct->mode = UFFDIO_REGISTER_MODE_MISSING;
	rc = ioctl(fd, UFFDIO_REGISTER, register_struct);
	printf("ioctl(%d, UFFDIO_REGISTER, {range={start=%p, len=%#zx}, "
	       "mode=UFFDIO_REGISTER_MODE_MISSING, ioctls="
	       "1<<_UFFDIO_WAKE|1<<_UFFDIO_COPY|1<<_UFFDIO_ZEROPAGE",
	       fd, area2, pagesize);
	register_struct->ioctls &= ~(1ull<<_UFFDIO_WAKE|
				    1ull<<_UFFDIO_COPY|
				    1ull<<_UFFDIO_ZEROPAGE);
	if (register_struct->ioctls)
		printf("|%#" PRIx64, (uint64_t)register_struct->ioctls);
	printf("}) = %d\n", rc);

	/* With area2 registered we can now do the atomic copies onto it
	 * but be careful not to access it in any other way otherwise
	 * userfaultfd will cause us to stall.
	 */
	/* ---- COPY ---- */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct uffdio_copy, copy_struct);

	memset(copy_struct, 0, sizeof(*copy_struct));
	rc = ioctl(-1, UFFDIO_COPY, copy_struct);
	printf("ioctl(-1, UFFDIO_COPY, {dst=0, src=0, len=0, mode=0"
	       "}) = %d %s (%m)\n", rc, errno2name());

	rc = ioctl(fd, UFFDIO_COPY, NULL);
	printf("ioctl(%d, UFFDIO_COPY, NULL) = %d %s (%m)\n",
	       fd, rc, errno2name());

	copy_struct->dst = (uint64_t)(uintptr_t)area2;
	copy_struct->src = (uint64_t)(uintptr_t)area1;
	copy_struct->len = pagesize;
	copy_struct->mode = UFFDIO_COPY_MODE_DONTWAKE;
	rc = ioctl(fd, UFFDIO_COPY, copy_struct);
	printf("ioctl(%d, UFFDIO_COPY, {dst=%p, src=%p, len=%#zx,"
	       " mode=UFFDIO_COPY_MODE_DONTWAKE, copy=%#zx}) = %d\n",
	       fd, area2, area1, pagesize, pagesize, rc);

	/* ---- ZEROPAGE ---- */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct uffdio_zeropage, zero_struct);
	madvise(area2, pagesize, MADV_DONTNEED);

	memset(zero_struct, 0, sizeof(*zero_struct));
	rc = ioctl(-1, UFFDIO_ZEROPAGE, zero_struct);
	printf("ioctl(-1, UFFDIO_ZEROPAGE, {range={start=0, len=0}, mode=0"
	       "}) = %d %s (%m)\n", rc, errno2name());

	rc = ioctl(fd, UFFDIO_ZEROPAGE, NULL);
	printf("ioctl(%d, UFFDIO_ZEROPAGE, NULL) = %d %s (%m)\n",
	       fd, rc, errno2name());

	zero_struct->range.start = (uint64_t)(uintptr_t)area2;
	zero_struct->range.len = pagesize;
	zero_struct->mode = UFFDIO_ZEROPAGE_MODE_DONTWAKE;
	rc = ioctl(fd, UFFDIO_ZEROPAGE, zero_struct);
	printf("ioctl(%d, UFFDIO_ZEROPAGE, {range={start=%p, len=%#zx},"
	       " mode=UFFDIO_ZEROPAGE_MODE_DONTWAKE, zeropage=%#zx}) = %d\n",
	       fd, area2, pagesize, pagesize, rc);

	/* ---- WAKE ---- */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct uffdio_range, range_struct);
	memset(range_struct, 0, sizeof(*range_struct));

	rc = ioctl(-1, UFFDIO_WAKE, range_struct);
	printf("ioctl(-1, UFFDIO_WAKE, {start=0, len=0}) = %d %s (%m)\n",
	       rc, errno2name());

	rc = ioctl(fd, UFFDIO_WAKE, NULL);
	printf("ioctl(%d, UFFDIO_WAKE, NULL) = %d %s (%m)\n",
	       fd, rc, errno2name());

	range_struct->start = (uint64_t)(uintptr_t)area2;
	range_struct->len = pagesize;
	rc = ioctl(fd, UFFDIO_WAKE, range_struct);
	printf("ioctl(%d, UFFDIO_WAKE, {start=%p, len=%#zx}) = %d\n",
	       fd, area2, pagesize, rc);

	/* ---- UNREGISTER ---- */
	memset(range_struct, 0, sizeof(*range_struct));

	rc = ioctl(-1, UFFDIO_UNREGISTER, range_struct);
	printf("ioctl(-1, UFFDIO_UNREGISTER, {start=0, len=0}) = %d %s (%m)\n",
	       rc, errno2name());

	rc = ioctl(fd, UFFDIO_UNREGISTER, NULL);
	printf("ioctl(%d, UFFDIO_UNREGISTER, NULL) = %d %s (%m)\n",
	       fd, rc, errno2name());

	range_struct->start = (uint64_t)(uintptr_t)area2;
	range_struct->len = pagesize;
	rc = ioctl(fd, UFFDIO_UNREGISTER, range_struct);
	printf("ioctl(%d, UFFDIO_UNREGISTER, {start=%p, len=%#zx}) = %d\n",
	       fd, area2, pagesize, rc);
	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_userfaultfd && HAVE_LINUX_USERFAULTFD_H")

#endif
