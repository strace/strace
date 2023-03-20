/*
 * Test -l/--syscall-limit options.
 *
 * Copyright (c) 2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#ifndef PRINT_VALID
# define PRINT_VALID 1
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
#else
		if (chdir(invalid_path) == 0)
			error_msg_and_fail("chdir: %s", invalid_path);
		printf("%-5u chdir(\"%s\") = %s\n", pid, invalid_path, sprintrc(-1));
#endif
	} else {
		if (chdir(invalid_path) == 0)
			error_msg_and_fail("chdir: %s", invalid_path);
	}
}

static void
test_rmdir(int pid, bool print)
{
	if (rmdir(invalid_path) == 0)
		error_msg_and_fail("rmdir: %s", invalid_path);
	if (print)
		printf("%-5u rmdir(\"%s\") = %s\n", pid, invalid_path, sprintrc(-1));
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

		/* the tracer is expected to detach at this point */

		test_rmdir(pid, /* print */ false);

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

	return write_status("parent_status",
			    WIFEXITED(status) ? WEXITSTATUS(status) : 1);
}
