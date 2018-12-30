/*
 * Copyright (c) 2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/nbd.h>

#define XLAT_MACROS_ONLY
#include "xlat/nbd_ioctl_cmds.h"
#include "xlat/nbd_ioctl_flags.h"
#undef XLAT_MACROS_ONLY

#define RVAL_EBADF " = -1 EBADF (%m)\n"

int
main(void)
{
	static const unsigned long ubeef = (unsigned long) 0xcafef00ddeadbeefULL;
	static const char null_path[] = "/dev/null";

	int fd = open(null_path, O_RDONLY);
	if (fd < 0)
		perror_msg_and_fail("open(\"%s\")", null_path);

	skip_if_unavailable("/proc/self/fd/");

	ioctl(-1, NBD_DISCONNECT, NULL);
	printf("ioctl(-1, NBD_DISCONNECT)" RVAL_EBADF);
	ioctl(-1, NBD_CLEAR_SOCK, NULL);
	printf("ioctl(-1, NBD_CLEAR_SOCK)" RVAL_EBADF);
	ioctl(-1, NBD_DO_IT, NULL);
	printf("ioctl(-1, NBD_DO_IT)" RVAL_EBADF);
	ioctl(-1, NBD_CLEAR_QUE, NULL);
	printf("ioctl(-1, NBD_CLEAR_QUE)" RVAL_EBADF);
	ioctl(-1, NBD_PRINT_DEBUG, NULL);
	printf("ioctl(-1, NBD_PRINT_DEBUG)" RVAL_EBADF);
	ioctl(-1, NBD_SET_SOCK, fd);
	printf("ioctl(-1, NBD_SET_SOCK, %d</dev/null>)" RVAL_EBADF, fd);

	ioctl(-1, NBD_SET_BLKSIZE, ubeef);
	printf("ioctl(-1, NBD_SET_BLKSIZE, %lu)" RVAL_EBADF, ubeef);
	ioctl(-1, NBD_SET_SIZE, ubeef);
	printf("ioctl(-1, NBD_SET_SIZE, %lu)" RVAL_EBADF, ubeef);
	ioctl(-1, NBD_SET_SIZE_BLOCKS, ubeef);
	printf("ioctl(-1, NBD_SET_SIZE_BLOCKS, %lu)" RVAL_EBADF, ubeef);

	ioctl(-1, NBD_SET_TIMEOUT, ubeef);
	printf("ioctl(-1, NBD_SET_TIMEOUT, %lu)" RVAL_EBADF, ubeef);

	ioctl(-1, NBD_SET_FLAGS, 0);
	printf("ioctl(-1, NBD_SET_FLAGS, 0)" RVAL_EBADF);
	ioctl(-1, NBD_SET_FLAGS, NBD_FLAG_HAS_FLAGS);
	printf("ioctl(-1, NBD_SET_FLAGS, NBD_FLAG_HAS_FLAGS)" RVAL_EBADF);
	ioctl(-1, NBD_SET_FLAGS, NBD_FLAG_READ_ONLY);
	printf("ioctl(-1, NBD_SET_FLAGS, NBD_FLAG_READ_ONLY)" RVAL_EBADF);
	ioctl(-1, NBD_SET_FLAGS, NBD_FLAG_SEND_FLUSH);
	printf("ioctl(-1, NBD_SET_FLAGS, NBD_FLAG_SEND_FLUSH)" RVAL_EBADF);
	ioctl(-1, NBD_SET_FLAGS, NBD_FLAG_SEND_FUA);
	printf("ioctl(-1, NBD_SET_FLAGS, NBD_FLAG_SEND_FUA)" RVAL_EBADF);
	ioctl(-1, NBD_SET_FLAGS, NBD_FLAG_SEND_TRIM);
	printf("ioctl(-1, NBD_SET_FLAGS, NBD_FLAG_SEND_TRIM)" RVAL_EBADF);
	ioctl(-1, NBD_SET_FLAGS, NBD_FLAG_SEND_WRITE_ZEROES);
	printf("ioctl(-1, NBD_SET_FLAGS, NBD_FLAG_SEND_WRITE_ZEROES)" RVAL_EBADF);
	ioctl(-1, NBD_SET_FLAGS, NBD_FLAG_SEND_DF);
	printf("ioctl(-1, NBD_SET_FLAGS, NBD_FLAG_SEND_DF)" RVAL_EBADF);
	ioctl(-1, NBD_SET_FLAGS, NBD_FLAG_CAN_MULTI_CONN);
	printf("ioctl(-1, NBD_SET_FLAGS, NBD_FLAG_CAN_MULTI_CONN)" RVAL_EBADF);
	ioctl(-1, NBD_SET_FLAGS, NBD_FLAG_SEND_RESIZE);
	printf("ioctl(-1, NBD_SET_FLAGS, NBD_FLAG_SEND_RESIZE)" RVAL_EBADF);
	ioctl(-1, NBD_SET_FLAGS, NBD_FLAG_SEND_CACHE);
	printf("ioctl(-1, NBD_SET_FLAGS, NBD_FLAG_SEND_CACHE)" RVAL_EBADF);

	ioctl(-1, NBD_SET_FLAGS, NBD_FLAG_HAS_FLAGS|NBD_FLAG_READ_ONLY|
	                         NBD_FLAG_SEND_FUA|NBD_FLAG_SEND_CACHE|
	                         (1 << 15)|(1<<31));
	printf("ioctl(-1, NBD_SET_FLAGS, NBD_FLAG_HAS_FLAGS|NBD_FLAG_READ_ONLY|"
	       "NBD_FLAG_SEND_FUA|NBD_FLAG_SEND_CACHE|0x80008000)" RVAL_EBADF);

	ioctl(-1, _IOC(_IOC_NONE, 0xab, 0xb, 0), NULL);
	printf("ioctl(-1, _IOC(_IOC_NONE, 0xab, 0xb, 0), 0)" RVAL_EBADF);

	puts("+++ exited with 0 +++");
	return 0;
}
