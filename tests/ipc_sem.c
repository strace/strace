/*
 * Copyright (c) 2015 Andreas Schwab <schwab@suse.de>
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
#include <sys/sem.h>

#include "xlat.h"
#include "xlat/resource_flags.h"

union semun {
	int              val;    /* Value for SETVAL */
	struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
	unsigned short  *array;  /* Array for GETALL, SETALL */
	struct seminfo  *__buf;  /* Buffer for IPC_INFO
				    (Linux-specific) */
};

static int id = -1;

static void
cleanup(void)
{
	semctl(id, 0, IPC_RMID, 0);
	printf("semctl\\(%d, 0, (IPC_64\\|)?IPC_RMID, \\[?NULL\\]?\\) += 0\n",
	       id);
	id = -1;
}

int
main(void)
{
	static const key_t private_key =
		(key_t) (0xffffffff00000000ULL | IPC_PRIVATE);
	static const key_t bogus_key = (key_t) 0xeca86420fdb97531ULL;
	static const int bogus_semid = 0xfdb97531;
	static const int bogus_semnum = 0xeca86420;
	static const int bogus_size = 0xdec0ded1;
	static const int bogus_flags = 0xface1e55;
	static const int bogus_cmd = 0xdeadbeef;
	static const unsigned long bogus_arg =
		(unsigned long) 0xbadc0dedfffffaceULL;

	int rc;
	union semun un;
	struct semid_ds ds;
	struct seminfo info;

	rc = semget(bogus_key, bogus_size, bogus_flags);
	printf("semget\\(%#llx, %d, %s%s%s%#x\\|%#04o\\) += %s\n",
	       zero_extend_signed_to_ull(bogus_key), bogus_size,
	       IPC_CREAT & bogus_flags ? "IPC_CREAT\\|" : "",
	       IPC_EXCL & bogus_flags ? "IPC_EXCL\\|" : "",
	       IPC_NOWAIT & bogus_flags ? "IPC_NOWAIT\\|" : "",
	       bogus_flags & ~(0777 | IPC_CREAT | IPC_EXCL | IPC_NOWAIT),
	       bogus_flags & 0777, sprintrc_grep(rc));

	id = semget(private_key, 1, 0600);
	if (id < 0)
		perror_msg_and_skip("semget");
	printf("semget\\(IPC_PRIVATE, 1, 0600\\) += %d\n", id);
	atexit(cleanup);

	rc = semctl(bogus_semid, bogus_semnum, bogus_cmd, bogus_arg);
#define SEMCTL_BOGUS_ARG_FMT "(%#lx|\\[(%#lx|NULL)\\]|NULL)"
	printf("semctl\\(%d, %d, (IPC_64\\|)?%#x /\\* SEM_\\?\\?\\? \\*/"
	       ", " SEMCTL_BOGUS_ARG_FMT "\\) += %s\n",
	       bogus_semid, bogus_semnum, bogus_cmd,
	       bogus_arg, bogus_arg, sprintrc_grep(rc));

	un.buf = &ds;
	if (semctl(id, 0, IPC_STAT, un))
		perror_msg_and_skip("semctl IPC_STAT");
	printf("semctl\\(%d, 0, (IPC_64\\|)?IPC_STAT, \\[?%p\\]?\\) += 0\n",
	       id, &ds);

	un.__buf = &info;
	rc = semctl(0, 0, SEM_INFO, un);
	printf("semctl\\(0, 0, (IPC_64\\|)?SEM_INFO, \\[?%p\\]?\\) += %s\n",
	       &info, sprintrc_grep(rc));

	un.buf = &ds;
	rc = semctl(id, 0, SEM_STAT, un);
	printf("semctl\\(%d, 0, (IPC_64\\|)?SEM_STAT, \\[?%p\\]?\\) += %s\n",
	       id, &ds, sprintrc_grep(rc));

	return 0;
}
