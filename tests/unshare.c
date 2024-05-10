/*
 * Check decoding of unshare syscall.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <limits.h>
#include <stdio.h>
#include <unistd.h>

#ifdef PRINT_NAMESPACE
#define LINE_END ""
#else
#define LINE_END "\n"
#endif

int
main(void)
{
	static const kernel_ulong_t bogus_flags =
		(kernel_ulong_t) 0xbadc0ded0000000fULL;

	static struct {
		kernel_ulong_t val;
		const char *str;
		const char *fnames[8];
	} unshare_flags[] = {
		{ ARG_STR(0), { NULL } },
		{ 0xdeadcaf5,
			"CLONE_NEWTIME|CLONE_FS|CLONE_SIGHAND|CLONE_THREAD"
			"|CLONE_SYSVSEM|CLONE_NEWCGROUP|CLONE_NEWUTS"
			"|CLONE_NEWIPC|CLONE_NEWUSER|CLONE_NEWNET|0x80a8c075",
			{ NULL } },
		{ 0x00000080|0x00020000|0x02000000|0x04000000|0x08000000|0x20000000|0x40000000,
			"CLONE_NEWTIME|CLONE_NEWNS|CLONE_NEWCGROUP"
			"|CLONE_NEWUTS|CLONE_NEWIPC|CLONE_NEWPID"
			"|CLONE_NEWNET",
			{ "cgroup", "ipc", "mnt", "net", "pid", "time", "uts", NULL } },
		{ 0x00020000, "CLONE_NEWNS", { "mnt", NULL } },
		{ 0x2000000, "CLONE_NEWCGROUP", { "cgroup", NULL } },
		{ 0x10000000, "CLONE_NEWUSER", { "user", NULL } },
		{ ARG_STR(0x81f8f07f) " /* CLONE_??? */", { NULL } },
	};

	long rc = syscall(__NR_unshare, bogus_flags);
	printf("unshare(%#llx /* CLONE_??? */) = %s\n",
	       (unsigned long long) bogus_flags, sprintrc(rc));

#ifdef PRINT_NAMESPACE
	if (chdir("/proc/self/ns") != 0)
		perror_msg_and_skip("chdir");
#endif

	for (unsigned int i = 0; i < ARRAY_SIZE(unshare_flags); ++i) {
		rc = syscall(__NR_unshare, unshare_flags[i].val);
		printf("unshare(%s) = %s%s",
		       unshare_flags[i].str, sprintrc(rc), rc == 0? LINE_END: "\n");

#ifdef PRINT_NAMESPACE
		if (rc == 0) {
			unsigned int j;
			for (j = 0; unshare_flags[i].fnames[j]; j++) {
				char buf[PATH_MAX + 1];
				int n = readlink(unshare_flags[i].fnames[j], buf, sizeof(buf));
				if (n < 0)
					perror_msg_and_skip("readlink");
				else if ((size_t)n >= sizeof(buf))
					error_msg_and_skip("too large readlink result");
				buf[n] = '\0';
				if (j == 0)
					printf(" (%s", buf);
				else
					printf(", %s", buf);
			}
			printf("%s", j == 0? "\n": ")\n");
		}
#endif
	}

	puts("+++ exited with 0 +++");

	return 0;
}
