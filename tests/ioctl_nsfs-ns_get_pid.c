/*
 * Check decoding of NS_GET_{PID,TGID}_{FROM,IN}_PIDNS commands of ioctl syscall.
 *
 * Copyright (c) 2017-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include "pidns.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <linux/nsfs.h>

int
main(void)
{
	PIDNS_TEST_INIT;

#ifdef PIDNS_TRANSLATION
	pidns_print_leader();
	errno = EBADF;
	printf("ioctl(-1, NS_GET_USERNS)" RVAL_EBADF);
	fflush(NULL);
	ioctl(-1, NS_GET_USERNS);
#endif

	static const struct {
		unsigned int pt;
		enum {
			TRANS_RET,
			TRANS_ARG,
		} tr;
		unsigned int val;
		const char *str;
	} ns_get_pid_ops[] = {
		{ PT_TID,  TRANS_RET, ARG_STR(NS_GET_PID_FROM_PIDNS) },
		{ PT_TGID, TRANS_RET, ARG_STR(NS_GET_TGID_FROM_PIDNS) },
		{ PT_TID,  TRANS_ARG, ARG_STR(NS_GET_PID_IN_PIDNS) },
		{ PT_TGID, TRANS_ARG, ARG_STR(NS_GET_TGID_IN_PIDNS) },
	};

	for (size_t i = 0; i < ARRAY_SIZE(ns_get_pid_ops); ++i) {
		int pid = 0xfacefed1 + i;
		int rc = ioctl(-1, ns_get_pid_ops[i].val, pid);
		pidns_print_leader();
		printf("ioctl(-1, %s, %d) = %s\n",
		       ns_get_pid_ops[i].str, pid, sprintrc(rc));
	}

	static const char ns_pid_path[] = "/proc/self/ns/pid";
	int ns_pid_fd = open(ns_pid_path, O_RDONLY);
	if (ns_pid_fd < 0) {
		perror(ns_pid_path);
	} else {
		int pid = getpid();
		const char *pid_str = pidns_pid2str(PT_TGID);
		int tid = syscall(__NR_gettid);
		const char *tid_str = pidns_pid2str(PT_TID);
		for (size_t i = 0; i < ARRAY_SIZE(ns_get_pid_ops); ++i) {
			int id = ns_get_pid_ops[i].pt == PT_TID ? tid : pid;
			const char *id_str = ns_get_pid_ops[i].pt == PT_TID ?
					     tid_str : pid_str;
			int rc = ioctl(ns_pid_fd, ns_get_pid_ops[i].val, id);
			pidns_print_leader();
			printf("ioctl(%d, %s, %d%s) = %s%s\n",
			       ns_pid_fd, ns_get_pid_ops[i].str, id,
			       ns_get_pid_ops[i].tr == TRANS_ARG ? id_str : "",
			       sprintrc(rc),
			       (ns_get_pid_ops[i].tr == TRANS_RET
				&& rc != -1) ? id_str : "");
		}
		close(ns_pid_fd);
	}

	pidns_print_leader();
	puts("+++ exited with 0 +++");
	return 0;
}
