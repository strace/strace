/*
 * Check decoding of ipc syscall.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#if defined __NR_ipc && defined HAVE_LINUX_IPC_H

# include <errno.h>
# include <stdio.h>
# include <unistd.h>
# include <linux/ipc.h>

# ifndef SEMCTL
#  define SEMCTL 3
# endif
# ifndef MSGRCV
#  define MSGRCV 12
# endif

static int
ipc_call(const unsigned short version, const unsigned short call,
	 long a1, long a2, long a3, long a4, long a5)
{
	const unsigned long val =
		(unsigned long) 0xfacefeed00000000ULL |
		(unsigned int) version << 16 |
		call;

	return syscall(__NR_ipc, val, a1, a2, a3, a4, a5);
}

static int
ipc_call0(const unsigned short version, const unsigned short call)
{
	int rc = ipc_call(version, call, 0, 0, 0, 0, 0);
	int saved_errno = errno;
	printf("ipc(");
	if (version)
		printf("%hu<<16|", version);
	errno = saved_errno;
	printf("%hu, 0, 0, 0, 0%s) = %d %s (%m)\n", call,
# ifdef __s390__
	       "",
# else
	       ", 0",
# endif
	       rc, errno2name());
	return rc;
}

int
main(void)
{
	void *const efault = tail_alloc(1) + 1;

	int rc = ipc_call(0, SEMCTL, 0, 0, 0, (long) efault, 0);
	if (rc != -1 || EFAULT != errno)
		perror_msg_and_skip("ipc");
	printf("semctl(0, 0, IPC_RMID, %p) = -1 EFAULT (%m)\n", efault);

	unsigned short call;
	for (call = 0; call <= 40; call += 10) {
		ipc_call0(0, call);
		ipc_call0(42, call);
	}

	rc = ipc_call(42, SEMCTL, 0, 0, 0, (long) efault, 0);
	int test_version = EFAULT == errno;
	if (test_version)
		printf("semctl(0, 0, IPC_RMID, %p) = %d %s (%m)\n",
		       efault, rc, errno2name());
	else
		printf("ipc(42<<16|SEMCTL, 0, 0, 0, %p) = %d %s (%m)\n",
		       efault, rc, errno2name());

	if (test_version) {
		const int msqid = -2;
		const long msgsz = -3;
		const long msgtyp = 0;

		rc = ipc_call(1, MSGRCV,
			      msqid, msgsz, IPC_NOWAIT, (long) efault, msgtyp);
		printf("msgrcv(%d, %p, %lu, %ld, IPC_NOWAIT) = %d %s (%m)\n",
		       msqid, efault, msgsz, msgtyp, rc, errno2name());
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_ipc && HAVE_LINUX_IPC_H")

#endif
