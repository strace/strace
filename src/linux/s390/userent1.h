/*
 * Copyright (c) 2014-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

XLAT_UOFF(u_tsize),
XLAT_UOFF(u_dsize),
XLAT_UOFF(u_ssize),
XLAT_UOFF(start_code),
/* S390[X] has no start_data */
XLAT_UOFF(start_stack),
XLAT_UOFF(signal),
XLAT_UOFF(u_ar0),
XLAT_UOFF(magic),
XLAT_UOFF(u_comm),
#include "../generic/userent0.h"
