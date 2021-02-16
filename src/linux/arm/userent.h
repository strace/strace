/*
 * Copyright (c) 2014-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

{ uoff(regs.ARM_r0),	"r0" },
{ uoff(regs.ARM_r1),	"r1" },
{ uoff(regs.ARM_r2),	"r2" },
{ uoff(regs.ARM_r3),	"r3" },
{ uoff(regs.ARM_r4),	"r4" },
{ uoff(regs.ARM_r5),	"r5" },
{ uoff(regs.ARM_r6),	"r6" },
{ uoff(regs.ARM_r7),	"r7" },
{ uoff(regs.ARM_r8),	"r8" },
{ uoff(regs.ARM_r9),	"r9" },
{ uoff(regs.ARM_r10),	"r10" },
{ uoff(regs.ARM_fp),	"fp" },
{ uoff(regs.ARM_ip),	"ip" },
{ uoff(regs.ARM_sp),	"sp" },
{ uoff(regs.ARM_lr),	"lr" },
{ uoff(regs.ARM_pc),	"pc" },
{ uoff(regs.ARM_cpsr),	"cpsr" },
/* Other fields in "struct user" */
XLAT_UOFF(u_fpvalid),
XLAT_UOFF(u_tsize),
XLAT_UOFF(u_dsize),
XLAT_UOFF(u_ssize),
XLAT_UOFF(start_code),
XLAT_UOFF(start_stack),
XLAT_UOFF(signal),
XLAT_UOFF(reserved),
XLAT_UOFF(u_ar0),
XLAT_UOFF(magic),
XLAT_UOFF(u_comm),
#include "userent0.h"
