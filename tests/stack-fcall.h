/*
 * Copyright (c) 2017-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <unistd.h>
#include "scno.h"
#include "gcc_compat.h"

#ifdef MANGLE

# define f0 _ZN2ns2f0Ei
# define f1 _ZN2ns2f1Ei
# define f2 _ZN2ns2f2Ei
# define f3 _ZN2ns2f3Ei

#endif

int f0(int i, unsigned long) ATTRIBUTE_NOINLINE;
int f1(int i, unsigned long) ATTRIBUTE_NOINLINE;
int f2(int i, unsigned long) ATTRIBUTE_NOINLINE;
int f3(int i, unsigned long) ATTRIBUTE_NOINLINE;

#define COMPLEX_BODY(i, f)				\
	do {						\
		int tid = syscall(__NR_gettid, f);	\
		if (i == tid)				\
			return 0;			\
		switch ((unsigned int) tid & 3) {	\
			case 0:				\
				i += f0(tid, f);	\
				break;			\
			case 1:				\
				i += f1(tid, f);	\
				break;			\
			case 2:				\
				i += f2(tid, f);	\
				break;			\
			case 3:				\
				i += f3(tid, f);	\
				break;			\
		}					\
	} while (0)
