#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <fcntl.h>
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

#if defined MMTIMER_GETRES \
 && defined HIDIOCGVERSION \
 && defined HIDIOCGPHYS \
 && defined EVIOCGBIT \
 && defined EV_KEY

int
main(void )
{
	struct termios tty;
	uint64_t data;

	if (ioctl(-1, TCGETS, &tty) != -1 ||
	    ioctl(-1, MMTIMER_GETRES, &data) != -1 ||
	    ioctl(-1, HIDIOCGVERSION, &data) != -1 ||
	    ioctl(-1, HIDIOCGPHYS(8), &data) != -1 ||
	    ioctl(-1, EVIOCGBIT(EV_KEY, 8), &data) != -1 ||
	    ioctl(-1, _IOR(0xde, 0xad, data), &data) != -1)
		return 77;

	return 0;
}

#else

int
main(void )
{
	return 77;
}

#endif
