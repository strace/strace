/*
 * Check success injection.
 *
 * Copyright (c) 2017 Elvira Khabirova <lineprinter0@gmail.com>
 * Copyright (c) 2017-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_chdir

# include <assert.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/stat.h>

int
main(int argc, char *argv[])
{
	assert(argc == 2);

	static const char dir[] = "..";
	struct stat before, after;

	if (stat(".", &before))
		perror_msg_and_fail("stat");

	long rval = syscall(__NR_chdir, dir);

	if (stat(".", &after))
		perror_msg_and_fail("stat");

	if (before.st_dev != after.st_dev || before.st_ino != after.st_ino)
		error_msg_and_fail("syscall succeeded");
	if (atol(argv[1]) != rval)
		error_msg_and_fail("expected retval %s, got retval %ld",
				   argv[1], rval);

	printf("chdir(\"%s\") = %ld (INJECTED)\n", dir, rval);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_chdir")

#endif
