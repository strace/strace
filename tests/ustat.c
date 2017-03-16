/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
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
		printf("ustat(makedev(%u, %u), %p) = %s\n",
		       major(dev), minor(dev), ust, sprintrc(rc));
	else
		printf("ustat(makedev(%u, %u)"
		       ", {f_tfree=%llu, f_tinode=%llu}) = 0\n",
		       major(dev), minor(dev),
		       zero_extend_signed_to_ull(ust->f_tfree),
		       zero_extend_signed_to_ull(ust->f_tinode));
# endif /* HAVE_USTAT_H */

	dev = (unsigned int) magic;
	rc = syscall(__NR_ustat, magic, 0);
	printf("ustat(makedev(%u, %u), NULL) = %s\n",
	       major(dev), minor(dev), sprintrc(rc));

	rc = syscall(__NR_ustat, magic, buf);
	printf("ustat(makedev(%u, %u), %p) = %s\n",
	       major(dev), minor(dev), buf, sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_ustat")

#endif
