/*
 * Check decoding of WDIOC* commands of ioctl syscall.
 *
 * Copyright (c) 2019-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/watchdog.h>

#define XLAT_MACROS_ONLY
#include "xlat/watchdog_ioctl_cmds.h"
#undef XLAT_MACROS_ONLY

int
main(void)
{
	int val = 123;

	ioctl(-1, WDIOC_GETSTATUS, &val);
	printf("ioctl(-1, WDIOC_GETSTATUS, %p)" RVAL_EBADF, &val);

	ioctl(-1, WDIOC_GETBOOTSTATUS, &val);
	printf("ioctl(-1, WDIOC_GETBOOTSTATUS, %p)" RVAL_EBADF, &val);

	ioctl(-1, WDIOC_GETTEMP, &val);
	printf("ioctl(-1, WDIOC_GETTEMP, %p)" RVAL_EBADF, &val);

	ioctl(-1, WDIOC_GETTIMEOUT, &val);
	printf("ioctl(-1, WDIOC_GETTIMEOUT, %p)" RVAL_EBADF, &val);

	ioctl(-1, WDIOC_GETPRETIMEOUT, &val);
	printf("ioctl(-1, WDIOC_GETPRETIMEOUT, %p)" RVAL_EBADF, &val);

	ioctl(-1, WDIOC_GETTIMELEFT, &val);
	printf("ioctl(-1, WDIOC_GETTIMELEFT, %p)" RVAL_EBADF, &val);

	ioctl(-1, WDIOC_SETTIMEOUT, &val);
	printf("ioctl(-1, WDIOC_SETTIMEOUT, [123])" RVAL_EBADF);

	ioctl(-1, WDIOC_SETPRETIMEOUT, &val);
	printf("ioctl(-1, WDIOC_SETPRETIMEOUT, [123])" RVAL_EBADF);

	ioctl(-1, WDIOC_KEEPALIVE);
	printf("ioctl(-1, WDIOC_KEEPALIVE)" RVAL_EBADF);

	ioctl(-1, _IOC(_IOC_NONE, 'W', 0xff, 0), &val);
	printf("ioctl(-1, _IOC(_IOC_NONE, %#x, 0xff, 0), %p)" RVAL_EBADF,
	       'W', &val);

	puts("+++ exited with 0 +++");
	return 0;
}
