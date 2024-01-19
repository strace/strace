/*
 * Copyright (c) 2016-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_symlink

# include <stdio.h>
# include <string.h>
# include <unistd.h>

int
main(int ac, char **av)
{
	static const char sample_str[] = "symlink.sample";
	static const char target_str[] = "symlink.target";

	TAIL_ALLOC_OBJECT_CONST_ARR(char, sample, sizeof(sample_str));
	TAIL_ALLOC_OBJECT_CONST_ARR(char, target, sizeof(target_str));
	TAIL_ALLOC_OBJECT_CONST_ARR(char, short_sample, sizeof(sample_str) - 1);
	TAIL_ALLOC_OBJECT_CONST_ARR(char, short_target, sizeof(target_str) - 1);

	long rc;

	memcpy(sample, sample_str, sizeof(sample_str));
	memcpy(target, target_str, sizeof(target_str));
	memcpy(short_sample, sample_str, sizeof(sample_str) - 1);
	memcpy(short_target, target_str, sizeof(target_str) - 1);

	rc = syscall(__NR_symlink, NULL, NULL);
# ifndef PATH_TRACING
	printf("symlink(NULL, NULL) = %s\n", sprintrc(rc));
# endif

	rc = syscall(__NR_symlink, NULL, sample);
	printf("symlink(NULL, \"%s\") = %s\n", sample, sprintrc(rc));

	rc = syscall(__NR_symlink, short_target, sample);
	printf("symlink(%p, \"%s\") = %s\n",
	       short_target, sample, sprintrc(rc));

	rc = syscall(__NR_symlink, target, short_sample);
# ifndef PATH_TRACING
	printf("symlink(\"%s\", %p) = %s\n",
	       target, short_sample, sprintrc(rc));
# endif

	rc = syscall(__NR_symlink, sample, av[0]);
# ifndef PATH_TRACING
	printf("symlink(\"%s\", \"%s\") = %s\n", sample, av[0], sprintrc(rc));
# endif

	rc = syscall(__NR_symlink, target, sample);
	printf("symlink(\"%s\", \"%s\") = %s\n", target, sample, sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_symlink")

#endif
