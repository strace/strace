/*
 * Copyright (c) 2019 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_KERNEL_TIMEX_H
# define STRACE_KERNEL_TIMEX_H

# if HAVE_ARCH_TIME32_SYSCALLS

typedef struct {
	unsigned int modes;
	int offset;
	int freq;
	int maxerror;
	int esterror;
	int status;
	int constant;
	int precision;
	int tolerance;
	struct {
		int tv_sec;
		int tv_usec;
	} time;
	int tick;
	int ppsfreq;
	int jitter;
	int shift;
	int stabil;
	int jitcnt;
	int calcnt;
	int errcnt;
	int stbcnt;
	int tai;
	int pad0[11];
} kernel_timex32_t;

# endif /* HAVE_ARCH_TIME32_SYSCALLS */

#endif /* !STRACE_KERNEL_TIMEX_H */
