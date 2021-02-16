/*
 * Copyright (c) 2013-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "../32/syscallent.h"
/* [244 ... 259] are arch specific */
[245] = { 2,	0,	SEN(printargs),	"metag_setglobalbit"	},
[246] = { 1,	0,	SEN(printargs),	"metag_set_fpu_flags"	},
[247] = { 1,	0,	SEN(printargs),	"metag_set_tls"		},
[248] = { 0,	PU|NF,	SEN(printargs),	"metag_get_tls"		},
