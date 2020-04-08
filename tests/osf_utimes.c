/*
 * Check decoding of osf_utimes syscall.
 *
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2017-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_osf_utimes

# define TEST_SYSCALL_NR	__NR_osf_utimes
# define TEST_SYSCALL_STR	"osf_utimes"
# define TEST_STRUCT		struct timeval32
struct timeval32 { int tv_sec, tv_usec; };
# include "xutimes.c"

#else

SKIP_MAIN_UNDEFINED("__NR_osf_utimes")

#endif
