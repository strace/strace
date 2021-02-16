/*
 * Copyright (c) 2019-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#define XLAT_PT_REGS_OFF(member)		\
	{ offsetof(struct pt_regs, member),	\
	 "offsetof(struct pt_regs, " #member ")" }

XLAT_PT_REGS_OFF(gr[0]),
XLAT_PT_REGS_OFF(gr[1]),
XLAT_PT_REGS_OFF(gr[2]),
XLAT_PT_REGS_OFF(gr[3]),
XLAT_PT_REGS_OFF(gr[4]),
XLAT_PT_REGS_OFF(gr[5]),
XLAT_PT_REGS_OFF(gr[6]),
XLAT_PT_REGS_OFF(gr[7]),
XLAT_PT_REGS_OFF(gr[8]),
XLAT_PT_REGS_OFF(gr[9]),
XLAT_PT_REGS_OFF(gr[10]),
XLAT_PT_REGS_OFF(gr[11]),
XLAT_PT_REGS_OFF(gr[12]),
XLAT_PT_REGS_OFF(gr[13]),
XLAT_PT_REGS_OFF(gr[14]),
XLAT_PT_REGS_OFF(gr[15]),
XLAT_PT_REGS_OFF(gr[16]),
XLAT_PT_REGS_OFF(gr[17]),
XLAT_PT_REGS_OFF(gr[18]),
XLAT_PT_REGS_OFF(gr[19]),
XLAT_PT_REGS_OFF(gr[20]),
XLAT_PT_REGS_OFF(gr[21]),
XLAT_PT_REGS_OFF(gr[22]),
XLAT_PT_REGS_OFF(gr[23]),
XLAT_PT_REGS_OFF(gr[24]),
XLAT_PT_REGS_OFF(gr[25]),
XLAT_PT_REGS_OFF(gr[26]),
XLAT_PT_REGS_OFF(gr[27]),
XLAT_PT_REGS_OFF(gr[28]),
XLAT_PT_REGS_OFF(gr[29]),
XLAT_PT_REGS_OFF(gr[30]),
XLAT_PT_REGS_OFF(gr[31]),

XLAT_PT_REGS_OFF(sr[0]),
XLAT_PT_REGS_OFF(sr[1]),
XLAT_PT_REGS_OFF(sr[2]),
XLAT_PT_REGS_OFF(sr[3]),
XLAT_PT_REGS_OFF(sr[4]),
XLAT_PT_REGS_OFF(sr[5]),
XLAT_PT_REGS_OFF(sr[6]),

XLAT_PT_REGS_OFF(iasq[0]),
XLAT_PT_REGS_OFF(iasq[1]),

XLAT_PT_REGS_OFF(iaoq[0]),
XLAT_PT_REGS_OFF(iaoq[1]),

XLAT_PT_REGS_OFF(cr27),
XLAT_PT_REGS_OFF(orig_r28),
XLAT_PT_REGS_OFF(ksp),
XLAT_PT_REGS_OFF(kpc),
XLAT_PT_REGS_OFF(sar),
XLAT_PT_REGS_OFF(iir),
XLAT_PT_REGS_OFF(isr),
XLAT_PT_REGS_OFF(ior),
XLAT_PT_REGS_OFF(ipsw),

#undef XLAT_PT_REGS_OFF
