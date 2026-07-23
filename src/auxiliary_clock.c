/*
 * Linux UAPI auxiliary-clock helpers for the time decoder.
 *
 * This separate compilation unit includes <linux/time.h> for the clock
 * definitions but no libc headers: their time declarations can conflict
 * with the UAPI definitions.
 *
 * Copyright (c) 2026 Thomas Weißschuh <thomas.weissschuh@linutronix.de>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "auxiliary_clock.h"
#include <linux/time.h>

int
is_auxiliary_clock(int clockid)
{
	return clockid >= CLOCK_AUX && clockid <= CLOCK_AUX_LAST;
}

unsigned int
auxiliary_clock_num(int clockid)
{
	return clockid - CLOCK_AUX;
}
