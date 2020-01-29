/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "largefile_wrappers.h"
#include "xstring.h"

static unsigned int
get_mnt_ns_of_pid(int pid)
{
	unsigned int ns = 0;
	char path[PATH_MAX + 1];
	xsprintf(path, "/proc/%s/ns/mnt", pid_to_str(pid));

	int fd = open_file(path, O_RDONLY);
	if (fd >= 0) {
		strace_stat_t st;
		if (fstat_fd(fd, &st) == 0)
			ns = st.st_ino;
		close(fd);
	}

	return ns;
}

/**
 * Returns the MOUNT namespace of the tracee
 */
unsigned int
get_mnt_ns(struct tcb *tcp)
{
	if (tcp->mnt_ns)
		return tcp->mnt_ns;

	int proc_pid = 0;
	translate_pid(NULL, tcp->pid, PT_TID, &proc_pid);

	if (proc_pid)
		tcp->mnt_ns = get_mnt_ns_of_pid(proc_pid);

	return tcp->mnt_ns;
}

/**
 * Returns the MOUNT namespace of strace
 */
unsigned int
get_our_mnt_ns(void)
{
	static unsigned int our_ns = 0;
	static bool our_ns_initialised = false;

	if (!our_ns_initialised) {
		our_ns = get_mnt_ns_of_pid(getpid());
		our_ns_initialised = true;
	}

	return our_ns;
}
