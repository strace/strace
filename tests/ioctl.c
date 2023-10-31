/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

#include <linux/hiddev.h>
#include <linux/input.h>
#include <linux/mmtimer.h>
#include <linux/videodev2.h>

int
main(void)
{
	uint64_t data = 0;

#ifndef POWERPC
	struct termios tty;
	(void) ioctl(-1, TCGETS, &tty);
	printf("ioctl(-1, TCGETS, %p)" RVAL_EBADF, &tty);
#endif

	(void) ioctl(-1, MMTIMER_GETRES, &data);
	printf("ioctl(-1, MMTIMER_GETRES, %p)" RVAL_EBADF, &data);

	(void) ioctl(-1, VIDIOC_ENUMINPUT, 0);
	printf("ioctl(-1, VIDIOC_ENUMINPUT, NULL)" RVAL_EBADF);

	(void) ioctl(-1, HIDIOCGVERSION, &data);
	printf("ioctl(-1, HIDIOCGRDESCSIZE or HIDIOCGVERSION, %p)"
	       RVAL_EBADF, &data);

	(void) ioctl(-1, HIDIOCGPHYS(8), &data);
	printf("ioctl(-1, HIDIOCGPHYS(8), %p)" RVAL_EBADF, &data);

	(void) ioctl(-1, EVIOCGBIT(EV_KEY, 8), &data);
	printf("ioctl(-1, EVIOCGBIT(EV_KEY, 8), %p)" RVAL_EBADF, &data);

	(void) ioctl(-1, _IOR('M', 13, int), &data);
	printf("ioctl(-1, MIXER_READ(13) or OTPSELECT, [MTD_OTP_OFF])"
	       RVAL_EBADF);

	(void) ioctl(-1, _IOC(_IOC_WRITE, 0xde, 0, 0), (kernel_ulong_t) -1ULL);
	printf("ioctl(-1, _IOC(_IOC_WRITE, 0xde, 0, 0), %#lx)"
	       RVAL_EBADF, -1UL);

	(void) ioctl(-1, _IOR(0xde, 0xad, data), &data);
	printf("ioctl(-1, _IOC(_IOC_READ, 0xde, 0xad, 0x8), %p)"
	       RVAL_EBADF, &data);

	(void) ioctl(-1, 'Z' << 8, &data);
	printf("ioctl(-1, ZFS_IOC_POOL_CREATE, %p)" RVAL_EBADF, &data);

	(void) ioctl(-1, 'Z' << 8 | 'A', &data);
	printf("ioctl(-1, ZFS_IOC_SEND_SPACE, %p)" RVAL_EBADF, &data);

	(void) ioctl(-1, _IOR(0x12, 125, char[256]), &data);
	printf("ioctl(-1, BLKZNAME, %p)" RVAL_EBADF, &data);

	(void) ioctl(-1, ('K' << 8) + 1, &data);
	printf("ioctl(-1, KSTAT_IOC_CHAIN_ID, %p)" RVAL_EBADF, &data);

	puts("+++ exited with 0 +++");
	return 0;
}
