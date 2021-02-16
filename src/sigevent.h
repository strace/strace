/*
 * Copyright (c) 2016-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_SIGEVENT_H
# define STRACE_SIGEVENT_H

typedef struct {
	union {
		int sival_int;
		void *sival_ptr;
	} sigev_value;
	int sigev_signo;
	int sigev_notify;
	union {
		int tid;
		struct {
			void *function;
			void *attribute;
		} sigev_thread;
	} sigev_un;
} struct_sigevent;

#endif /* !STRACE_SIGEVENT_H */
