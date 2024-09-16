/*
 * Copyright (c) 2015-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>

int
main(void)
{
	uint8_t array[8] = { 0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef };
	char array_str[] = "\\xfe\\xed\\xfa\\xce\\xde\\xad\\xbe\\xef";

	/* Zero-length read */
	(void) ioctl(-1, _IOC(_IOC_READ, 0xde, 0x0, 0), (kernel_ulong_t) -1ULL);
	printf("ioctl(-1, _IOC(_IOC_READ, 0xde, 0, 0), %#lx)"
	       RVAL_EBADF, -1UL);

	/* Zero-length write */
	(void) ioctl(-1, _IOC(_IOC_WRITE, 0xde, 0, 0), (kernel_ulong_t) -1ULL);
	printf("ioctl(-1, _IOC(_IOC_WRITE, 0xde, 0, 0), %#lx)"
	       RVAL_EBADF, -1UL);

	/* Data read from ioctl */
	(void) ioctl(-1, _IOR(0xde, 0xad, array), array);
	printf("ioctl(-1, _IOC(_IOC_READ, 0xde, 0xad, 0x8), \"%s\")"
	       RVAL_EBADF, array_str);

	/* Data written to ioctl */
	(void) ioctl(-1, _IOW(0xde, 0xad, array), array);
	printf("ioctl(-1, _IOC(_IOC_WRITE, 0xde, 0xad, 0x8), \"%s\")"
	       RVAL_EBADF, array_str);

	/* Bi-directional data movement */
	(void) ioctl(-1, _IOWR(0xde, 0xad, array), array);
	printf("ioctl(-1, _IOC(_IOC_READ|_IOC_WRITE, 0xde, 0xad, 0x8)"
	       ", \"%s\" => \"%s\")"
	       RVAL_EBADF, array_str, array_str);

	/* Neither read nor write implied by ioctl, but valid arg provided */
	(void) ioctl(-1, _IOC(_IOC_NONE, 0xde, 0xad, sizeof(array)), array);
	printf("ioctl(-1, _IOC(_IOC_NONE, 0xde, 0xad, 0x8), \"%s\" => \"%s\")"
	       RVAL_EBADF, array_str, array_str);

	/* Arg expected for ioctl, but arg pointer is bad */
	(void) ioctl(-1, _IOC(_IOC_NONE, 0xde, 0, 8), (kernel_ulong_t) -1ULL);
	printf("ioctl(-1, _IOC(_IOC_NONE, 0xde, 0, 0x8), %#lx)"
	       RVAL_EBADF, -1UL);

	puts("+++ exited with 0 +++");
	return 0;
}
