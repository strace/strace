/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2020 The strace developers.
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

#include <linux/types.h>

#ifdef HAVE_LINUX_MMTIMER_H
# include <linux/mmtimer.h>
#endif
#ifdef HAVE_LINUX_HIDDEV_H
# include <linux/hiddev.h>
#endif
#ifdef HAVE_LINUX_INPUT_H
# include <linux/input.h>
#endif

#include <linux/videodev2.h>

#if defined MMTIMER_GETRES \
 && defined VIDIOC_ENUMINPUT \
 && defined HIDIOCGVERSION \
 && defined HIDIOCGPHYS \
 && defined EVIOCGBIT \
 && defined EV_KEY

int
main(void)
{
	uint64_t data = 0;

# ifndef POWERPC
	struct termios tty;
	(void) ioctl(-1, TCGETS, &tty);
	printf("ioctl(-1, TCGETS, %p)"
	       " = -1 EBADF (%m)\n", &tty);
# endif

	(void) ioctl(-1, MMTIMER_GETRES, &data);
	printf("ioctl(-1, MMTIMER_GETRES, %p)"
	       " = -1 EBADF (%m)\n", &data);

	(void) ioctl(-1, VIDIOC_ENUMINPUT, 0);
	printf("ioctl(-1, VIDIOC_ENUMINPUT, NULL)"
	       " = -1 EBADF (%m)\n");

	(void) ioctl(-1, HIDIOCGVERSION, &data);
	printf("ioctl(-1, HIDIOCGRDESCSIZE or HIDIOCGVERSION, %p)"
	       " = -1 EBADF (%m)\n", &data);

	(void) ioctl(-1, HIDIOCGPHYS(8), &data);
	printf("ioctl(-1, HIDIOCGPHYS(8), %p)"
	       " = -1 EBADF (%m)\n", &data);

	(void) ioctl(-1, EVIOCGBIT(EV_KEY, 8), &data);
	printf("ioctl(-1, EVIOCGBIT(EV_KEY, 8), %p)"
	       " = -1 EBADF (%m)\n", &data);

	(void) ioctl(-1, _IOR('M', 13, int), &data);
# ifdef HAVE_STRUCT_MTD_WRITE_REQ
	printf("ioctl(-1, MIXER_READ(13) or OTPSELECT, [MTD_OTP_OFF])"
	       " = -1 EBADF (%m)\n");
# else
	printf("ioctl(-1, MIXER_READ(13) or OTPSELECT, %p)"
	       " = -1 EBADF (%m)\n", &data);
# endif

	(void) ioctl(-1, _IOC(_IOC_WRITE, 0xde, 0, 0), (kernel_ulong_t) -1ULL);
	printf("ioctl(-1, _IOC(_IOC_WRITE, 0xde, 0, 0), %#lx)"
	       " = -1 EBADF (%m)\n", -1UL);

	(void) ioctl(-1, _IOR(0xde, 0xad, data), &data);
	printf("ioctl(-1, _IOC(_IOC_READ, 0xde, 0xad, 0x8), %p)"
	       " = -1 EBADF (%m)\n", &data);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("MMTIMER_GETRES && VIDIOC_ENUMINPUT"
		    " && HIDIOCGVERSION && HIDIOCGPHYS"
		    " && EVIOCGBIT && EV_KEY")

#endif
