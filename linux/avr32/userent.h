/*
 * Copyright (c) 2014-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

{ uoff(regs.sr),	"sr" },
{ uoff(regs.pc),	"pc" },
{ uoff(regs.lr),	"lr" },
{ uoff(regs.sp),	"sp" },
{ uoff(regs.r12),	"r12" },
{ uoff(regs.r11),	"r11" },
{ uoff(regs.r10),	"r10" },
{ uoff(regs.r9),	"r9" },
{ uoff(regs.r8),	"r8" },
{ uoff(regs.r7),	"r7" },
{ uoff(regs.r6),	"r6" },
{ uoff(regs.r5),	"r5" },
{ uoff(regs.r4),	"r4" },
{ uoff(regs.r3),	"r3" },
{ uoff(regs.r2),	"r2" },
{ uoff(regs.r1),	"r1" },
{ uoff(regs.r0),	"r0" },
{ uoff(regs.r12_orig),	"orig_r12" },
/* Other fields in "struct user" */
XLAT_UOFF(u_tsize),
XLAT_UOFF(u_dsize),
XLAT_UOFF(u_ssize),
XLAT_UOFF(start_code),
XLAT_UOFF(start_data),
XLAT_UOFF(start_stack),
XLAT_UOFF(signal),
XLAT_UOFF(u_ar0),
XLAT_UOFF(magic),
XLAT_UOFF(u_comm),
#include "../userent0.h"
