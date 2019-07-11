/*
 * Copyright (c) 2014-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "stack-fcall.h"

int main(void)
{
	f0(0, (unsigned long) (void *) main);
	f0(1, (unsigned long) (void *) main);
	return 0;
}
