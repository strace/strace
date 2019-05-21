/*
 * Check decoding of RND* commands of ioctl syscall.
 *
 * Copyright (c) 2018-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/random.h>

#define XLAT_MACROS_ONLY
#include "xlat/random_ioctl_cmds.h"
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
	printf("ioctl(-1, FASTRPC_IOCTL_INIT_ATTACH or RNDZAPENTCNT)"
	       RVAL_EBADF);
	ioctl(-1, RNDCLEARPOOL);
	printf("ioctl(-1, RNDCLEARPOOL)" RVAL_EBADF);
	ioctl(-1, RNDRESEEDCRNG);
	printf("ioctl(-1, RNDRESEEDCRNG)" RVAL_EBADF);

	ioctl(-1, _IO('R', 0xff), NULL);
	printf("ioctl(-1, _IOC(_IOC_NONE, %#x, 0xff, 0), 0)" RVAL_EBADF, 'R');

	puts("+++ exited with 0 +++");
	return 0;
}
