/*
 * [gs]etsockopt() wrappers that avoid glibc and perform syscalls directly.
 *
 * Copyright (c) 2019 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2019-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <unistd.h>
#include <sys/socket.h>

#include "k_sockopt.h"

static const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;

#if defined __NR_getsockopt || defined __NR_setsockopt
static const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
#endif

#define SC_getsockopt 15
long
k_getsockopt(const unsigned int fd, const unsigned int level,
	     const unsigned int optname, const void *const optval,
	     const unsigned int *len)
{
	return syscall(
#ifdef __NR_getsockopt
		__NR_getsockopt,
#else /* socketcall */
		__NR_socketcall, SC_getsockopt,
#endif
		fill | fd , fill | level, fill | optname, optval, len
#ifdef __NR_getsockopt
		, bad
#endif
		);
}

# define SC_setsockopt 14
long
k_setsockopt(const unsigned int fd, const unsigned int level,
	     const unsigned int optname, const void *const optval,
	     const unsigned int len)
{
	return syscall(
#ifdef __NR_setsockopt
		__NR_setsockopt,
#else /* socketcall */
		__NR_socketcall, SC_setsockopt,
#endif
		fill | fd , fill | level, fill | optname, optval, fill | len
#ifdef __NR_setsockopt
		, bad
#endif
		);
}
