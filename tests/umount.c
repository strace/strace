/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
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

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/syscall.h>

#ifdef __NR_oldumount
# define TEST_SYSCALL_STR "oldumount"
#else
# if defined __NR_umount && defined __NR_umount2
#  define __NR_oldumount __NR_umount
#  define TEST_SYSCALL_STR "umount"
# endif
#endif

int
main(void)
{
#ifdef __NR_oldumount
	static const char sample[] = "umount.sample";
	if (mkdir(sample, 0700)) {
		perror(sample);
		return 77;
	}
	(void) syscall(__NR_oldumount, sample);
	printf("%s(\"%s\") = -1 ", TEST_SYSCALL_STR, sample);
	switch (errno) {
		case ENOSYS:
			printf("ENOSYS (%m)\n");
			break;
		case EPERM:
			printf("EPERM (%m)\n");
			break;
		default:
			printf("EINVAL (%m)\n");
	}
	(void) rmdir(sample);
	puts("+++ exited with 0 +++");
	return 0;
#else
	return 77;
#endif
}
