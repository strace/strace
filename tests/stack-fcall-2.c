/*
 * Copyright (c) 2014-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "stack-fcall.h"

int
f2(int i, unsigned long f)
{
	f ^= (unsigned long) (void *) f2;
	COMPLEX_BODY(i, f);
	return f3(i, f) - i;
}
