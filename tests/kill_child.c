/*
 * Check for the corner case that previously lead to segfault
 * due to an attempt to access unitialised tcp->s_ent.
 *
 * 13994 ????( <unfinished ...>
 * ...
 * 13994 <... ???? resumed>) = ?
 *
 * Copyright (c) 2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <sched.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define ITERS    10000
#define SC_ITERS 10000

int
main(void)
{
	volatile sig_atomic_t *const mem =
		mmap(NULL, get_page_size(), PROT_READ | PROT_WRITE,
		     MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (mem == MAP_FAILED)
		perror_msg_and_fail("mmap");

	for (unsigned int i = 0; i < ITERS; ++i) {
		mem[0] = mem[1] = 0;

		const pid_t pid = fork();
		if (pid < 0)
			perror_msg_and_fail("fork");

		if (!pid) {
			/* wait for the parent */
			while (!mem[0])
				;
			/* let the parent know we are running */
			mem[1] = 1;

			for (unsigned int j = 0; j < SC_ITERS; j++)
				sched_yield();

			pause();
			return 0;
		}

		/* let the child know we are running */
		mem[0] = 1;
		/* wait for the child */
		while (!mem[1])
			;

		if (kill(pid, SIGKILL))
			perror_msg_and_fail("kill");
		if (wait(NULL) != pid)
			perror_msg_and_fail("wait");
	}

	return 0;
}
