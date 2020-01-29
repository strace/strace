/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "xmalloc.h"
#include <unistd.h>

#if defined TEST_SECONTEXT && defined HAVE_SELINUX_RUNTIME

void update_secontext_type(const char *file, const char *newtype);

# ifdef PRINT_SECONTEXT_FULL

char *secontext_full_file(const char *) ATTRIBUTE_MALLOC;
char *secontext_full_pid(pid_t) ATTRIBUTE_MALLOC;

#  define SECONTEXT_FILE(filename)	secontext_full_file(filename)
#  define SECONTEXT_PID(pid)		secontext_full_pid(pid)

# else

char *secontext_short_file(const char *) ATTRIBUTE_MALLOC;
char *secontext_short_pid(pid_t) ATTRIBUTE_MALLOC;

#  define SECONTEXT_FILE(filename)	secontext_short_file(filename)
#  define SECONTEXT_PID(pid)		secontext_short_pid(pid)

# endif

#else

static inline void
update_secontext_type(const char *file, const char *newtype)
{
}

# define SECONTEXT_FILE(filename)		xstrdup("")
# define SECONTEXT_PID(pid)			xstrdup("")

#endif

#define SECONTEXT_PID_MY()		SECONTEXT_PID(getpid())
