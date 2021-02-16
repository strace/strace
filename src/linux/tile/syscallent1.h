/*
 * Copyright (c) 2013-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#define sys_ARCH_mmap sys_mmap_4koff
#define ARCH_WANT_SYNC_FILE_RANGE2 1
#include "../32/syscallent.h"
/* [244 ... 259] are arch specific */
[244] = { 1,	0,	SEN(printargs),	"cmpxchg_badaddr"	},
[245] = { 3,	0,	SEN(printargs),	"cacheflush"		},
