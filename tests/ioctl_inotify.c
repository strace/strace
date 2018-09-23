/*
 * This file is part of ioctl_inotify strace test.
 *
 * Copyright (c) 2018 The strace developers.
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

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <asm/unistd.h>
#include <linux/ioctl.h>

#ifndef INOTIFY_IOC_SETNEXTWD
# define INOTIFY_IOC_SETNEXTWD  _IOW('I', 0, int32_t)
#endif

static long
sys_ioctl(kernel_long_t fd, kernel_ulong_t cmd, kernel_ulong_t arg)
{
	return syscall(__NR_ioctl, fd, cmd, arg);
}

int
main(void)
{
	static const kernel_ulong_t unknown_inotify_cmd =
		(kernel_ulong_t) 0xbadc0dedfeed49edULL;
	static const kernel_ulong_t magic =
		(kernel_ulong_t) 0xdeadbeefbadc0dedULL;

	/* Unknown inotify commands */
	sys_ioctl(-1, unknown_inotify_cmd, magic);
	printf("ioctl(-1, _IOC(%s_IOC_READ|_IOC_WRITE, 0x49, %#x, %#x), "
	       "%#lx) = -1 EBADF (%m)\n",
	       _IOC_DIR((unsigned int) unknown_inotify_cmd) & _IOC_NONE ?
	       "_IOC_NONE|" : "",
	       _IOC_NR((unsigned int) unknown_inotify_cmd),
	       _IOC_SIZE((unsigned int) unknown_inotify_cmd),
	       (unsigned long) magic);

	sys_ioctl(-1, INOTIFY_IOC_SETNEXTWD + 1, magic);
	printf("ioctl(-1, _IOC(_IOC_WRITE, 0x49, %#x, %#x), %#lx)"
	       " = -1 EBADF (%m)\n",
	       (unsigned int) _IOC_NR(INOTIFY_IOC_SETNEXTWD + 1),
	       (unsigned int) _IOC_SIZE(INOTIFY_IOC_SETNEXTWD + 1),
	       (unsigned long) magic);

	/* INOTIFY_IOC_SETNEXTWD */
	sys_ioctl(-1, INOTIFY_IOC_SETNEXTWD, magic);
	printf("ioctl(-1, INOTIFY_IOC_SETNEXTWD, %d) = -1 EBADF (%m)\n",
	       (int) magic);

	puts("+++ exited with 0 +++");
	return 0;
}
