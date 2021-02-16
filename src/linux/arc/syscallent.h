/*
 * Copyright (c) 2013-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#define sys_ARCH_mmap sys_mmap_pgoff
#include "../32/syscallent.h"
/* [244 ... 259] are arch specific */
[244] = { 3,	0,	SEN(printargs),	"cacheflush"	},
[245] = { 1,	0,	SEN(printargs),	"arc_settls"	},
[246] = { 0,	PU|NF,	SEN(printargs),	"arc_gettls"	},
[247] = { 3,	0,	SEN(sysfs),	"sysfs"		},
[248] = { 3,	0,	SEN(printargs),	"arc_usr_cmpxchg"},
