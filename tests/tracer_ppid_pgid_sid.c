/*
 * Helper program for strace-DDD.test
 *
 * Copyright (c) 2019-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "xmalloc.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int
fetch_tracer_pid(const char *str)
{
	for (; isspace(*str); ++str)
		;
	return atoi(str);
}

static int
get_tracer_pid(void)
{
	static const char status[] = "/proc/self/status";
	FILE *fp = fopen(status, "r");
	if (!fp)
		perror_msg_and_fail("fopen: %s", status);

	static const char prefix[] = "TracerPid:";
	const size_t prefix_len = sizeof(prefix) - 1;
	const char *str = NULL;
	char *line = NULL;
	size_t n = 0;

	while (getline(&line, &n, fp) > 0) {
		if (strncmp(line, prefix, prefix_len) == 0) {
			str = line + prefix_len;
			break;
		}
	}
	if (!str && !line)
		perror_msg_and_fail("getline");

	int pid = str ? fetch_tracer_pid(str) : 0;
	free(line);
	fclose(fp);

	return pid;
}

static void
get_ppid_pgid_sid(int pid, int *ppid, int *pgid, int *sid)
{
	char *stat = xasprintf("/proc/%d/stat", pid);
	FILE *fp = fopen(stat, "r");
	if (!fp)
		perror_msg_and_fail("fopen: %s", stat);
	char buf[4096];
	if (!fgets(buf, sizeof(buf), fp))
		perror_msg_and_fail("fgets: %s", stat);

	fclose(fp);

	const char *p = strrchr(buf, ')');
	if (!p)
		error_msg_and_fail("%s: parenthesis not found", stat);
	++p;

	if (sscanf(p, " %*c %d %d %d", ppid, pgid, sid) != 3)
		error_msg_and_fail("%s: sscanf failed", stat);
}

int
main(void)
{
	int tracer_pid = get_tracer_pid();
	if (tracer_pid < 0)
		error_msg_and_fail("tracer_pid = %d", tracer_pid);

	int ppid = 0, pgid = 0, sid = 0;
	get_ppid_pgid_sid(tracer_pid, &ppid, &pgid, &sid);
	printf("%d %d %d %d\n", tracer_pid, ppid, pgid, sid);

	return 0;
}
