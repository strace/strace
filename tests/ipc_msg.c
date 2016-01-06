/*
 * Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>

static int id = -1;

static void
cleanup(void)
{
	msgctl(id, IPC_RMID, NULL);
	printf("msgctl\\(%d, (IPC_64\\|)?IPC_RMID, NULL\\) += 0\n", id);
	id = -1;
}

int
main(void)
{
	int rc;
	struct msqid_ds ds;

	id = msgget(IPC_PRIVATE, 0600);
	if (id < 0)
		perror_msg_and_skip("msgget");
	printf("msgget\\(IPC_PRIVATE, 0600\\) += %d\n", id);
	atexit(cleanup);

	if (msgctl(id, IPC_STAT, &ds))
		perror_msg_and_skip("msgctl IPC_STAT");
	printf("msgctl\\(%d, (IPC_64\\|)?IPC_STAT, \\{msg_perm=\\{uid=%u, gid=%u, "
		"mode=%#o, key=%u, cuid=%u, cgid=%u\\}, msg_stime=%u, msg_rtime=%u, "
		"msg_ctime=%u, msg_qnum=%u, msg_qbytes=%u, msg_lspid=%u, "
		"msg_lrpid=%u\\}\\) += 0\n",
		id, (unsigned) ds.msg_perm.uid, (unsigned) ds.msg_perm.gid,
		(unsigned) ds.msg_perm.mode, (unsigned) ds.msg_perm.__key,
		(unsigned) ds.msg_perm.cuid, (unsigned) ds.msg_perm.cgid,
		(unsigned) ds.msg_stime, (unsigned) ds.msg_rtime,
		(unsigned) ds.msg_ctime, (unsigned) ds.msg_qnum,
		(unsigned) ds.msg_qbytes, (unsigned) ds.msg_lspid,
		(unsigned) ds.msg_lrpid);

	int max = msgctl(0, MSG_INFO, &ds);
	if (max < 0)
		perror_msg_and_skip("msgctl MSG_INFO");
	printf("msgctl\\(0, (IPC_64\\|)?MSG_INFO, %p\\) += %d\n", &ds, max);

	rc = msgctl(id, MSG_STAT, &ds);
	if (rc != id) {
		/*
		 * In linux < v2.6.24-rc1 the first argument must be
		 * an index in the kernel's internal array.
		 */
		if (-1 != rc || EINVAL != errno)
			perror_msg_and_skip("msgctl MSG_STAT");
		printf("msgctl\\(%d, (IPC_64\\|)?MSG_STAT, %p\\) += -1 EINVAL \\(%m\\)\n", id, &ds);
	} else {
		printf("msgctl\\(%d, (IPC_64\\|)?MSG_STAT, %p\\) += %d\n", id, &ds, id);
	}

	return 0;
}
