/*
 * Invoke a prctl syscall with very specific arguments for use as a marker.
 *
 * Copyright (c) 2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include <unistd.h>

long
prctl_marker(void)
{
	return syscall(__NR_prctl, -1U,
				   (unsigned long) -2U,
				   (unsigned long) -3U,
				   (unsigned long) -4U,
				   (unsigned long) -5U);
}
