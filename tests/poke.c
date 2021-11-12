/*
 * Check poke injection.
 *
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int
main(int argc, char *argv[])
{
	FILE *exp_err = NULL;

	if (argc > 1) {
		exp_err = fopen(argv[1], "w");
		if (!exp_err)
			perror_msg_and_fail("fopen: %s", argv[1]);
		setvbuf(exp_err, NULL, _IONBF, 0);
	}

	pid_t pid = getpid();
	char *const chdir_buf = tail_alloc(PATH_MAX);
	char *const getcwd_buf = tail_alloc(PATH_MAX);
	char *p;

	/*
	 * regular poke at a properly aligned address on entering syscall
	 */
	memset(chdir_buf, '/', PATH_MAX);
	p = chdir_buf;
	printf("chdir(\"%.*s\") = %s (INJECTED: args)\n",
	       PATH_MAX - 1, p, sprintrc(chdir(p)));

	/*
	 * regular poke at an unaligned address on entering syscall
	 */
	memset(chdir_buf, '/', PATH_MAX);
	chdir_buf[PATH_MAX - 1] = '\0';
	p = chdir_buf + 1;
	printf("chdir(\"%.*s\") = %s (INJECTED: args)\n",
	       PATH_MAX - 2, p, sprintrc(chdir(p)));
	if (chdir_buf[0] != '/')
		error_msg_and_fail("failed to poke at unaligned address"
				   " %p properly", p);

	/*
	 * poke at an inaccessible but properly aligned address
	 */
	p = chdir_buf + PATH_MAX;
	printf("chdir(%p) = %s\n", p, sprintrc(chdir(p)));
	if (exp_err) {
		fprintf(exp_err,
			".*: Failed to tamper with process %d: couldn't poke\n",
			pid);
	}

	/*
	 * poke at an inaccessible unaligned address
	 */
	++p;
	printf("chdir(%p) = %s\n", p, sprintrc(chdir(p)));
	if (exp_err) {
		fprintf(exp_err,
			".*: Failed to tamper with process %d: couldn't poke\n",
			pid);
	}

	/*
	 * poke at a partially accessible unaligned address
	 */
	memset(chdir_buf, '/', PATH_MAX);
	p = chdir_buf + PATH_MAX - 1;
	printf("chdir(%p) = %s (INJECTED: args)\n", p, sprintrc(chdir(p)));
	if (exp_err) {
		fprintf(exp_err,
			".*: pid:%d short write"
			" \\(1 < [[:digit:]]+\\) @%p(: .*)?\n",
			pid, p);
		fprintf(exp_err,
			".*: short read \\(1 < [[:digit:]]+\\) @%p: .*\n", p);
	}

	/*
	 * poke at a partially accessible properly aligned address
	 */
	memset(chdir_buf, '/', PATH_MAX);
	p -= 7;
	printf("chdir(%p) = %s (INJECTED: args)\n", p, sprintrc(chdir(p)));
	if (exp_err) {
		fprintf(exp_err,
			".*: pid:%d short write"
			" \\(8 < [[:digit:]]+\\) @%p(: .*)?\n",
			pid, p);
		fprintf(exp_err,
			".*: short read \\(8 < [[:digit:]]+\\) @%p: .*\n", p);
	}

	/*
	 * regular poke on exiting syscall
	 */
	p = getcwd_buf;
	long res = syscall(__NR_getcwd, p, PATH_MAX);
	if (res <= 0)
		perror_msg_and_fail("getcwd");

	printf("getcwd(");
	print_quoted_string(p);
	printf(", %u) = %ld (INJECTED: args)\n", PATH_MAX, res);

	if (exp_err)
		fclose(exp_err);

	puts("+++ exited with 0 +++");
	return 0;
}
