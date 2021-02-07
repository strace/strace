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
	memset(chdir_buf, '/', PATH_MAX);
	char *const getcwd_buf = tail_alloc(PATH_MAX);

	/*
	 * regular poke on entering syscall
	 */
	char *p = chdir_buf;
	printf("chdir(\"%.*s\") = %s (INJECTED: args)\n",
	       PATH_MAX - 1, p, sprintrc(chdir(p)));

	/*
	 * poke at inaccessible address
	 */
	p += PATH_MAX;
	printf("chdir(%p) = %s\n", p, sprintrc(chdir(p)));
	if (exp_err) {
		fprintf(exp_err,
			".*: Failed to tamper with process %d: couldn't poke\n",
			pid);
	}

	/*
	 * poke at unaligned address,
	 * short read,
	 * if process_vm_writev is used: short write.
	 */
	--p;
	printf("chdir(%p) = %s", p, sprintrc(chdir(p)));
	if (*p != '/')
		printf(" (INJECTED: args)");
	printf("\n");
	if (exp_err) {
		fprintf(exp_err,
			".*: short read \\(1 < [[:digit:]]+\\) @%p: .*\n", p);
		if (*p != '/') {
			fprintf(exp_err,
				".*: pid:%d short write"
				" \\(1 < [[:digit:]]+\\) @%p\n",
				pid, p);
		}
	}

	/*
	 * poke at a properly aligned address,
	 * short read,
	 * if process_vm_writev is not used: short write,
	 * if process_vm_writev is used: short write is likely.
	 */
	p -= 7;
	printf("chdir(%p) = %s", p, sprintrc(chdir(p)));
	if (*p != '/')
		printf(" (INJECTED: args)");
	printf("\n");
	if (exp_err) {
		fprintf(exp_err,
			".*: short read \\(8 < [[:digit:]]+\\) @%p: .*\n", p);
		if (*p != '/') {
			fprintf(exp_err,
				".*: pid:%d short write"
				" \\(8 < [[:digit:]]+\\) @%p(: .*)?\n",
				pid, p);
		}
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
