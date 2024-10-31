/*
 * [gs]etsockopt() wrappers that avoid glibc and perform syscalls directly.
 *
 * Copyright (c) 2019 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2019-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef STRACE_K_SOCKOPT_H
#define STRACE_K_SOCKOPT_H

extern long
k_getsockopt(const unsigned int fd, const unsigned int level,
	     const unsigned int optname, const void *const optval,
	     const unsigned int *len);

extern long
k_setsockopt(const unsigned int fd, const unsigned int level,
	     const unsigned int optname, const void *const optval,
	     const unsigned int len);

#endif /* STRACE_K_SOCKOPT_H */
