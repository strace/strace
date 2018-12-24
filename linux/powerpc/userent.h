/*
 * Copyright (c) 2014-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef PT_ORIG_R3
# define PT_ORIG_R3 34
#endif
#define REGSIZE (sizeof(unsigned long))
{ REGSIZE*PT_R0,		"r0" },
{ REGSIZE*PT_R1,		"r1" },
{ REGSIZE*PT_R2,		"r2" },
{ REGSIZE*PT_R3,		"r3" },
{ REGSIZE*PT_R4,		"r4" },
{ REGSIZE*PT_R5,		"r5" },
{ REGSIZE*PT_R6,		"r6" },
{ REGSIZE*PT_R7,		"r7" },
{ REGSIZE*PT_R8,		"r8" },
{ REGSIZE*PT_R9,		"r9" },
{ REGSIZE*PT_R10,		"r10" },
{ REGSIZE*PT_R11,		"r11" },
{ REGSIZE*PT_R12,		"r12" },
{ REGSIZE*PT_R13,		"r13" },
{ REGSIZE*PT_R14,		"r14" },
{ REGSIZE*PT_R15,		"r15" },
{ REGSIZE*PT_R16,		"r16" },
{ REGSIZE*PT_R17,		"r17" },
{ REGSIZE*PT_R18,		"r18" },
{ REGSIZE*PT_R19,		"r19" },
{ REGSIZE*PT_R20,		"r20" },
{ REGSIZE*PT_R21,		"r21" },
{ REGSIZE*PT_R22,		"r22" },
{ REGSIZE*PT_R23,		"r23" },
{ REGSIZE*PT_R24,		"r24" },
{ REGSIZE*PT_R25,		"r25" },
{ REGSIZE*PT_R26,		"r26" },
{ REGSIZE*PT_R27,		"r27" },
{ REGSIZE*PT_R28,		"r28" },
{ REGSIZE*PT_R29,		"r29" },
{ REGSIZE*PT_R30,		"r30" },
{ REGSIZE*PT_R31,		"r31" },
{ REGSIZE*PT_NIP,		"NIP" },
{ REGSIZE*PT_MSR,		"MSR" },
{ REGSIZE*PT_ORIG_R3,		"ORIG_R3" },
{ REGSIZE*PT_CTR,		"CTR" },
{ REGSIZE*PT_LNK,		"LNK" },
{ REGSIZE*PT_XER,		"XER" },
{ REGSIZE*PT_CCR,		"CCR" },
{ REGSIZE*PT_FPR0,		"FPR0" },
#undef REGSIZE
/* Other fields in "struct user" */
#include "../userent0.h"
