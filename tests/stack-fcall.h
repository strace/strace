/*
 * Copyright (c) 2017-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifdef MANGLE

# define f0 _ZN2ns2f0Ei
# define f1 _ZN2ns2f1Ei
# define f2 _ZN2ns2f2Ei
# define f3 _ZN2ns2f3Ei

#endif

int f0(int i, unsigned long);
int f1(int i, unsigned long);
int f2(int i, unsigned long);
int f3(int i, unsigned long);
