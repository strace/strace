/*
 * Copyright (c) 2014-2015 Dmitry V. Levin <ldv@altlinux.org>
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/syscall.h>

int
main(void)
{
#if defined(__NR_getuid) \
 && defined(__NR_setuid) \
 && defined(__NR_getresuid) \
 && defined(__NR_setreuid) \
 && defined(__NR_setresuid) \
 && defined(__NR_fchown) \
 && defined(__NR_getgroups) \
 \
 && defined(__NR_getuid32) \
 && defined(__NR_setuid32) \
 && defined(__NR_getresuid32) \
 && defined(__NR_setreuid32) \
 && defined(__NR_setresuid32) \
 && defined(__NR_fchown32) \
 && defined(__NR_getgroups32) \
 \
 && __NR_getuid != __NR_getuid32 \
 && __NR_setuid != __NR_setuid32 \
 && __NR_getresuid != __NR_getresuid32 \
 && __NR_setreuid != __NR_setreuid32 \
 && __NR_setresuid != __NR_setresuid32 \
 && __NR_fchown != __NR_fchown32 \
 && __NR_getgroups != __NR_getgroups32 \
 /**/
	int uid;
	int size;
	int *list = 0;

	uid = syscall(__NR_getuid);

	(void) close(0);
	if (open("/proc/sys/kernel/overflowuid", O_RDONLY) == 0) {
		/* we trust the kernel */
		char buf[sizeof(int)*3];
		int n = read(0, buf, sizeof(buf) - 1);
		if (n) {
			buf[n] = '\0';
			n = atoi(buf);
			if (uid == n)
				return 77;
		}
		close(0);
	}

	assert(syscall(__NR_setuid, uid) == 0);
	{
		/*
		 * uids returned by getresuid should be ignored
		 * to avoid 16bit vs 32bit issues.
		 */
		int r, e, s;
		assert(syscall(__NR_getresuid, &r, &e, &s) == 0);
	}
	assert(syscall(__NR_setreuid, -1, 0xffff) == 0);
	assert(syscall(__NR_setresuid, uid, -1, 0xffff) == 0);
	assert(syscall(__NR_fchown, 1, -1, 0xffff) == 0);
	assert((size = syscall(__NR_getgroups, 0, list)) >= 0);
	assert(list = calloc(size + 1, sizeof(*list)));
	assert(syscall(__NR_getgroups, size, list) == size);
	return 0;
#else
	return 77;
#endif
}
