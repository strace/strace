/*
 * Copyright (c) 2014-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

XLAT(4*EBX),
XLAT(4*ECX),
XLAT(4*EDX),
XLAT(4*ESI),
XLAT(4*EDI),
XLAT(4*EBP),
XLAT(4*EAX),
XLAT(4*DS),
XLAT(4*ES),
XLAT(4*FS),
XLAT(4*GS),
XLAT(4*ORIG_EAX),
XLAT(4*EIP),
XLAT(4*CS),
XLAT(4*EFL),
XLAT(4*UESP),
XLAT(4*SS),
/* Other fields in "struct user" */
#include "userent0.h"
