/*
 * Copyright (c) 2016-2019 The strace developers.
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

#define XLAT_MACROS_ONLY
#include "xlat/semop_flags.h"
#undef XLAT_MACROS_ONLY

static const char *errstr;

static long
k_semop(const unsigned int semid,
	const kernel_ulong_t sops,
	const unsigned int nsops);

union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
	struct seminfo *__buf;
};

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
	static kernel_ulong_t bogus_sops = (kernel_ulong_t) -1ULL;
	static const unsigned int bogus_nsops = 0xdeadbeefU;

	id = semget(IPC_PRIVATE, 1, 0600);
	if (id < 0)
		perror_msg_and_skip("semget");
	atexit(cleanup);

	union semun sem_union = { .val = 0 };
	if (semctl(id, 0, SETVAL, sem_union) == -1)
		perror_msg_and_skip("semctl");

	k_semop(bogus_semid, 0, bogus_nsops);
	printf("semop(%d, NULL, %u) = %s\n",
		bogus_semid, bogus_nsops, errstr);

	k_semop(bogus_semid, bogus_sops, 1);
	printf("semop(%d, %#llx, %u) = %s\n",
		bogus_semid, (unsigned long long) bogus_sops, 1, errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct sembuf, sem_b2);
	sem_b2->sem_num = 0xface;
	sem_b2->sem_op = 0xf00d;
	sem_b2->sem_flg = 0xbeef;

	k_semop(bogus_semid, (uintptr_t) sem_b2, 2);
	printf("semop(%d, [{%hu, %hd, %s%s%#hx}, ... /* %p */], %u) = %s\n",
		bogus_semid, sem_b2->sem_num, sem_b2->sem_op,
		sem_b2->sem_flg & SEM_UNDO ? "SEM_UNDO|" : "",
		sem_b2->sem_flg & IPC_NOWAIT ? "IPC_NOWAIT|" : "",
		(short) (sem_b2->sem_flg & ~(SEM_UNDO | IPC_NOWAIT)),
		sem_b2 + 1, 2, errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct sembuf, sem_b);
	sem_b->sem_num = 0;
	sem_b->sem_op = 1;
	sem_b->sem_flg = SEM_UNDO;

	k_semop(id, (uintptr_t) sem_b, 1);
	printf("semop(%d, [{0, 1, SEM_UNDO}], 1) = %s\n", id, errstr);

	sem_b->sem_op = -1;
	k_semop(id, (uintptr_t) sem_b, 1);
	printf("semop(%d, [{0, -1, SEM_UNDO}], 1) = %s\n", id, errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
