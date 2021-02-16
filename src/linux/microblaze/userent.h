/*
 * Copyright (c) 2014-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

{ PT_GPR(0),		"r0" },
{ PT_GPR(1),		"r1" },
{ PT_GPR(2),		"r2" },
{ PT_GPR(3),		"r3" },
{ PT_GPR(4),		"r4" },
{ PT_GPR(5),		"r5" },
{ PT_GPR(6),		"r6" },
{ PT_GPR(7),		"r7" },
{ PT_GPR(8),		"r8" },
{ PT_GPR(9),		"r9" },
{ PT_GPR(10),		"r10" },
{ PT_GPR(11),		"r11" },
{ PT_GPR(12),		"r12" },
{ PT_GPR(13),		"r13" },
{ PT_GPR(14),		"r14" },
{ PT_GPR(15),		"r15" },
{ PT_GPR(16),		"r16" },
{ PT_GPR(17),		"r17" },
{ PT_GPR(18),		"r18" },
{ PT_GPR(19),		"r19" },
{ PT_GPR(20),		"r20" },
{ PT_GPR(21),		"r21" },
{ PT_GPR(22),		"r22" },
{ PT_GPR(23),		"r23" },
{ PT_GPR(24),		"r24" },
{ PT_GPR(25),		"r25" },
{ PT_GPR(26),		"r26" },
{ PT_GPR(27),		"r27" },
{ PT_GPR(28),		"r28" },
{ PT_GPR(29),		"r29" },
{ PT_GPR(30),		"r30" },
{ PT_GPR(31),		"r31" },
{ PT_PC,		"rpc" },
{ PT_MSR,		"rmsr" },
{ PT_EAR,		"rear" },
{ PT_ESR,		"resr" },
{ PT_FSR,		"rfsr" },
{ PT_KERNEL_MODE,	"kernel_mode" },
/* Other fields in "struct user" */
#include "userent0.h"
