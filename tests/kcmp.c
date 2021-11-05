/*
 * Check decoding of kcmp syscall.
 *
 * Copyright (c) 2016-2017 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include "pidns.h"

#include <fcntl.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/kcmp.h>

#ifndef SKIP_IF_PROC_IS_UNAVAILABLE
# define SKIP_IF_PROC_IS_UNAVAILABLE
#endif

#ifndef VERBOSE_FD
# define VERBOSE_FD 0
#endif

static const kernel_ulong_t kcmp_max_type = KCMP_EPOLL_TFD;

static const char null_path[] = "/dev/null";
static const char zero_path[] = "/dev/zero";

#define NULL_FD 23
#define ZERO_FD 42

static void
printpidfd(const char *prefix, pid_t pid, unsigned fd)
{
	const char *path = NULL;

#if VERBOSE_FD
	if (pid == getpid()) {
		switch (fd)
		{
		case NULL_FD:
			path = null_path;
			break;
		case ZERO_FD:
			path = zero_path;
			break;
		}
	}
#endif

	if (path)
		printf("%s%d<%s>", prefix, fd, path);
	else
		printf("%s%d", prefix, fd);
}

/*
 * Last argument is optional and is used as follows:
 *  * When type is KCMP_EPOLL_TFD, it signalises whether idx2 is a valid
 *    pointer.
 */
static void
do_kcmp(kernel_ulong_t pid1, kernel_ulong_t pid2, kernel_ulong_t type,
	const char *type_str, kernel_ulong_t idx1, kernel_ulong_t idx2, ...)
{
	long rc;
	const char *errstr;

	rc = syscall(__NR_kcmp, pid1, pid2, type, idx1, idx2);
	errstr = sprintrc(rc);

	const char *pid_str = pidns_pid2str(PT_TGID);
	pidns_print_leader();
	printf("kcmp(%d%s, %d%s, ",
		(int) pid1, (int) pid1 == getpid() ? pid_str : "",
		(int) pid2, (int) pid2 == getpid() ? pid_str : "");

	if (type_str)
		printf("%s", type_str);
	else
		printf("%#x /* KCMP_??? */", (int) type);

	if (type == KCMP_FILE) {
		printpidfd(", ", pid1, idx1);
		printpidfd(", ", pid2, idx2);
	} else if (type == KCMP_EPOLL_TFD) {
		va_list ap;
		int valid_ptr;

		va_start(ap, idx2);
		valid_ptr = va_arg(ap, int);
		va_end(ap);

		printpidfd(", ", pid1, idx1);
		printf(", ");

		if (valid_ptr) {
			struct kcmp_epoll_slot *slot =
				(struct kcmp_epoll_slot *) (uintptr_t) idx2;

			printpidfd("{efd=", pid2, slot->efd);
			printpidfd(", tfd=", pid2, slot->tfd);
			printf(", toff=%llu}", (unsigned long long) slot->toff);
		} else {
			if (idx2)
				printf("%#llx", (unsigned long long) idx2);
			else
				printf("NULL");
		}
	} else if (type > kcmp_max_type) {
		printf(", %#llx, %#llx",
		       (unsigned long long) idx1, (unsigned long long) idx2);
	}

	printf(") = %s\n", errstr);
}

int
main(void)
{
	SKIP_IF_PROC_IS_UNAVAILABLE;
	PIDNS_TEST_INIT;

	static const kernel_ulong_t bogus_pid1 =
		(kernel_ulong_t) 0xdeadca75face1057ULL;
	static const kernel_ulong_t bogus_pid2 =
		(kernel_ulong_t) 0xdefaced1defaced2ULL;
	static const kernel_ulong_t bogus_type =
		(kernel_ulong_t) 0xbadc0dedda7adeadULL;
	static const kernel_ulong_t bogus_idx1 =
		(kernel_ulong_t) 0xdec0ded3dec0ded4ULL;
	static const kernel_ulong_t bogus_idx2 =
		(kernel_ulong_t) 0xba5e1e55deadc0deULL;
	static const struct kcmp_epoll_slot slot_data[] = {
		{ 0xdeadc0de, 0xfacef157, 0xbadc0ded },
		{ NULL_FD, ZERO_FD, 0 },
		{ 0, 0, 0 },
	};
	static kernel_ulong_t ptr_check =
		F8ILL_KULONG_SUPPORTED ? F8ILL_KULONG_MASK : 0;

	int fd;
	TAIL_ALLOC_OBJECT_CONST_PTR(struct kcmp_epoll_slot, slot);

	/* Open some files to test printpidfd */
	fd = open(null_path, O_RDONLY);
	if (fd < 0)
		perror_msg_and_fail("open(\"%s\")", null_path);
	if (fd != NULL_FD) {
		if (dup2(fd, NULL_FD) < 0)
			perror_msg_and_fail("dup2(fd, NULL_FD)");
		close(fd);
	}

	fd = open(zero_path, O_RDONLY);
	if (fd < 0)
		perror_msg_and_fail("open(\"%s\")", zero_path);
	if (fd != ZERO_FD) {
		if (dup2(fd, ZERO_FD) < 0)
			perror_msg_and_fail("dup2(fd, ZERO_FD)");
		close(fd);
	}

	close(0);

	/* Invalid values */
	do_kcmp(bogus_pid1, bogus_pid2, bogus_type, NULL, bogus_idx1,
		bogus_idx2);
	do_kcmp(F8ILL_KULONG_MASK, F8ILL_KULONG_MASK, kcmp_max_type + 1, NULL,
		0, 0);

	/* KCMP_FILE is the only type which has additional args */
	do_kcmp(3141592653U, 2718281828U, ARG_STR(KCMP_FILE), bogus_idx1,
		bogus_idx2);
	do_kcmp(getpid(), getpid(), ARG_STR(KCMP_FILE), NULL_FD, ZERO_FD);

	/* Types without additional args */
	do_kcmp(-1, -1, ARG_STR(KCMP_VM), bogus_idx1, bogus_idx2);
	do_kcmp(-1, -1, ARG_STR(KCMP_FILES), bogus_idx1, bogus_idx2);
	do_kcmp(-1, -1, ARG_STR(KCMP_FS), bogus_idx1, bogus_idx2);
	do_kcmp(-1, -1, ARG_STR(KCMP_SIGHAND), bogus_idx1, bogus_idx2);
	do_kcmp(-1, -1, ARG_STR(KCMP_IO), bogus_idx1, bogus_idx2);
	do_kcmp(-1, -1, ARG_STR(KCMP_SYSVSEM), bogus_idx1, bogus_idx2);

	/* KCMP_EPOLL_TFD checks */
	do_kcmp(-1, -1, ARG_STR(KCMP_EPOLL_TFD),
		F8ILL_KULONG_MASK | 2718281828U, ptr_check, 0);
	do_kcmp(-1, -1, ARG_STR(KCMP_EPOLL_TFD),
		3141592653U, (uintptr_t) slot + 1, 0);

	for (unsigned int i = 0; i < ARRAY_SIZE(slot_data); ++i) {
		memcpy(slot, slot_data + i, sizeof(*slot));

		do_kcmp(getpid(), -1, ARG_STR(KCMP_EPOLL_TFD), NULL_FD,
			(uintptr_t) slot, 1);
	}

	pidns_print_leader();
	puts("+++ exited with 0 +++");

	return 0;
}
