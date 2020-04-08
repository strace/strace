/*
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_getrandom

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	unsigned char buf[4];

	if (syscall(__NR_getrandom, buf, sizeof(buf) - 1, 0) != sizeof(buf) - 1)
		perror_msg_and_skip("getrandom");
	printf("getrandom(\"\\x%02x\\x%02x\\x%02x\", 3, 0) = 3\n",
	       (int) buf[0], (int) buf[1], (int) buf[2]);

	if (syscall(__NR_getrandom, buf, sizeof(buf), 1) != sizeof(buf))
		perror_msg_and_skip("getrandom");
	printf("getrandom(\"\\x%02x\\x%02x\\x%02x\"..., 4, GRND_NONBLOCK) = 4\n",
	       (int) buf[0], (int) buf[1], (int) buf[2]);

	if (syscall(__NR_getrandom, buf, sizeof(buf), 0x3003) != -1)
		perror_msg_and_skip("getrandom");
	printf("getrandom(%p, 4, GRND_NONBLOCK|GRND_RANDOM|0x3000) = "
	       "-1 EINVAL (%m)\n", buf);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_getrandom")

#endif
