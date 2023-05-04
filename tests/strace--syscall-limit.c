/*
 * Test --syscall-limit option.
 *
 * Copyright (c) 2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#ifndef PRINT_VALID
# define PRINT_VALID 1
#endif
#ifndef PRINT_INVALID
# define PRINT_INVALID 1
#endif
#ifndef PRINT_STATS
# define PRINT_STATS 0
#endif
#ifndef UNLINKAT_CNT
# define UNLINKAT_CNT 1
#endif
#ifndef TOTAL_CNT
# define TOTAL_CNT 3
#endif

static int
write_status(const char *name, int value)
{
	FILE *fp = fopen(name, "w");
	if (!fp)
		perror_msg_and_fail("fopen: %s", name);
	if (fprintf(fp, "%d\n", value) <= 0)
		perror_msg_and_fail("fprintf: %s", name);
	if (fclose(fp))
		perror_msg_and_fail("fclose: %s", name);
	fflush(stdout);
	return value;
}

static const char valid_path[] = ".";
static const char invalid_path[] = "invalid.dir";

static void
test_chdir(int pid, bool print)
{
	if (chdir(valid_path))
		perror_msg_and_fail("chdir: %s", valid_path);

	if (print) {
#if PRINT_VALID
		printf("%-5u chdir(\"%s\") = 0\n", pid, valid_path);
#else /* !PRINT_VALID */
		if (chdir(invalid_path) == 0)
			error_msg_and_fail("chdir: %s", invalid_path);
# if PRINT_INVALID
		printf("%-5u chdir(\"%s\") = %s\n", pid, invalid_path, sprintrc(-1));
# endif
#endif /* PRINT_VALID */
	} else {
		if (chdir(invalid_path) == 0)
			error_msg_and_fail("chdir: %s", invalid_path);
	}
}

static void
test_rmdir(int pid, bool print)
{
#ifndef AT_FDCWD
# define AT_FDCWD -100
#endif
#ifndef AT_REMOVEDIR
# define AT_REMOVEDIR 0x200
#endif
	if (syscall(__NR_unlinkat, AT_FDCWD, "invalid.dir", AT_REMOVEDIR) == 0)
		error_msg_and_fail("unlinkat: %s", invalid_path);
#if PRINT_INVALID
	if (print)
		printf("%-5u unlinkat(AT_FDCWD, \"%s\", AT_REMOVEDIR) = %s\n",
		       pid, invalid_path, sprintrc(-1));
#endif
}

int
main(void)
{
	int pid = getpid();

	test_chdir(pid, /* print */ true);

	fflush(stdout);
	pid_t child = fork();
	if (child < 0)
		perror_msg_and_fail("fork");

	if (child == 0) {
		int pid = getpid();

		test_rmdir(pid, /* print */ true);
		test_chdir(pid, /* print */ true);

		/*
		 * the tracer is expected to detach at this point
		 * if TOTAL_CNT < 4.
		 */

		test_rmdir(pid, /* print */ TOTAL_CNT > 3);

		exit(0);
	}

	int status;
	while ((waitpid(child, &status, 0)) != child) {
		if (errno == EINTR)
			continue;
		perror_msg_and_fail("waitpid: %d", child);
	}

	/* the tracer is expected to detach at this point */

	test_chdir(pid, /* print */ false);

#if PRINT_STATS
# if defined MPERS_IS_m32
	printf("System call usage summary for 32 bit mode:\n");
# endif
# if defined MPERS_IS_mx32
	printf("System call usage summary for x32 mode:\n");
# endif
	printf("%9s %s\n", "calls", "syscall");
	printf("--------- ----------------\n");
	printf("%9d %s\n", 2, "chdir");
	printf("%9d %s\n", UNLINKAT_CNT, "unlinkat");
	printf("--------- ----------------\n");
	printf("%9d %s\n", TOTAL_CNT, "total");
#endif /* PRINT_STATS */

	return write_status("parent_status",
			    WIFEXITED(status) ? WEXITSTATUS(status) : 1);
}
