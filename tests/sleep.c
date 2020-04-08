/*
 * A simple nanosleep based sleep(1) replacement.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_nanosleep

# include <stdlib.h>
# include <unistd.h>

# include "kernel_old_timespec.h"

int
main(int ac, char **av)
{
	if (ac < 2)
		error_msg_and_fail("missing operand");

	if (ac > 2)
		error_msg_and_fail("extra operand");

	kernel_old_timespec_t ts = { atoi(av[1]), 0 };

	if (syscall(__NR_nanosleep, (unsigned long) &ts, 0))
		perror_msg_and_fail("nanosleep");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_nanosleep")

#endif
