/*
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#define shuffle_scno_pers_is_static
#define shuffle_scno_pers arm_shuffle_scno_pers
#include "../arm/shuffle_scno.c"
#undef shuffle_scno_pers
#undef shuffle_scno_pers_is_static

kernel_ulong_t
shuffle_scno_pers(kernel_ulong_t scno, int pers)
{
	if (pers == 1)
		return arm_shuffle_scno_pers(scno, pers);

	return scno;
}
