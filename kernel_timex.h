/*
 * Copyright (c) 2019 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_KERNEL_TIMEX_H
# define STRACE_KERNEL_TIMEX_H

# include "kernel_timeval.h"

typedef struct {
	unsigned int modes;
	int pad0;
	long long offset;
	long long freq;
	long long maxerror;
	long long esterror;
	int status;
	int pad1;
	long long constant;
	long long precision;
	long long tolerance;
	kernel_timeval64_t time;
	long long tick;
	long long ppsfreq;
	long long jitter;
	int shift;
	int pad2;
	long long stabil;
	long long jitcnt;
	long long calcnt;
	long long errcnt;
	long long stbcnt;
	int tai;
	int pad3[11];
} kernel_timex64_t;

# ifdef SPARC64

typedef struct {
	unsigned int modes;
	int pad0;
	long long offset;
	long long freq;
	long long maxerror;
	long long esterror;
	int status;
	int pad1;
	long long constant;
	long long precision;
	long long tolerance;
	struct {
		long long tv_sec;
		int tv_usec;
	} time;
	long long tick;
	long long ppsfreq;
	long long jitter;
	int shift;
	int pad2;
	long long stabil;
	long long jitcnt;
	long long calcnt;
	long long errcnt;
	long long stbcnt;
	int tai;
	int pad3[11];
} kernel_sparc64_timex_t;

# endif /* SPARC64 */

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
