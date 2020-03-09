/*
 * Copyright (c) 1999-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_KERNEL_RUSAGE_H
# define STRACE_KERNEL_RUSAGE_H

# include "kernel_timeval.h"

typedef struct {
	kernel_old_timeval_t	ru_utime;
	kernel_old_timeval_t	ru_stime;
	kernel_long_t		ru_maxrss;
	kernel_long_t		ru_ixrss;
	kernel_long_t		ru_idrss;
	kernel_long_t		ru_isrss;
	kernel_long_t		ru_minflt;
	kernel_long_t		ru_majflt;
	kernel_long_t		ru_nswap;
	kernel_long_t		ru_inblock;
	kernel_long_t		ru_oublock;
	kernel_long_t		ru_msgsnd;
	kernel_long_t		ru_msgrcv;
	kernel_long_t		ru_nsignals;
	kernel_long_t		ru_nvcsw;
	kernel_long_t		ru_nivcsw;
} kernel_rusage_t;

#endif /* !STRACE_KERNEL_RUSAGE_H */
