/*
 * Copyright (c) 2015 Andreas Schwab <schwab@suse.de>
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sem.h>

#include "xlat.h"
#include "xlat/resource_flags.h"

#ifndef SEM_STAT_ANY
# define SEM_STAT_ANY 20
#endif

#undef TEST_SEMCTL_BOGUS_CMD

/*
 * Starting with commit glibc-2.32.9000-147-ga16d2abd496bd974a882,
 * glibc skips semctl syscall invocations and returns EINVAL
 * for invalid semctl commands.
 *
 * Apparently, this change was later backported to vendor packages, e.g.:
 * Thu Mar 18 2021 Carlos O'Donell <carlos@redhat.com> - 2.28-153
 * - Support SEM_STAT_ANY via semctl. Return EINVAL for unknown commands
 *   to semctl, msgctl, and shmctl. (#1912670)
 */
#if GLIBC_PREREQ_GE(2, 28)
# define TEST_SEMCTL_BOGUS_CMD 0
#endif

#ifndef TEST_SEMCTL_BOGUS_CMD
# define TEST_SEMCTL_BOGUS_CMD 1
#endif

#if XLAT_RAW
# define str_ipc_flags "0xface1e00"
# define str_ipc_private "0"
# define str_ipc_rmid "0"
# define str_ipc_set "0x1"
# define str_ipc_stat "0x2"
# define str_ipc_info "0x3"
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
# define str_ipc_set "0x1 /\\* IPC_SET \\*/"
# define str_ipc_stat "0x2 /\\* IPC_STAT \\*/"
# define str_ipc_info "0x3 /\\* IPC_INFO \\*/"
# define str_sem_stat "0x12 /\\* SEM_STAT \\*/"
# define str_sem_info "0x13 /\\* SEM_INFO \\*/"
# define str_sem_stat_any "0x14 /\\* SEM_STAT_ANY \\*/"
# define str_ipc_64 "0x100 /\\* IPC_64 \\*/"
# define str_bogus_cmd "0xdeadbeef /\\* SEM_\\?\\?\\? \\*/"
#else
# define str_ipc_flags "IPC_CREAT\\|IPC_EXCL\\|IPC_NOWAIT\\|0xface1000"
# define str_ipc_private "IPC_PRIVATE"
# define str_ipc_rmid "IPC_RMID"
# define str_ipc_set "IPC_SET"
# define str_ipc_stat "IPC_STAT"
# define str_ipc_info "IPC_INFO"
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

static void
print_semid_ds(const char *const str_ipc_cmd,
              const struct semid_ds *const ds,
              const int rc)
{
	if (rc < 0) {
		printf("semctl\\(%d, 0, (%s\\|)?%s, (%p|\\[%p\\])\\) = %s\n",
		       id, str_ipc_64, str_ipc_cmd, &ds, &ds,
		       sprintrc_grep(rc));
		return;
	}
	printf("semctl\\(%d, 0, (%s\\|)?%s, \\{sem_perm=\\{uid=%u, gid=%u"
		", mode=%#o, key=%u, cuid=%u, cgid=%u\\}, sem_otime=%llu"
		", sem_ctime=%llu, sem_nsems=%llu\\}\\) = %d\n",
		id,
		str_ipc_64,
		str_ipc_cmd,
		(unsigned) ds->sem_perm.uid,
		(unsigned) ds->sem_perm.gid,
		(unsigned) ds->sem_perm.mode,
		(unsigned) ds->sem_perm.__key,
		(unsigned) ds->sem_perm.cuid,
		(unsigned) ds->sem_perm.cgid,
		(unsigned long long) ds->sem_otime,
		(unsigned long long) ds->sem_ctime,
		(unsigned long long) ds->sem_nsems,
		rc);
}

