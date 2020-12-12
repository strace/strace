/*
 * This file is part of semtimedop* strace tests.
 *
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "kernel_timespec.h"

#define XLAT_MACROS_ONLY
# include "xlat/semop_flags.h"
#undef XLAT_MACROS_ONLY

static long
k_semtimedop_imp(const kernel_ulong_t semid,
		 const kernel_ulong_t sops,
		 const kernel_ulong_t nsops,
		 const kernel_ulong_t timeout);

static const char *errstr;

static long
k_semtimedop(const unsigned int semid,
	     void *const sops,
	     const size_t nsops,
	     const void *const timeout)
{
	static const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced0defacedULL;
	static const kernel_ulong_t k_1 = (kernel_ulong_t) -1;
	const kernel_ulong_t k_semid = semid | (fill & (k_1 - -1U));
	const kernel_ulong_t k_sops = f8ill_ptr_to_kulong(sops);
	const kernel_ulong_t k_nsops = nsops | (fill & (k_1 - (size_t) -1));
	const kernel_ulong_t k_timeout = f8ill_ptr_to_kulong(timeout);
	const long rc = k_semtimedop_imp(k_semid, k_sops, k_nsops, k_timeout);
	errstr = sprintrc(rc);
	return rc;
}

static int id = -1;

static void
cleanup(void)
{
	semctl(id, 0, IPC_RMID, 0);
	id = -1;
}

int
main(void)
{
	static const int bogus_semid = 0xfdb97531;
	static void * const bogus_sops = (void *) -1UL;
	static const size_t bogus_nsops = (size_t) 0xdefaceddeadbeefULL;

	TAIL_ALLOC_OBJECT_CONST_PTR(semtimedop_timespec_t, ts);

	id = semget(IPC_PRIVATE, 1, 0600);
	if (id < 0)
		perror_msg_and_skip("semget");
	atexit(cleanup);

	union {
		int val;
		void *buf;
	} sem_union = { .val = 0 };
	if (semctl(id, 0, SETVAL, sem_union) == -1)
		perror_msg_and_skip("semctl");

	TAIL_ALLOC_OBJECT_CONST_PTR(struct sembuf, sem_b);
	TAIL_ALLOC_OBJECT_CONST_PTR(struct sembuf, sem_b2);

	k_semtimedop(bogus_semid, NULL, bogus_nsops, NULL);
	printf("%s(%d, NULL, %u, NULL) = %s\n",
	       SYSCALL_NAME, bogus_semid, (unsigned) bogus_nsops, errstr);

	k_semtimedop(bogus_semid, bogus_sops, 1, NULL);
	printf("%s(%d, %p, %u, NULL) = %s\n",
	       SYSCALL_NAME, bogus_semid, bogus_sops, 1, errstr);

	sem_b->sem_num = 0;
	sem_b->sem_op = 1;
	sem_b->sem_flg = SEM_UNDO;

	sem_b2->sem_num = 0xface;
	sem_b2->sem_op = 0xf00d;
	sem_b2->sem_flg = 0xbeef;

	k_semtimedop(bogus_semid, sem_b2, 2, NULL);
	printf("%s(%d, [{%hu, %hd, %s%s%#hx}, ... /* %p */], %u"
	       ", NULL) = %s\n",
	       SYSCALL_NAME, bogus_semid, sem_b2->sem_num, sem_b2->sem_op,
	       sem_b2->sem_flg & SEM_UNDO ? "SEM_UNDO|" : "",
	       sem_b2->sem_flg & IPC_NOWAIT ? "IPC_NOWAIT|" : "",
	       (short) (sem_b2->sem_flg & ~(SEM_UNDO | IPC_NOWAIT)),
	       sem_b2 + 1, 2, errstr);

	if (k_semtimedop(id, sem_b, 1, NULL))
		perror_msg_and_skip(SYSCALL_NAME ", 1");
	printf("%s(%d, [{0, 1, SEM_UNDO}], 1, NULL) = 0\n", SYSCALL_NAME, id);

	sem_b->sem_op = -1;
	if (k_semtimedop(id, sem_b, 1, NULL))
		perror_msg_and_skip(SYSCALL_NAME ", -1");
	printf("%s(%d, [{0, -1, SEM_UNDO}], 1, NULL) = 0\n", SYSCALL_NAME, id);

	k_semtimedop(bogus_semid, NULL, bogus_nsops, NULL);
	printf("%s(%d, NULL, %u, NULL) = %s\n",
	       SYSCALL_NAME, bogus_semid, (unsigned) bogus_nsops, errstr);

	k_semtimedop(id, sem_b + 1, 1, ts + 1);
	printf("%s(%d, %p, 1, %p) = %s\n",
	       SYSCALL_NAME, id, sem_b + 1, ts + 1, errstr);

	ts->tv_sec = 1;
	ts->tv_nsec = 123456789;
	k_semtimedop(bogus_semid, sem_b2, 2, ts);
	printf("%s(%d, [{%hu, %hd, %s%s%#hx}, ... /* %p */], %u"
	       ", {tv_sec=%lld, tv_nsec=%llu}) = %s\n",
	       SYSCALL_NAME, bogus_semid, sem_b2->sem_num, sem_b2->sem_op,
	       sem_b2->sem_flg & SEM_UNDO ? "SEM_UNDO|" : "",
	       sem_b2->sem_flg & IPC_NOWAIT ? "IPC_NOWAIT|" : "",
	       (short) (sem_b2->sem_flg & ~(SEM_UNDO | IPC_NOWAIT)),
	       sem_b2 + 1, 2,
	       (long long) ts->tv_sec, zero_extend_signed_to_ull(ts->tv_nsec),
	       errstr);

	sem_b->sem_op = 1;
	if (k_semtimedop(id, sem_b, 1, NULL))
		perror_msg_and_skip(SYSCALL_NAME ", 1");
	printf("%s(%d, [{0, 1, SEM_UNDO}], 1, NULL) = 0\n", SYSCALL_NAME, id);

	sem_b->sem_op = -1;
	if (k_semtimedop(id, sem_b, 1, ts))
		perror_msg_and_skip(SYSCALL_NAME ", -1");
	printf("%s(%d, [{0, -1, SEM_UNDO}], 1"
	       ", {tv_sec=%lld, tv_nsec=%llu}) = 0\n",
	       SYSCALL_NAME, id,
	       (long long) ts->tv_sec, zero_extend_signed_to_ull(ts->tv_nsec));

	sem_b->sem_op = 1;
	ts->tv_sec = 0xdeadbeefU;
	ts->tv_nsec = 0xfacefeedU;
	k_semtimedop(id, sem_b, 1, ts);
	printf("%s(%d, [{0, 1, SEM_UNDO}], 1"
	       ", {tv_sec=%lld, tv_nsec=%llu}) = %s\n",
	       SYSCALL_NAME, id, (long long) ts->tv_sec,
	       zero_extend_signed_to_ull(ts->tv_nsec), errstr);

	sem_b->sem_op = -1;
	ts->tv_sec = (typeof(ts->tv_sec)) 0xcafef00ddeadbeefLL;
	ts->tv_nsec = (typeof(ts->tv_nsec)) 0xbadc0dedfacefeedLL;
	k_semtimedop(id, sem_b, 1, ts);
	printf("%s(%d, [{0, -1, SEM_UNDO}], 1"
	       ", {tv_sec=%lld, tv_nsec=%llu}) = %s\n",
	       SYSCALL_NAME, id, (long long) ts->tv_sec,
	       zero_extend_signed_to_ull(ts->tv_nsec), errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
