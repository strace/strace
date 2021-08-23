/*
 * Check that fault injection works properly.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
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

static int is_raw;

static void
invoke(int iter, int fail, int err, const char *errstr)
{
	static char buf[sizeof(int) * 3 + 3];
	static int try;
	const struct iovec io = {
		.iov_base = buf,
		.iov_len = sprintf(buf, "%d.", ++try)
	};
	int rc;

	if (!fail) {
		rc = write(exp_fd, io.iov_base, io.iov_len);
		if (rc != (int) io.iov_len)
			perror_msg_and_fail("iter %d: write", iter);
	}

	errno = 0;
	rc = writev(got_fd, &io, 1);

	if (fail) {
		if (!(rc == -1 && errno == err))
			perror_msg_and_fail("iter %d: expected errno %d"
					    ", got rc == %d, errno == %d",
					    iter, err, rc, errno);

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
			perror_msg_and_fail("iter %d: expected %d"
					    ", got rc == %d, errno == %d",
					    iter, (int) io.iov_len, rc, errno);

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

static char *
parse_next_uint(const char *str, int *val_ptr)
{
	char *endptr;
	long val;

	if (str == NULL) {
		*val_ptr = -1;
		return NULL;
	}

	errno = 0;
	val = strtol(str, &endptr, 0);

	if (errno) {
		perror_msg_and_fail("strtol");
	} else if (val < 0 || val > INT_MAX) {
		error_msg_and_fail("strtol(\"%s\") returned a value "
				   "out of 0..INT_MAX", str);
	}

	*val_ptr = val;
	return *endptr ? endptr : NULL;
}

static int
parse_err(const char *str, char **endptr, const char **errstr)
{
	int err = str ? strtol(str, endptr, 0) : DEFAULT_ERRNO;
	assert(err >= 0 && err < 4096);

	if (!err) {
		if (!*str)
			err = DEFAULT_ERRNO;
		else if (!strcasecmp(str, "EINVAL"))
			err = EINVAL;
		else
			err = ENOSYS;
	}

	if (errstr) {
		errno = err;
		*errstr = errno2name();
	}

	return err;
}

/*
 * Fault list is whitespace-separated list of iter[:err] speifications,
 * where iter is to grow monotonically.
 */
static const char *
parse_fault(const char *str, int *val, int *err, const char **errstr)
{
	const char *pos = parse_next_uint(str, val);

	if (pos == NULL)
		return NULL;
	if (pos[0] == ':') {
		char *endptr;
		*err = parse_err(pos + 1, &endptr, errstr);
		pos = endptr;
	} else {
		*err = DEFAULT_ERRNO;
		errno = *err;
		*errstr = errno2name();
	}

	return pos;
}

int
main(int argc, char *argv[])
{
	assert(argc == 12 || argc == 13);

	is_raw = !strcmp("raw", argv[1]);
	const char *errstr;
	int err = parse_err(argv[2], NULL, &errstr);
	int first = atoi(argv[3]);
	int last = atoi(argv[4]);
	int step = atoi(argv[5]);
	int iter = atoi(argv[6]);
	int num_procs = atoi(argv[7]);
	char *exp_prefix = argv[8];
	char *got_prefix = argv[9];
	char *out_prefix = argv[10];
	char *pid_prefix = argv[11];
	/* Overrides first/last/step calculation */
	const char *iter_list = argc > 12 && argv[12][0] ? argv[12] : NULL;
	int next_err = err;
	const char *next_errstr = errstr;
	int next_fail = -1;

	iter_list = parse_fault(iter_list, &next_fail, &next_err, &next_errstr);

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
			if (argc > 12 && argv[12][0]) {
				if (i == next_fail) {
					invoke(i, true, next_err, next_errstr);
					iter_list = parse_fault(iter_list,
								&next_fail,
								&next_err,
								&next_errstr);
					continue;
				}
			} else {
				if (last != 0) {
					--first;
					if (last != -1)
						--last;
					if (first == 0) {
						fail = 1;
						first = step;
					}
				}
			}
			invoke(i, fail, err, errstr);
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
