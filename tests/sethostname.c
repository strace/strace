/*
 * Check decoding of sethostname syscall.
 *
 * Copyright (c) 2016 Fei Jie <feij.fnst@cn.fujitsu.com>
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
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

#ifdef __NR_sethostname

# include <stdio.h>
# include <unistd.h>

#ifdef HAVE_LINUX_UTSNAME_H
# include <linux/utsname.h>
#endif

#ifndef __NEW_UTS_LEN
# define __NEW_UTS_LEN 64
#endif

int
main(void)
{
	kernel_ulong_t len;
	long rc;

	len = __NEW_UTS_LEN;
	rc = syscall(__NR_sethostname, 0, len);
	printf("sethostname(NULL, %u) = %s\n",
	       (unsigned) len, sprintrc(rc));

	if (F8ILL_KULONG_MASK) {
		len |= F8ILL_KULONG_MASK;
		rc = syscall(__NR_sethostname, 0, len);
		printf("sethostname(NULL, %u) = %s\n",
		       (unsigned) len, sprintrc(rc));
	}

	len = __NEW_UTS_LEN + 1;
	void *const p = tail_alloc(len);
	rc = syscall(__NR_sethostname, p, len);
	printf("sethostname(%p, %u) = %s\n",
	       p, (unsigned) len, sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_sethostname")

#endif
