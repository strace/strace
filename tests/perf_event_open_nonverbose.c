/*
 * Check decoding of perf_event_open syscall.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "tests.h"
#include <asm/unistd.h>

#if defined(__NR_perf_event_open) && defined(HAVE_LINUX_PERF_EVENT_H)

# include <limits.h>
# include <stdio.h>
# include <unistd.h>

# include <linux/perf_event.h>

# include "xlat.h"
# include "xlat/perf_event_open_flags.h"

#if ULONG_MAX > UINT_MAX
#define LONG_STR_PREFIX "ffffffff"
#else
#define LONG_STR_PREFIX ""
#endif

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
