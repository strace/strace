/*
 * Check decoding of RND* commands of ioctl syscall.
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

#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/random.h>

#define XLAT_MACROS_ONLY
# include "xlat/random_ioctl_cmds.h"
#undef XLAT_MACROS_ONLY

#define RVAL_EBADF " = -1 EBADF (%m)\n"

int
main(void)
{
	union {
		char c[sizeof(struct rand_pool_info) + 8];
		struct rand_pool_info info;
	} u;
	struct rand_pool_info *info = &u.info;
	int cnt = 6;

	memcpy(info->buf, "12345678", 8);
	info->buf_size = 8;
	info->entropy_count = 3;

	ioctl(-1, RNDGETENTCNT, &cnt);
	printf("ioctl(-1, RNDGETENTCNT, %p)" RVAL_EBADF, &cnt);
	ioctl(-1, RNDADDTOENTCNT, &cnt);
	printf("ioctl(-1, RNDADDTOENTCNT, [6])" RVAL_EBADF);

	ioctl(-1, RNDADDENTROPY, NULL);
	printf("ioctl(-1, RNDADDENTROPY, NULL)" RVAL_EBADF);
	ioctl(-1, RNDADDENTROPY, info);
	printf("ioctl(-1, RNDADDENTROPY, {entropy_count=3, buf_size=8, buf=\"12345678\"})" RVAL_EBADF);

	ioctl(-1, RNDZAPENTCNT);
	printf("ioctl(-1, RNDZAPENTCNT)" RVAL_EBADF);
	ioctl(-1, RNDCLEARPOOL);
	printf("ioctl(-1, RNDCLEARPOOL)" RVAL_EBADF);
	ioctl(-1, RNDRESEEDCRNG);
	printf("ioctl(-1, RNDRESEEDCRNG)" RVAL_EBADF);

	ioctl(-1, _IO('R', 0xff), NULL);
	printf("ioctl(-1, _IOC(_IOC_NONE, %#x, 0xff, 0), 0)" RVAL_EBADF, 'R');

	puts("+++ exited with 0 +++");
	return 0;
}
