/*
 * Copyright (c) 2015-2020 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include <assert.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/sendfile.h>
#include <sys/prctl.h>

int main(void)
{
	const unsigned long pagesize = get_page_size();

#ifdef __s390__
	/*
	 * The si_addr field is unreliable:
	 * https://marc.info/?l=linux-s390&m=142515870124248&w=2
	 */
	error_msg_and_skip("s390: si_addr is unreliable");
#endif

	/* write instruction pointer length to the log */
	assert(syscall(__NR_write, -1, (void *) 8UL, 2 * sizeof(void *)) < 0);

	/* just a noticeable line in the log */
	assert(munmap(&main, 0) < 0);

	int pid = fork();
	if (pid < 0)
		perror_msg_and_fail("fork");

	if (!pid) {
		const unsigned long mask = ~(pagesize - 1);
		unsigned long addr = (unsigned long) &main & mask;
		unsigned long size = pagesize << 1;

#ifdef HAVE_DLADDR
		Dl_info info;
		if (dladdr(&main, &info)) {
			const unsigned long base =
				(unsigned long) info.dli_fbase & mask;
			if (base < addr) {
				size += addr - base;
				addr = base;
			}
		} else
#endif
		{
			addr -= size;
			size <<= 1;
		}

		/* Avoid creating core dumps */
		(void) prctl(PR_SET_DUMPABLE, 0, 0, 0, 0);

		/* SIGSEGV is expected */
		(void) munmap((void *) addr, size);
		(void) munmap((void *) addr, size);
		error_msg_and_skip("SIGSEGV did not happen");
	}

	int status;
	assert(wait(&status) == pid);
	assert(WIFSIGNALED(status));
	assert(WTERMSIG(status) == SIGSEGV);

	/* dump process map for debug purposes */
	close(0);
	if (!open("/proc/self/maps", O_RDONLY))
		(void) sendfile(1, 0, NULL, pagesize);

	return 0;
}
