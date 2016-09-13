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

#include "xlat.h"
#include "xlat/resource_flags.h"

/*
 * Before glibc-2.22-122-gbe48165, ppc64 code tried to retrieve data
 * provided in third argument of msgctl call (in case of IPC_SET cmd)
 * which led to segmentation fault.
 */
#undef TEST_MSGCTL_BOGUS_ADDR
#if defined __GLIBC__ && defined POWERPC64
# if !(defined __GLIBC_MINOR__) \
   || ((__GLIBC__ << 16) + __GLIBC_MINOR__ < (2 << 16) + 23)
#  define TEST_MSGCTL_BOGUS_ADDR 0
# endif
#endif /* __GLIBC__ && POWERPC64 */

#ifndef TEST_MSGCTL_BOGUS_ADDR
# define TEST_MSGCTL_BOGUS_ADDR 1
#endif

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
	static const key_t private_key =
		(key_t) (0xffffffff00000000ULL | IPC_PRIVATE);
	static const key_t bogus_key = (key_t) 0xeca86420fdb97531ULL;
	static const int bogus_msgid = 0xfdb97531;
	static const int bogus_cmd = 0xdeadbeef;
#if TEST_MSGCTL_BOGUS_ADDR
	static void * const bogus_addr = (void *) -1L;
#endif
	static const int bogus_flags = 0xface1e55 & ~IPC_CREAT;

	int rc;
	struct msqid_ds ds;

	rc = msgget(bogus_key, bogus_flags);
	printf("msgget\\(%#llx, %s%s%s%#x\\|%#04o\\) += %s\n",
	       zero_extend_signed_to_ull(bogus_key),
	       IPC_CREAT & bogus_flags ? "IPC_CREAT\\|" : "",
	       IPC_EXCL & bogus_flags ? "IPC_EXCL\\|" : "",
	       IPC_NOWAIT & bogus_flags ? "IPC_NOWAIT\\|" : "",
	       bogus_flags & ~(0777 | IPC_CREAT | IPC_EXCL | IPC_NOWAIT),
	       bogus_flags & 0777, sprintrc_grep(rc));

	id = msgget(private_key, 0600);
	if (id < 0)
		perror_msg_and_skip("msgget");
	printf("msgget\\(IPC_PRIVATE, 0600\\) += %d\n", id);
	atexit(cleanup);

	rc = msgctl(bogus_msgid, bogus_cmd, NULL);
	printf("msgctl\\(%d, (IPC_64\\|)?%#x /\\* MSG_\\?\\?\\? \\*/, NULL\\)"
	       " += %s\n", bogus_msgid, bogus_cmd, sprintrc_grep(rc));

#if TEST_MSGCTL_BOGUS_ADDR
	rc = msgctl(bogus_msgid, IPC_SET, bogus_addr);
	printf("msgctl\\(%d, (IPC_64\\|)?IPC_SET, %p\\) += %s\n",
	       bogus_msgid, bogus_addr, sprintrc_grep(rc));
#endif

	if (msgctl(id, IPC_STAT, &ds))
		perror_msg_and_skip("msgctl IPC_STAT");
	printf("msgctl\\(%d, (IPC_64\\|)?IPC_STAT, \\{msg_perm=\\{uid=%u"
	       ", gid=%u, mode=%#o, key=%u, cuid=%u, cgid=%u\\}, msg_stime=%u"
	       ", msg_rtime=%u, msg_ctime=%u, msg_qnum=%u, msg_qbytes=%u"
	       ", msg_lspid=%u, msg_lrpid=%u\\}\\) += 0\n",
	       id, (unsigned) ds.msg_perm.uid, (unsigned) ds.msg_perm.gid,
	       (unsigned) ds.msg_perm.mode, (unsigned) ds.msg_perm.__key,
	       (unsigned) ds.msg_perm.cuid, (unsigned) ds.msg_perm.cgid,
	       (unsigned) ds.msg_stime, (unsigned) ds.msg_rtime,
	       (unsigned) ds.msg_ctime, (unsigned) ds.msg_qnum,
	       (unsigned) ds.msg_qbytes, (unsigned) ds.msg_lspid,
	       (unsigned) ds.msg_lrpid);

	if (msgctl(id, IPC_SET, &ds))
		perror_msg_and_skip("msgctl IPC_SET");
	printf("msgctl\\(%d, (IPC_64\\|)?IPC_SET, \\{msg_perm=\\{uid=%u"
	       ", gid=%u, mode=%#o\\}, ...\\}\\) += 0\n",
	       id, (unsigned) ds.msg_perm.uid, (unsigned) ds.msg_perm.gid,
	       (unsigned) ds.msg_perm.mode);

	rc = msgctl(0, MSG_INFO, &ds);
	printf("msgctl\\(0, (IPC_64\\|)?MSG_INFO, %p\\) += %s\n",
	       &ds, sprintrc_grep(rc));

	rc = msgctl(id, MSG_STAT, &ds);
	printf("msgctl\\(%d, (IPC_64\\|)?MSG_STAT, %p\\) += %s\n",
	       id, &ds, sprintrc_grep(rc));

	return 0;
}
