/*
 * Copyright (c) 2014-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

XLAT(8*R15),
XLAT(8*R14),
XLAT(8*R13),
XLAT(8*R12),
XLAT(8*RBP),
XLAT(8*RBX),
XLAT(8*R11),
XLAT(8*R10),
XLAT(8*R9),
XLAT(8*R8),
XLAT(8*RAX),
XLAT(8*RCX),
XLAT(8*RDX),
XLAT(8*RSI),
XLAT(8*RDI),
XLAT(8*ORIG_RAX),
XLAT(8*RIP),
XLAT(8*CS),
{ 8*EFLAGS, "8*EFL" },
XLAT(8*RSP),
XLAT(8*SS),
/* Other fields in "struct user" */
#include "../i386/userent0.h"
