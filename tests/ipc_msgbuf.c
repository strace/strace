/*
 * Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "scno.h"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>

#define text_string "STRACE_STRING"
#define msgsz sizeof(text_string)

static int msqid = -1;

#if XLAT_RAW
# define str_ipc_creat "0x200"
# define str_ipc_private "0"
# define str_ipc_rmid "0"
# define str_ipc_64 "0x100"
#elif XLAT_VERBOSE
# define str_ipc_creat "0x200 /\\* IPC_CREAT \\*/"
# define str_ipc_private "0 /\\* IPC_PRIVATE \\*/"
# define str_ipc_rmid "0 /\\* IPC_RMID \\*/"
# define str_ipc_64 "0x100 /\\* IPC_64 \\*/"
#else
# define str_ipc_creat "IPC_CREAT"
# define str_ipc_private "IPC_PRIVATE"
# define str_ipc_rmid "IPC_RMID"
# define str_ipc_64 "IPC_64"
#endif

static int
cleanup(void)
{
	if (msqid != -1) {
		int rc = msgctl(msqid, IPC_RMID, 0);
		printf("msgctl\\(%d, (%s\\|)?%s, NULL\\) = 0\n",
		       msqid, str_ipc_64, str_ipc_rmid);
		msqid = -1;
		if (rc == -1)
			return 77;
		puts("\\+\\+\\+ exited with 0 \\+\\+\\+");
	}
	return 0;
}

static int
sys_msgrcv(int msqid, void *msgp, size_t sz, kernel_long_t msgtyp,
	   int msgflg)
{
#if defined __x86_64__ && defined __ILP32__
	return syscall(__NR_msgrcv, msqid, msgp, sz, msgtyp, msgflg);
#else
	return msgrcv(msqid, msgp, sz, msgtyp, msgflg);
#endif
}

int
main(void)
{
	/* mtype has to be positive */
	const kernel_long_t mtype = (kernel_long_t) 0x7facefed5adc0dedULL;
	struct {
		kernel_long_t mtype;
		char mtext[msgsz];
	} msg = {
		.mtype = mtype,
		.mtext = text_string
	};
	msqid = msgget(IPC_PRIVATE, IPC_CREAT | S_IRWXU);
	if (msqid == -1)
		perror_msg_and_skip("msgget");
	printf("msgget\\(%s, %s\\|0700\\) = %d\n",
	       str_ipc_private, str_ipc_creat, msqid);

	typedef void (*atexit_func)(void);
	atexit((atexit_func) cleanup);

	printf("msgsnd\\(%d, \\{mtype=%lld, mtext=\"" text_string
	       "\\\\0\"\\}, 14, 0\\) = 0\n",
	       msqid, (long long) mtype);
	if (msgsnd(msqid, &msg, msgsz, 0) == -1)
		perror_msg_and_skip("msgsnd");

	if (sys_msgrcv(msqid, &msg, msgsz, -mtype, 0) != msgsz)
		perror_msg_and_skip("msgrcv");
	printf("msgrcv\\(%d, \\{mtype=%lld, mtext=\"" text_string
	       "\\\\0\"\\}, 14, %lld, 0\\) = 14\n",
	       msqid, (long long) mtype, -(long long) mtype);

	return cleanup();
}
