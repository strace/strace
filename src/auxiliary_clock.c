/*
 * Copyright (c) 2026 Thomas Weißschuh <thomas.weissschuh@linutronix.de>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <linux/time.h>

#include "auxiliary_clock.h"

bool
is_auxiliary_clock(int clockid)
{
	return clockid >= CLOCK_AUX && clockid <= CLOCK_AUX_LAST;
}

unsigned int
auxiliary_clock_num(int clockid)
{
	return clockid - CLOCK_AUX;
}
