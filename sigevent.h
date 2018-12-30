/*
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_SIGEVENT_H
# define STRACE_SIGEVENT_H

typedef struct {
	union {
		int sival_int;
		unsigned long sival_ptr;
	} sigev_value;
	int sigev_signo;
	int sigev_notify;
	union {
		int tid;
		struct {
			unsigned long function;
			unsigned long attribute;
		} sigev_thread;
	} sigev_un;
} struct_sigevent;

#endif /* !STRACE_SIGEVENT_H */
