/*
 * Check tampering notes.
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PATH_LEN 64

int
main(int argc, char *argv[])
{
	if (argc < 2)
		error_msg_and_skip("Insufficient arguments");

	const char *notes = NULL;
	const int notes_case = atoi(argv[1]);
	switch (notes_case) {
	case 1:
		notes = "\\(INJECTED: args\\)";
		break;
	case 2:
		notes = "\\(INJECTED: args\\) \\(DELAYED\\)";
		break;
	case 3:
		notes = "\\(INJECTED: args\\) \\(DELAYED\\)";
		break;
	case 4:
		notes = "\\(INJECTED: args, retval\\)";
		break;
	case 5:
		notes = "\\(INJECTED: args, retval\\) \\(DELAYED\\)";
		break;
	case 6:
		notes = "\\(INJECTED: args, retval\\) \\(DELAYED\\)";
		break;
	default:
		error_msg_and_fail("Unsupported argument: %s", argv[1]);
	}

	char *const p = tail_alloc(PATH_LEN);
	memset(p, '/', PATH_LEN);

	if (chdir(p)) {
		; /* Check the return value to pacify the compiler.  */
	}
	printf("chdir\\(.*\\) = .* %s\n", notes);

	char *const cur_dir = tail_alloc(PATH_MAX);
	long res = syscall(__NR_getcwd, cur_dir, PATH_MAX);
	if (res <= 0)
		perror_msg_and_fail("getcwd");
	printf("getcwd\\(.*\\) = .* %s\n", notes);

	return 0;
}
