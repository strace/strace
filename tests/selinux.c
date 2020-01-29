#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <selinux/selinux.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "xmalloc.h"

/*
 * Contexts allocate memory which may be freed using a free() call.
 */

#define CONTEXT_FORMAT_SPACE_BEFORE(string)		\
	context_format(string, " [%s]")
#define CONTEXT_FORMAT_SPACE_AFTER(string)		\
	context_format(string, "[%s] ")
#define CONTEXT_FORMAT_SPACE_BEFORE_QUOTED(string)	\
	context_format(string, " \\[%s\\]")
#define CONTEXT_FORMAT_SPACE_AFTER_QUOTED(string)	\
	context_format(string, "\\[%s\\] ")

#ifdef PRINT_SECONTEXT_FULL

#define SELINUX_FILECONTEXT(filename)		\
	CONTEXT_FORMAT_SPACE_BEFORE(get_file_context_full(filename))
#define SELINUX_PIDCONTEXT(pid)			\
	CONTEXT_FORMAT_SPACE_AFTER(get_pid_context_full(pid))
#define SELINUX_FILECONTEXT_QUOTED(filename)	\
	CONTEXT_FORMAT_SPACE_BEFORE_QUOTED(get_file_context_full(filename))
#define SELINUX_PIDCONTEXT_QUOTED(pid)		\
	CONTEXT_FORMAT_SPACE_AFTER_QUOTED(get_pid_context_full(pid))

#else

#define SELINUX_FILECONTEXT(filename)		\
	CONTEXT_FORMAT_SPACE_BEFORE(get_file_context_short(filename))
#define SELINUX_PIDCONTEXT(pid)			\
	CONTEXT_FORMAT_SPACE_AFTER(get_pid_context_short(pid))
#define SELINUX_FILECONTEXT_QUOTED(filename)	\
	CONTEXT_FORMAT_SPACE_BEFORE_QUOTED(get_file_context_short(filename))
#define SELINUX_PIDCONTEXT_QUOTED(pid)		\
	CONTEXT_FORMAT_SPACE_AFTER_QUOTED(get_pid_context_short(pid))

#endif

#define SELINUX_MYCONTEXT()		SELINUX_PIDCONTEXT(getpid())
#define SELINUX_MYCONTEXT_QUOTED()	SELINUX_PIDCONTEXT_QUOTED(getpid())

char *
context_format(char *context, const char *fmt)
{
	int saved_errno = errno;
	char *res = context ? xasprintf(fmt, context) : xstrdup("");
	free(context);
	errno = saved_errno;
	return res;
}

char *
get_file_context_full(const char *filename)
{
	int saved_errno = errno;
	char *full_context = NULL;
	char *secontext;
	if (getfilecon(filename, &secontext) >= 0) {
		full_context = xstrdup(secontext);
		freecon(secontext);
	}
	errno = saved_errno;
	return full_context;
}

char *
get_file_context_short(const char *filename)
{
	char *ctx = get_file_context_full(filename);

	if (ctx == NULL)
		return ctx;

	int saved_errno = errno;

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
get_pid_context_full(pid_t pid)
{
	int saved_errno = errno;
	char *full_context = NULL;
	char *secontext;

	if (getpidcon(pid, &secontext) == 0) {
		full_context = xstrdup(secontext);
		freecon(secontext);
	}
	errno = saved_errno;
	return full_context;
}

char *
get_pid_context_short(pid_t pid)
{
	char *ctx = get_pid_context_full(pid);

	if (ctx == NULL)
		return ctx;

	int saved_errno = errno;

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

void
update_context_type(const char *file, const char *newtype)
{
	char *ctx = get_file_context_full(file);

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
