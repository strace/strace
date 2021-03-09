/*
 * Copyright (c) 2018-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <linux/ioctl.h>

#ifndef INOTIFY_IOC_SETNEXTWD
# define INOTIFY_IOC_SETNEXTWD  _IOW('I', 0, int32_t)
#endif

int
inotify_ioctl(struct tcb *const tcp, const unsigned int code,
	       const kernel_ulong_t arg)
{
	switch (code) {
	case INOTIFY_IOC_SETNEXTWD:
		tprint_arg_next();
		PRINT_VAL_D((int) arg);

		return RVAL_IOCTL_DECODED;
	}

	return RVAL_DECODED;
}
