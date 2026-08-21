/*
 * Copyright (c) 2026 Thomas Weißschuh <thomas.weissschuh@linutronix.de>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdbool.h>

bool
is_auxiliary_clock(int clockid);

unsigned int
auxiliary_clock_num(int clockid);
