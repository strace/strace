/*
 * Copyright (c) 2014-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

XLAT(4*PT_D1),
XLAT(4*PT_D2),
XLAT(4*PT_D3),
XLAT(4*PT_D4),
XLAT(4*PT_D5),
XLAT(4*PT_D6),
XLAT(4*PT_D7),
XLAT(4*PT_A0),
XLAT(4*PT_A1),
XLAT(4*PT_A2),
XLAT(4*PT_A3),
XLAT(4*PT_A4),
XLAT(4*PT_A5),
XLAT(4*PT_A6),
XLAT(4*PT_D0),
XLAT(4*PT_USP),
XLAT(4*PT_ORIG_D0),
XLAT(4*PT_SR),
XLAT(4*PT_PC),
/* Other fields in "struct user" */
XLAT_UOFF(u_fpvalid),
XLAT_UOFF(m68kfp),
XLAT_UOFF(u_tsize),
XLAT_UOFF(u_dsize),
XLAT_UOFF(u_ssize),
XLAT_UOFF(start_code),
XLAT_UOFF(start_stack),
XLAT_UOFF(signal),
XLAT_UOFF(reserved),
XLAT_UOFF(u_ar0),
XLAT_UOFF(u_fpstate),
XLAT_UOFF(magic),
XLAT_UOFF(u_comm),
#include "userent0.h"
