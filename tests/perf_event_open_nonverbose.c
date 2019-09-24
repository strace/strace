/*
 * Check decoding of perf_event_open syscall.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#if defined(__NR_perf_event_open) && defined(HAVE_LINUX_PERF_EVENT_H)

# include <limits.h>
# include <stdio.h>
# include <unistd.h>

# include <linux/perf_event.h>

# include "xlat.h"
# include "xlat/perf_event_open_flags.h"

# if ULONG_MAX > UINT_MAX
#  define LONG_STR_PREFIX "ffffffff"
# else
#  define LONG_STR_PREFIX ""
# endif

static const char *printaddr(void *ptr)
{
	static char buf[sizeof("0x") + sizeof(void *) * 2];

	if (ptr == NULL)
		return "NULL";

	snprintf(buf, sizeof(buf), "%#lx", (unsigned long)ptr);

	return buf;
}

int
main(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(struct perf_event_attr, attr);

	attr->type = PERF_TYPE_HARDWARE;
	attr->size = sizeof(*attr);

	struct {
		struct perf_event_attr *attr;
		pid_t pid;
		int cpu;
		int group_fd;
		unsigned long flags;
		const char *flags_str;
	} args[] = {
		{ NULL,     0xfacef00d, 0xbadabba7, -1,
			(unsigned long) 0xFFFFFFFFFFFFFFFFLLU,
			"PERF_FLAG_FD_NO_GROUP|PERF_FLAG_FD_OUTPUT|"
			"PERF_FLAG_PID_CGROUP|PERF_FLAG_FD_CLOEXEC|"
			"0x" LONG_STR_PREFIX "fffffff0"
			},
		{ attr + 1, 0,          0,          0,
			0, "0" },
		{ attr,     -1,         -1,         1,
			PERF_FLAG_FD_CLOEXEC, "PERF_FLAG_FD_CLOEXEC" },
		{ attr - 1, -100,       100,        0xface1e55,
			PERF_FLAG_FD_NO_GROUP | PERF_FLAG_FD_OUTPUT |
			PERF_FLAG_PID_CGROUP | PERF_FLAG_FD_CLOEXEC,
			"PERF_FLAG_FD_NO_GROUP|PERF_FLAG_FD_OUTPUT|"
			"PERF_FLAG_PID_CGROUP|PERF_FLAG_FD_CLOEXEC" },
	};
	size_t i;
	int rc;

	for (i = 0; i < ARRAY_SIZE(args); i++) {
		rc = syscall(__NR_perf_event_open, args[i].attr, args[i].pid,
			args[i].cpu, args[i].group_fd, args[i].flags);
		printf("perf_event_open(%s, %d, %d, %d, %s) = %s\n",
			printaddr(args[i].attr), args[i].pid, args[i].cpu,
			args[i].group_fd, args[i].flags_str, sprintrc(rc));
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_perf_event_open && HAVE_LINUX_PERF_EVENT_H");

#endif
