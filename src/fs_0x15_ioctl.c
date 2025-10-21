/*
 * Decoders of linux/fs.h 0x15 ioctl commands.
 *
 * Copyright (c) 2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/fs.h>

int
fs_0x15_ioctl(struct tcb *const tcp, const unsigned int code,
	      const kernel_ulong_t arg)
{
	switch (code) {
	default:
		return RVAL_DECODED;
	}

	return RVAL_IOCTL_DECODED;
}
