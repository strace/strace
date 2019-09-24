/*
 * Find out the size of kernel's sigset_t.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2017-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <signal.h>
#include <unistd.h>
#include "scno.h"

/*
 * If the sigset size specified to rt_sigprocmask is not equal to the size
 * of kernel's sigset_t, the kernel does not look at anything else and fails
 * with EINVAL.
 *
 * Otherwise, if both pointers specified to rt_sigprocmask are NULL,
 * the kernel just returns 0.
 *
 * This vaguely documented kernel feature can be used to probe
 * the kernel and find out the size of kernel's sigset_t.
 */

unsigned int
get_sigset_size(void)
{
	static unsigned int set_size;

	if (!set_size) {
		static const unsigned int big_size = 1024 / 8;

		for (set_size = big_size; set_size; set_size >>= 1) {
			if (!syscall(__NR_rt_sigprocmask, SIG_SETMASK,
				     NULL, NULL, set_size))
				break;
		}

		if (!set_size)
			perror_msg_and_fail("rt_sigprocmask");
	}

	return set_size;
}
