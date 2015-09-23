#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

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
main(void )
{
	struct termios tty;
	uint64_t data = 0;

	(void) ioctl(-1, TCGETS, &tty);
	printf("ioctl(-1, TCGETS, %p)"
	       " = -1 EBADF (Bad file descriptor)\n", &tty);

	(void) ioctl(-1, MMTIMER_GETRES, &data);
	printf("ioctl(-1, MMTIMER_GETRES, %p)"
	       " = -1 EBADF (Bad file descriptor)\n", &data);

	(void) ioctl(-1, VIDIOC_ENUMINPUT, 0);
	printf("ioctl(-1, VIDIOC_ENUMINPUT, 0)"
	       " = -1 EBADF (Bad file descriptor)\n");

	(void) ioctl(-1, HIDIOCGVERSION, &data);
	printf("ioctl(-1, HIDIOCGRDESCSIZE or HIDIOCGVERSION, %p)"
	       " = -1 EBADF (Bad file descriptor)\n", &data);

	(void) ioctl(-1, HIDIOCGPHYS(8), &data);
	printf("ioctl(-1, HIDIOCGPHYS(8), %p)"
	       " = -1 EBADF (Bad file descriptor)\n", &data);

	(void) ioctl(-1, EVIOCGBIT(EV_KEY, 8), &data);
	printf("ioctl(-1, EVIOCGBIT(EV_KEY, 8), %p)"
	       " = -1 EBADF (Bad file descriptor)\n", &data);

	(void) ioctl(-1, _IOR('M', 13, int), &data);
	printf("ioctl(-1, MIXER_READ(13) or OTPSELECT, [MTD_OTP_OFF])"
	       " = -1 EBADF (Bad file descriptor)\n");

	(void) ioctl(-1, _IOR(0xde, 0xad, data), &data);
	printf("ioctl(-1, _IOC(_IOC_READ, 0xde, 0xad, 0x08), %p)"
	       " = -1 EBADF (Bad file descriptor)\n", &data);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

int
main(void )
{
	return 77;
}

#endif
