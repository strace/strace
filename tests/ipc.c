/*
 * Check decoding of ipc syscall.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
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
