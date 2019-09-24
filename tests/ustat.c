/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_ustat

# include <stdio.h>
# include <sys/stat.h>
# include <sys/sysmacros.h>
# include <unistd.h>
# ifdef HAVE_USTAT_H
#  include <ustat.h>
# endif

int
main(void)
{
	const kernel_ulong_t magic = (kernel_ulong_t) 0xfacefeedffffffff;
	unsigned long long buf[4];
	unsigned int dev;
	long rc;

# ifdef HAVE_USTAT_H
	TAIL_ALLOC_OBJECT_CONST_PTR(struct ustat, ust);
	struct stat st;
	if (stat(".", &st))
		perror_msg_and_fail("stat");

	dev = (unsigned int) st.st_dev;
	rc = syscall(__NR_ustat, dev, ust);
	if (rc)
		printf("ustat(makedev(%#x, %#x), %p) = %s\n",
		       major(dev), minor(dev), ust, sprintrc(rc));
	else
		printf("ustat(makedev(%#x, %#x)"
		       ", {f_tfree=%llu, f_tinode=%llu}) = 0\n",
		       major(dev), minor(dev),
		       zero_extend_signed_to_ull(ust->f_tfree),
		       zero_extend_signed_to_ull(ust->f_tinode));
# endif /* HAVE_USTAT_H */

	dev = (unsigned int) magic;
	rc = syscall(__NR_ustat, magic, 0);
	printf("ustat(makedev(%#x, %#x), NULL) = %s\n",
	       major(dev), minor(dev), sprintrc(rc));

	rc = syscall(__NR_ustat, magic, buf);
	printf("ustat(makedev(%#x, %#x), %p) = %s\n",
	       major(dev), minor(dev), buf, sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_ustat")

#endif
