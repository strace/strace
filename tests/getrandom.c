/*
 * Check decoding of getrandom syscall.
 *
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <stdio.h>
#include <unistd.h>

int
main(void)
{
	unsigned char buf[4];

	/*
	 * syscall/printf are in the inverted order to trigger tcache
	 * initialisation first see glibc-2.33.9000-879-gfc859c3.
	 */
	printf("getrandom(NULL, 0, 0) = 0\n");
	if (syscall(__NR_getrandom, 0, 0, 0) != 0)
		perror_msg_and_skip("getrandom(NULL, 0, 0)");

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
