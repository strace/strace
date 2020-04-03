/*
 * Check decoding of prctl PR_GET_NAME/PR_SET_NAME operations.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <sys/prctl.h>

#if defined PR_GET_NAME && defined PR_SET_NAME

# include <stdio.h>
# include <string.h>
# include <unistd.h>

int
main(void)
{
	static const char str[] = "0123456789abcdef";
	static const int len = sizeof(str) - 1;
	char *name = tail_memdup(str, sizeof(str));
	int i;
	int rc;

	rc = prctl(PR_SET_NAME, NULL);
	printf("prctl(PR_SET_NAME, NULL) = %s\n", sprintrc(rc));

	for (i = 0; i <= len; ++i) {
		rc = prctl(PR_SET_NAME, name + len - i);
		printf("prctl(PR_SET_NAME, \"%.*s\"%s) = %s\n",
		       i < len - 1 ? i : len - 1,
		       str + len - i,
		       i < len - 1 ? "" : "...",
		       sprintrc(rc));
	}

	*name = -1;
	++name;
	memcpy(name, str, len);

	for (i = 0; i <= len; ++i) {
		rc = prctl(PR_SET_NAME, name + len - i);
		if (i < len - 1)
			printf("prctl(PR_SET_NAME, %p) = %s\n",
			       name + len - i, sprintrc(rc));
		else
			printf("prctl(PR_SET_NAME, \"%.*s\"...) = %s\n",
			       len - 1, str + len - i, sprintrc(rc));
	}

	rc = prctl(PR_GET_NAME, NULL);
	printf("prctl(PR_GET_NAME, NULL) = %s\n", sprintrc(rc));

	for (i = 0; i < len; ++i) {
		rc = prctl(PR_GET_NAME, name + len - i);
		printf("prctl(PR_GET_NAME, %p) = %s\n",
		       name + len - i, sprintrc(rc));
	}

	rc = prctl(PR_GET_NAME, name);
	if (rc)
		printf("prctl(PR_GET_NAME, %p) = %s\n",
		       name, sprintrc(rc));
	else
		printf("prctl(PR_GET_NAME, \"%.*s\") = %s\n",
		       len - 1, name, sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("PR_GET_NAME && PR_SET_NAME")

#endif
