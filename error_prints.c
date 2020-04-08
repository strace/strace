/*
 * Copyright (c) 1999-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "error_prints.h"

#ifndef HAVE_PROGRAM_INVOCATION_NAME
extern char *program_invocation_name;
#endif

static void
verror_msg(int err_no, const char *fmt, va_list p)
{
	char *msg;

	fflush(NULL);

	/* We want to print entire message with single fprintf to ensure
	 * message integrity if stderr is shared with other programs.
	 * Thus we use vasprintf + single fprintf.
	 */
	msg = NULL;
	if (vasprintf(&msg, fmt, p) >= 0) {
		if (err_no)
			fprintf(stderr, "%s: %s: %s\n",
				program_invocation_name, msg, strerror(err_no));
		else
			fprintf(stderr, "%s: %s\n",
				program_invocation_name, msg);
		free(msg);
	} else {
		/* malloc in vasprintf failed, try it without malloc */
		fprintf(stderr, "%s: ", program_invocation_name);
		vfprintf(stderr, fmt, p);
		if (err_no)
			fprintf(stderr, ": %s\n", strerror(err_no));
		else
			putc('\n', stderr);
	}
	/* We don't switch stderr to buffered, thus fprintf(stderr)
	 * always flushes its output and this is not necessary: */
	/* fflush(stderr); */
}

void
error_msg(const char *fmt, ...)
{
	va_list p;
	va_start(p, fmt);
	verror_msg(0, fmt, p);
	va_end(p);
}

void
error_msg_and_die(const char *fmt, ...)
{
	va_list p;
	va_start(p, fmt);
	verror_msg(0, fmt, p);
	va_end(p);
	die();
}

void
error_msg_and_help(const char *fmt, ...)
{
	if (fmt != NULL) {
		va_list p;
		va_start(p, fmt);
		verror_msg(0, fmt, p);
		va_end(p);
	}
	fprintf(stderr, "Try '%s -h' for more information.\n",
		program_invocation_name);
	die();
}

void
perror_msg(const char *fmt, ...)
{
	va_list p;
	va_start(p, fmt);
	verror_msg(errno, fmt, p);
	va_end(p);
}

void
perror_msg_and_die(const char *fmt, ...)
{
	va_list p;
	va_start(p, fmt);
	verror_msg(errno, fmt, p);
	va_end(p);
	die();
}
