/*
 * Helper program for strace-D.test
 *
 * Copyright (c) 2019-2020 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int
main(void)
{
	static const char status[] = "/proc/self/status";
	FILE *fp = fopen(status, "r");
	if (!fp)
		perror_msg_and_fail("fopen: %s", status);

	static const char prefix1[] = "PPid:";
	static const char prefix2[] = "TracerPid:";
	char *line = NULL;
	size_t n = 0;

	while (getline(&line, &n, fp) > 0) {
		if (strncmp(line, prefix1, sizeof(prefix1) - 1) == 0 ||
		    strncmp(line, prefix2, sizeof(prefix2) - 1) == 0)
			fputs(line, stdout);
	}

	if (!line)
		perror_msg_and_fail("getline");

	free(line);
	fclose(fp);

	return 0;
}
