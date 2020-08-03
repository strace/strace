/*
 * Copyright (c) 2014-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "stack-fcall.h"

int
f1(int i, unsigned long f)
{
	f ^= (unsigned long) (void *) f1;
	COMPLEX_BODY(i, f);
	return f2(i, f) + i;
}
