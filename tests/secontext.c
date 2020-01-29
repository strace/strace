/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#ifdef HAVE_SELINUX_RUNTIME

# include <assert.h>
# include <errno.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <selinux/selinux.h>

# include "xmalloc.h"

# define TEST_SECONTEXT
# include "secontext.h"

static char *
secontext_format(char *context, const char *fmt)
	ATTRIBUTE_FORMAT((printf, 2, 0)) ATTRIBUTE_MALLOC;

static char *
secontext_format(char *context, const char *fmt)
{
	int saved_errno = errno;
	char *res = context ? xasprintf(fmt, context) : xstrdup("");
	free(context);
	errno = saved_errno;
	return res;
}

# define FORMAT_SPACE_BEFORE(string)	secontext_format(string, " [%s]")
# define FORMAT_SPACE_AFTER(string)	secontext_format(string, "[%s] ")

static char *
strip_trailing_newlines(char *context)
{
	/*
	 * On the CI at least, the context may have a trailing \n,
	 * let's remove it just in case.
	 */
	size_t len = strlen(context);
	for (; len > 0; --len) {
		if (context[len - 1] != '\n')
			break;
	}
	context[len] = '\0';
	return context;
}

static char *
raw_secontext_full_file(const char *filename)
{
	int saved_errno = errno;
	char *full_secontext = NULL;
	char *secontext;

	if (getfilecon(filename, &secontext) >= 0) {
		full_secontext = strip_trailing_newlines(xstrdup(secontext));
		freecon(secontext);
	}
	errno = saved_errno;
	return full_secontext;
}

static char *
raw_secontext_short_file(const char *filename)
{
	int saved_errno = errno;

	char *ctx = raw_secontext_full_file(filename);
	if (ctx == NULL)
		return ctx;

	char *saveptr = NULL;
	const char *token;
	unsigned int i;

	char *ctx_copy = xstrdup(ctx);
	char *context = NULL;
	for (token = strtok_r(ctx_copy, ":", &saveptr), i = 0;
	     token; token = strtok_r(NULL, ":", &saveptr), i++) {
		if (i == 2) {
			context = xstrdup(token);
			break;
		}
	}
	if (context == NULL)
		context = xstrdup(ctx);
	free(ctx_copy);
	free(ctx);

	errno = saved_errno;
	return context;
}

static char *
raw_secontext_full_pid(pid_t pid)
{
	int saved_errno = errno;
	char *full_secontext = NULL;
	char *secontext;

	if (getpidcon(pid, &secontext) == 0) {
		full_secontext = strip_trailing_newlines(xstrdup(secontext));
		freecon(secontext);
	}
	errno = saved_errno;
	return full_secontext;
}

static char *
raw_secontext_short_pid(pid_t pid)
{
	int saved_errno = errno;

	char *ctx = raw_secontext_full_pid(pid);
	if (ctx == NULL)
		return ctx;

	char *saveptr = NULL;
	const char *token;
	int i;

	char *ctx_copy = xstrdup(ctx);
	char *context = NULL;
	for (token = strtok_r(ctx_copy, ":", &saveptr), i = 0;
	     token; token = strtok_r(NULL, ":", &saveptr), i++) {
		if (i == 2) {
			context = xstrdup(token);
			break;
		}
	}
	if (context == NULL)
		context = xstrdup(ctx);
	free(ctx_copy);
	free(ctx);

	errno = saved_errno;
	return context;
}

char *
secontext_full_file(const char *filename)
{
	return FORMAT_SPACE_BEFORE(raw_secontext_full_file(filename));
}

char *
secontext_full_pid(pid_t pid)
{
	return FORMAT_SPACE_AFTER(raw_secontext_full_pid(pid));
}

char *
secontext_short_file(const char *filename)
{
	return FORMAT_SPACE_BEFORE(raw_secontext_short_file(filename));
}

char *
secontext_short_pid(pid_t pid)
{
	return FORMAT_SPACE_AFTER(raw_secontext_short_pid(pid));
}

void
update_secontext_type(const char *file, const char *newtype)
{
	char *ctx = raw_secontext_full_file(file);
	if (ctx == NULL)
		return;

	char *saveptr = NULL;
	char *token;
	int field;
	char *split[4];

	for (token = strtok_r(ctx, ":", &saveptr), field = 0;
	     token; token = strtok_r(NULL, ":", &saveptr), field++) {
		assert(field < 4);
		split[field] = token;
	}
	assert(field == 4);

	char *newcontext = xasprintf("%s:%s:%s:%s", split[0], split[1],
				     newtype, split[3]);

	(void) setfilecon(file, newcontext);

	free(newcontext);
	free(ctx);
}

#endif /* HAVE_SELINUX_RUNTIME */
