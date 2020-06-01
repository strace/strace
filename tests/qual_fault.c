/*
 * Check that fault injection works properly.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/wait.h>

static int exp_fd;
static int got_fd;
static int out_fd;

#define DEFAULT_ERRNO ENOSYS

static const char *errstr;
static int is_raw, err, first, last, step, iter, try;

static void
invoke(int fail)
{
	static char buf[sizeof(int) * 3 + 3];
	const struct iovec io = {
		.iov_base = buf,
		.iov_len = sprintf(buf, "%d.", ++try)
	};
	int rc;

	if (!fail) {
		rc = write(exp_fd, io.iov_base, io.iov_len);
		if (rc != (int) io.iov_len)
			perror_msg_and_fail("write");
	}

	errno = 0;
	rc = writev(got_fd, &io, 1);

	if (fail) {
		if (!(rc == -1 && errno == err))
			perror_msg_and_fail("expected errno %d"
					    ", got rc == %d, errno == %d",
					    err, rc, errno);

		if (is_raw)
			tprintf("writev(%#x, %p, 0x1)"
				" = -1 %s (%m) (INJECTED)\n",
				got_fd, &io, errstr);
		else
			tprintf("writev(%d, [{iov_base=\"%s\", iov_len=%d}], 1)"
				" = -1 %s (%m) (INJECTED)\n",
				got_fd, buf, (int) io.iov_len, errstr);
	} else {
		if (rc != (int) io.iov_len)
			perror_msg_and_fail("expected %d"
					    ", got rc == %d, errno == %d",
					    (int) io.iov_len, rc, errno);

		if (is_raw)
			tprintf("writev(%#x, %p, 0x1) = %#x\n",
				got_fd, &io, rc);
		else
			tprintf("writev(%d, [{iov_base=\"%s\", iov_len=%d}], 1)"
				" = %d\n",
				got_fd, buf, (int) io.iov_len,
				(int) io.iov_len);
	}
}

static int
open_file(const char *prefix, int proc)
{
	static const int open_flags = O_WRONLY | O_TRUNC | O_CREAT;
	static char path[PATH_MAX + 1];

	snprintf(path, sizeof(path), "%s.%d", prefix, proc);

	int fd = open(path, open_flags, 0600);
	if (fd < 0)
		perror_msg_and_fail("open: %s", path);

	return fd;
}

int
main(int argc, char *argv[])
{
	assert(argc == 12);

	is_raw = !strcmp("raw", argv[1]);

	errstr = argv[2];
	err = atoi(errstr);
	assert(err >= 0);

	if (!err) {
		if (!*errstr)
			err = DEFAULT_ERRNO;
		else if (!strcasecmp(errstr, "EINVAL"))
			err = EINVAL;
		else
			err = ENOSYS;
	}

	errno = err;
	errstr = errno2name();

	first = atoi(argv[3]);
	last = atoi(argv[4]);
	step = atoi(argv[5]);
	iter = atoi(argv[6]);
	int num_procs = atoi(argv[7]);
	char *exp_prefix = argv[8];
	char *got_prefix = argv[9];
	char *out_prefix = argv[10];
	char *pid_prefix = argv[11];

	assert(first > 0);
	assert(step >= 0);
	assert(num_procs > 0);

	int proc;
	for (proc = 0; proc < num_procs; ++proc) {
		int ret = fork();

		if (ret < 0)
			perror_msg_and_fail("fork");

		if (ret > 0) {
			int pidfd = open_file(pid_prefix, proc);

			char pidstr[sizeof(ret) * 3];
			int len = snprintf(pidstr, sizeof(pidstr), "%d", ret);
			assert(len > 0 && len < (int) sizeof(pidstr));
			assert(write(pidfd, pidstr, len) == len);

			close(pidfd);

			continue;
		}

		tprintf("%s", "");

		exp_fd = open_file(exp_prefix, proc);
		got_fd = open_file(got_prefix, proc);
		out_fd = open_file(out_prefix, proc);

		/* This magic forces tprintf to write where we want it. */
		dup2(out_fd, 3);

		int i;
		for (i = 1; i <= iter; ++i) {
			int fail = 0;
			if (last != 0) {
				--first;
				if (last != -1)
					--last;
				if (first == 0) {
					fail = 1;
					first = step;
				}
			}
			invoke(fail);
		}

		tprintf("%s\n", "+++ exited with 0 +++");
		return 0;
	}

	for (proc = 0; proc < num_procs; ++proc) {
		int status;
		int ret = wait(&status);
		if (ret <= 0)
			perror_msg_and_fail("wait %d", proc);
		if (status)
			error_msg_and_fail("wait: pid=%d status=%d",
					   ret, status);
	}

	return 0;
}
