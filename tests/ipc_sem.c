/*
 * Copyright (c) 2015 Andreas Schwab <schwab@suse.de>
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>

#include "xlat.h"
#include "xlat/resource_flags.h"

#ifndef SEM_STAT_ANY
# define SEM_STAT_ANY 20
#endif

#if XLAT_RAW
# define str_ipc_flags "0xface1e00"
# define str_ipc_private "0"
# define str_ipc_rmid "0"
# define str_ipc_stat "0x2"
# define str_sem_stat "0x12"
# define str_sem_info "0x13"
# define str_sem_stat_any "0x14"
# define str_ipc_64 "0x100"
# define str_bogus_cmd "0xdeadbeef"
#elif XLAT_VERBOSE
# define str_ipc_flags \
	"0xface1e00 /\\* IPC_CREAT\\|IPC_EXCL\\|IPC_NOWAIT\\|0xface1000 \\*/"
# define str_ipc_private "0 /\\* IPC_PRIVATE \\*/"
# define str_ipc_rmid "0 /\\* IPC_RMID \\*/"
# define str_ipc_stat "0x2 /\\* IPC_STAT \\*/"
# define str_sem_stat "0x12 /\\* SEM_STAT \\*/"
# define str_sem_info "0x13 /\\* SEM_INFO \\*/"
# define str_sem_stat_any "0x14 /\\* SEM_STAT_ANY \\*/"
# define str_ipc_64 "0x100 /\\* IPC_64 \\*/"
# define str_bogus_cmd "0xdeadbeef /\\* SEM_\\?\\?\\? \\*/"
#else
# define str_ipc_flags "IPC_CREAT\\|IPC_EXCL\\|IPC_NOWAIT\\|0xface1000"
# define str_ipc_private "IPC_PRIVATE"
# define str_ipc_rmid "IPC_RMID"
# define str_ipc_stat "IPC_STAT"
# define str_sem_stat "SEM_STAT"
# define str_sem_info "SEM_INFO"
# define str_sem_stat_any "SEM_STAT_ANY"
# define str_ipc_64 "IPC_64"
# define str_bogus_cmd "0xdeadbeef /\\* SEM_\\?\\?\\? \\*/"
#endif

union semun {
	int		 val;    /* Value for SETVAL */
	struct semid_ds	*buf;    /* Buffer for IPC_STAT, IPC_SET */
	unsigned short	*array;  /* Array for GETALL, SETALL */
	struct seminfo	*__buf;  /* Buffer for IPC_INFO
				    (Linux-specific) */
};

static int id = -1;

static void
cleanup(void)
{
	semctl(id, 0, IPC_RMID, 0);
	printf("semctl\\(%d, 0, (%s\\|)?%s, \\[?NULL\\]?\\) = 0\n",
	       id, str_ipc_64, str_ipc_rmid);
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
	printf("semget\\(%#llx, %d, %s\\|%#04o\\) = %s\n",
	       zero_extend_signed_to_ull(bogus_key), bogus_size,
	       str_ipc_flags, bogus_flags & 0777, sprintrc_grep(rc));

	id = semget(private_key, 1, 0600);
	if (id < 0)
		perror_msg_and_skip("semget");
	printf("semget\\(%s, 1, 0600\\) = %d\n", str_ipc_private, id);
	atexit(cleanup);

	rc = semctl(bogus_semid, bogus_semnum, bogus_cmd, bogus_arg);
#define SEMCTL_BOGUS_ARG_FMT "(%#lx|\\[(%#lx|NULL)\\]|NULL)"
	printf("semctl\\(%d, %d, (%s\\|)?%s, " SEMCTL_BOGUS_ARG_FMT "\\) = %s\n",
	       bogus_semid, bogus_semnum, str_ipc_64, str_bogus_cmd,
	       bogus_arg, bogus_arg, sprintrc_grep(rc));

	un.buf = &ds;
	if (semctl(id, 0, IPC_STAT, un))
		perror_msg_and_skip("semctl IPC_STAT");
	printf("semctl\\(%d, 0, (%s\\|)?%s, \\[?%p\\]?\\) = 0\n",
	       id, str_ipc_64, str_ipc_stat, &ds);

	un.__buf = &info;
	rc = semctl(0, 0, SEM_INFO, un);
	printf("semctl\\(0, 0, (%s\\|)?%s, \\[?%p\\]?\\) = %s\n",
	       str_ipc_64, str_sem_info, &info, sprintrc_grep(rc));

	un.buf = &ds;
	rc = semctl(id, 0, SEM_STAT, un);
	printf("semctl\\(%d, 0, (%s\\|)?%s, \\[?%p\\]?\\) = %s\n",
	       id, str_ipc_64, str_sem_stat, &ds, sprintrc_grep(rc));

	rc = semctl(id, 0, SEM_STAT_ANY, un);
	printf("semctl\\(%d, 0, (%s\\|)?%s, (%p|\\[(%p|NULL)\\]|NULL)\\) = %s\n",
	       id, str_ipc_64, str_sem_stat_any, &ds, &ds, sprintrc_grep(rc));

	return 0;
}