static void
print_sem_info(const char *const str_ipc_cmd,
               const struct seminfo *const info,
               const int rc)
{
	if (rc < 0) {
		printf("semctl\\(%d, 0, (%s\\|)?%s, (%p|\\[%p\\])\\) = %s\n",
		       id, str_ipc_64, str_ipc_cmd, &info, &info,
		       sprintrc_grep(rc));
		return;
	}

	printf("semctl\\(%d, 0, (%s\\|)?%s, \\{semmap=%d, semmni=%d"
	       ", semmns=%d, semmnu=%d, semmsl=%d, semopm=%d, semume=%d"
	       ", semusz=%d, semvmx=%d, semaem=%d\\}\\) = %d\n",
	       id,
	       str_ipc_64,
	       str_ipc_cmd,
	       info->semmap,
	       info->semmni,
	       info->semmns,
	       info->semmnu,
	       info->semmsl,
	       info->semopm,
	       info->semume,
	       info->semusz,
	       info->semvmx,
	       info->semaem,
	       rc);
}

int
main(void)
{
	static const key_t private_key =
		(key_t) (0xffffffff00000000ULL | IPC_PRIVATE);
	static const key_t bogus_key = (key_t) 0xeca86420fdb97531ULL;
	static const int bogus_size = 0xdec0ded1;
	static const int bogus_flags = 0xface1e55;
#if TEST_SEMCTL_BOGUS_CMD
	static const int bogus_semid = 0xfdb97531;
	static const int bogus_semnum = 0xeca86420;
	static const int bogus_cmd = 0xdeadbeef;
	static const unsigned long bogus_arg =
		(unsigned long) 0xbadc0dedfffffaceULL;
#endif

	int rc;
	union semun un;
	struct semid_ds ds;
	struct seminfo info;

	memset(&ds, 0, sizeof(ds));

	rc = semget(bogus_key, bogus_size, bogus_flags);
	printf("semget\\(%#llx, %d, %s\\|%#04o\\) = %s\n",
	       zero_extend_signed_to_ull(bogus_key), bogus_size,
	       str_ipc_flags, bogus_flags & 0777, sprintrc_grep(rc));

	id = semget(private_key, 1, 0600);
	if (id < 0)
		perror_msg_and_skip("semget");
	printf("semget\\(%s, 1, 0600\\) = %d\n", str_ipc_private, id);
	atexit(cleanup);

#if TEST_SEMCTL_BOGUS_CMD
	rc = semctl(bogus_semid, bogus_semnum, bogus_cmd, bogus_arg);
# define SEMCTL_BOGUS_ARG_FMT "(%#lx|\\[(%#lx|NULL)\\]|NULL)"
	printf("semctl\\(%d, %d, (%s\\|)?%s, " SEMCTL_BOGUS_ARG_FMT "\\) = %s\n",
	       bogus_semid, bogus_semnum, str_ipc_64, str_bogus_cmd,
	       bogus_arg, bogus_arg, sprintrc_grep(rc));
#endif

	un.__buf = &info;
	rc = semctl(id, 0, IPC_INFO, un);
        print_sem_info(str_ipc_info, &info, rc);

	rc = semctl(id, 0, SEM_INFO, un);
        print_sem_info(str_sem_info, &info, rc);

	un.buf = &ds;
	rc = semctl(id, 0, IPC_STAT, un);
	if (rc < 0)
		perror_msg_and_skip("semctl IPC_STAT");
	print_semid_ds(str_ipc_stat, &ds, rc);

	if (semctl(id, 0, IPC_SET, un))
		perror_msg_and_skip("semctl IPC_SET");
	printf("semctl\\(%d, 0, (%s\\|)?%s, \\{sem_perm=\\{uid=%u, gid=%u"
	       ", mode=%#o\\}\\}\\) = 0\n",
	       id, str_ipc_64, str_ipc_set,
	       (unsigned) ds.sem_perm.uid,
	       (unsigned) ds.sem_perm.gid,
	       (unsigned) ds.sem_perm.mode);

	rc = semctl(id, 0, SEM_STAT, un);
	print_semid_ds(str_sem_stat, &ds, rc);

/*
 * glibc fails to pass the buffer for SEM_STAT_ANY command,
 * so the kernel receives garbage instead of un.buf address:
 * https://sourceware.org/bugzilla/show_bug.cgi?id=26637
 * musl doesn't pass the buffer either.
 */
#if 0
	rc = semctl(id, 0, SEM_STAT_ANY, un);
	print_semid_ds(str_sem_stat_any, &ds, rc);
#endif

	return 0;
}
