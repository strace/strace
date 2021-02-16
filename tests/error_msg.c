/*
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#define perror_msg_and_fail perror_msg_and_fail
#define error_msg_and_fail error_msg_and_fail

#include "tests.h"
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
perror_msg_and_fail(const char *fmt, ...)
{
	int err_no = errno;
	va_list p;

	va_start(p, fmt);
	vfprintf(stderr, fmt, p);
	if (err_no)
		fprintf(stderr, ": %s\n", strerror(err_no));
	else
		putc('\n', stderr);
	exit(1);
}

void
error_msg_and_fail(const char *fmt, ...)
{
	va_list p;

	va_start(p, fmt);
	vfprintf(stderr, fmt, p);
	putc('\n', stderr);
	exit(1);
}

void
error_msg_and_skip(const char *fmt, ...)
{
	va_list p;

	va_start(p, fmt);
	vfprintf(stderr, fmt, p);
	putc('\n', stderr);
	exit(77);
}

void
perror_msg_and_skip(const char *fmt, ...)
{
	int err_no = errno;
	va_list p;

	va_start(p, fmt);
	vfprintf(stderr, fmt, p);
	if (err_no)
		fprintf(stderr, ": %s\n", strerror(err_no));
	else
		putc('\n', stderr);
	exit(77);
}
