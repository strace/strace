/*
 * Testing framework for PID namespace translation
 *
 * Copyright (c) 2020 Ákos Uzonyi <uzonyi.akos@gmail.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "tests.h"
#include "pidns.h"
#include "nsfs.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>
#include <sys/wait.h>
#include <linux/sched.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#ifndef CLONE_NEWUSER
# define CLONE_NEWUSER 0x10000000
#endif

#ifndef CLONE_NEWPID
# define CLONE_NEWPID 0x20000000
#endif

static bool pidns_translation = false;
static bool pidns_unshared = false;

/* Our PIDs in strace's namespace */
static pid_t pidns_strace_ids[PT_COUNT];

void
pidns_print_leader(void)
{
	if (pidns_translation)
		printf("%-5d ", pidns_strace_ids[PT_TID]);
}

const char *
pidns_pid2str(enum pid_type type)
{
	static const char format[] = " /* %d in strace's PID NS */";
	static char buf[PT_COUNT][sizeof(format) + sizeof(int) * 3];

	if (type < 0 || type >= PT_COUNT)
		return "";

	if (!pidns_unshared || !pidns_strace_ids[type])
		return "";

	snprintf(buf[type], sizeof(buf[type]), format, pidns_strace_ids[type]);
	return buf[type];
}

/**
 * This function is like fork, but does a few more things. It sets up the
 * child's PGID and SID according to the parameters. Also it fills the
 * pidns_strace_ids array in the child's memory with the PIDs of the child in
 * parent's PID namespace. In the parent it waits for the child to terminate
 * (but leaves the zombie to use it later as a process group). If the child
 * terminates with nonzero exit status, the test is failed.
 *
 * @param pgid     The process group the child should be moved to. It's expected
 *                 to be a PID of a zombie process (will be reaped). If
 *                 negative, leave the child in the process group of the parent.
 *                 If 0, move the process to its own process group.
 * @param new_sid  Whether child should be moved to a new session.
 */
static pid_t
pidns_fork(pid_t pgid, bool new_sid)
{
	int strace_ids_pipe[2];
	if (pipe(strace_ids_pipe) < 0)
		perror_msg_and_fail("pipe");

	fflush(stdout);
	pid_t pid = fork();
	if (pid < 0)
		perror_msg_and_fail("fork");

	if (!pid) {
		close(strace_ids_pipe[1]);

		ssize_t len = read(strace_ids_pipe[0], pidns_strace_ids,
				sizeof(pidns_strace_ids));
		if (len < 0)
			perror_msg_and_fail("read");
		if (len != sizeof(pidns_strace_ids))
			error_msg_and_fail("read returned < sizeof(pidns_strace_ids)");

		close(strace_ids_pipe[0]);

		if (pidns_strace_ids[PT_SID])
			setsid();

		return 0;
	}

	pidns_strace_ids[PT_TID] = pid;
	pidns_strace_ids[PT_TGID] = pid;
	pidns_strace_ids[PT_PGID] = 0;
	pidns_strace_ids[PT_SID] = 0;

	if (!pgid)
		pgid = pid;

	if (pgid > 0) {
		if (setpgid(pid, pgid) < 0)
			perror_msg_and_fail("setpgid");

		pidns_strace_ids[PT_PGID] = pgid;
	}

	/* Reap group leader to test PGID decoding */
	if (pgid > 0 && pgid != pid) {
		int ret = waitpid(pgid, NULL, WNOHANG);
		if (ret < 0)
			perror_msg_and_fail("wait");
		if (!ret)
			error_msg_and_fail("could not reap group leader");
	}

	if (new_sid) {
		pidns_strace_ids[PT_SID] = pid;
		pidns_strace_ids[PT_PGID] = pid;
	}

	ssize_t len = write(strace_ids_pipe[1], pidns_strace_ids,
	                     sizeof(pidns_strace_ids));
	if (len < 0)
		perror_msg_and_fail("write");
	if (len != sizeof(pidns_strace_ids))
		error_msg_and_fail("write returned < sizeof(pidns_strace_ids)");

	close(strace_ids_pipe[0]);
	close(strace_ids_pipe[1]);

	/* WNOWAIT: leave the zombie, to be able to use it as a process group */
	siginfo_t siginfo;
	if (waitid(P_PID, pid, &siginfo, WEXITED | WNOWAIT) < 0)
		perror_msg_and_fail("wait");
	if (siginfo.si_code != CLD_EXITED || siginfo.si_status)
		error_msg_and_fail("child terminated with nonzero exit status");

	return pid;
}

static void
create_init_process(void)
{
	int child_pipe[2];
	if (pipe(child_pipe) < 0)
		perror_msg_and_fail("pipe");

	pid_t pid = fork();
	if (pid < 0)
		perror_msg_and_fail("fork");

	if (!pid) {
		close(child_pipe[1]);
		if (read(child_pipe[0], &child_pipe[1], sizeof(int)) != 0)
			_exit(1);
		_exit(0);
	}

	close(child_pipe[0]);
}

void
check_ns_ioctl(void)
{
	int fd = open("/proc/self/ns/pid", O_RDONLY);
	if (fd < 0) {
		if (errno == ENOENT)
			perror_msg_and_skip("opening /proc/self/ns/pid");
		else
			perror_msg_and_fail("opening /proc/self/ns/pid");
	}

	int userns_fd = ioctl(fd, NS_GET_USERNS);
	if (userns_fd < 0) {
		if (errno == ENOTTY)
			error_msg_and_skip("NS_* ioctl commands are not "
			                   "supported by the kernel");
		else
			perror_msg_and_fail("ioctl(NS_GET_USERNS)");
	}

	close(userns_fd);
	close(fd);
}

void
pidns_test_init(void)
{
	pidns_translation = true;

	check_ns_ioctl();

	if (!pidns_fork(-1, false))
		return;

	/* Unshare user namespace too, so we do not need to be root */
	if (unshare(CLONE_NEWUSER | CLONE_NEWPID) < 0) {
		if (errno == EPERM)
			perror_msg_and_skip("unshare");

		perror_msg_and_fail("unshare");
	}

	pidns_unshared = true;

	create_init_process();

	if (!pidns_fork(-1, false))
		return;

	if (!pidns_fork(-1, true))
		return;

	pid_t pgid;
	if (!(pgid = pidns_fork(0, false)))
		return;

	if (!pidns_fork(pgid, false))
		return;

	exit(0);
}
