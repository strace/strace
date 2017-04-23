#include "tests.h"
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "xlat.h"
#include "xlat/semop_flags.h"

union semun
{
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
	static void * const bogus_sops = (void *) -1L;
	static const size_t bogus_nsops = (size_t) 0xdefaceddeadbeefULL;

	TAIL_ALLOC_OBJECT_CONST_PTR(struct timespec, ts);
	int rc;

	id = semget(IPC_PRIVATE, 1, 0600);
	if (id < 0)
		perror_msg_and_skip("semget");
	atexit(cleanup);

	union semun sem_union = { .val = 0 };
	if (semctl(id, 0, SETVAL, sem_union) == -1)
		perror_msg_and_skip("semctl");

	TAIL_ALLOC_OBJECT_CONST_PTR(struct sembuf, sem_b);
	TAIL_ALLOC_OBJECT_CONST_PTR(struct sembuf, sem_b2);

	rc = semop(bogus_semid, NULL, bogus_nsops);
	printf("semop(%d, NULL, %u) = %s\n",
		bogus_semid, (unsigned) bogus_nsops, sprintrc(rc));

	rc = semop(bogus_semid, bogus_sops, 1);
	printf("semop(%d, %p, %u) = %s\n",
		bogus_semid, bogus_sops, 1, sprintrc(rc));

	sem_b->sem_num = 0;
	sem_b->sem_op = 1;
	sem_b->sem_flg = SEM_UNDO;

	sem_b2->sem_num = 0xface;
	sem_b2->sem_op = 0xf00d;
	sem_b2->sem_flg = 0xbeef;

	rc = semop(bogus_semid, sem_b2, 2);
	printf("semop(%d, [{%hu, %hd, %s%s%#hx}, %p], %u) = %s\n",
		bogus_semid, sem_b2->sem_num, sem_b2->sem_op,
		sem_b2->sem_flg & SEM_UNDO ? "SEM_UNDO|" : "",
		sem_b2->sem_flg & IPC_NOWAIT ? "IPC_NOWAIT|" : "",
		sem_b2->sem_flg & ~(SEM_UNDO | IPC_NOWAIT),
		sem_b2 + 1, 2, sprintrc(rc));

	if (semop(id, sem_b, 1))
		perror_msg_and_skip("semop, 1");
	printf("semop(%d, [{0, 1, SEM_UNDO}], 1) = 0\n", id);

	sem_b->sem_op = -1;
	if (semop(id, sem_b, 1))
		perror_msg_and_skip("semop, -1");
	printf("semop(%d, [{0, -1, SEM_UNDO}], 1) = 0\n", id);

	rc = semtimedop(bogus_semid, NULL, bogus_nsops, NULL);
	printf("semtimedop(%d, NULL, %u, NULL) = %s\n",
		bogus_semid, (unsigned) bogus_nsops, sprintrc(rc));

	rc = semtimedop(id, sem_b + 1, 1, ts + 1);
	printf("semtimedop(%d, %p, 1, %p) = %s\n",
		id, sem_b + 1, ts + 1, sprintrc(rc));

	ts->tv_sec = 1;
	ts->tv_nsec = 123456789;
	rc = semtimedop(bogus_semid, sem_b2, 2, ts);
	printf("semtimedop(%d, [{%hu, %hd, %s%s%#hx}, %p], %u"
		", {tv_sec=%lld, tv_nsec=%llu}) = %s\n",
		bogus_semid, sem_b2->sem_num, sem_b2->sem_op,
		sem_b2->sem_flg & SEM_UNDO ? "SEM_UNDO|" : "",
		sem_b2->sem_flg & IPC_NOWAIT ? "IPC_NOWAIT|" : "",
		sem_b2->sem_flg & ~(SEM_UNDO | IPC_NOWAIT),
		sem_b2 + 1, 2,
		(long long) ts->tv_sec, zero_extend_signed_to_ull(ts->tv_nsec),
		sprintrc(rc));

	sem_b->sem_op = 1;
	if (semtimedop(id, sem_b, 1, NULL))
		perror_msg_and_skip("semtimedop, 1");
	printf("semtimedop(%d, [{0, 1, SEM_UNDO}], 1, NULL) = 0\n", id);

	sem_b->sem_op = -1;
	if (semtimedop(id, sem_b, 1, ts))
		perror_msg_and_skip("semtimedop, -1");
	printf("semtimedop(%d, [{0, -1, SEM_UNDO}], 1"
	       ", {tv_sec=%lld, tv_nsec=%llu}) = 0\n", id,
	       (long long) ts->tv_sec, zero_extend_signed_to_ull(ts->tv_nsec));

	sem_b->sem_op = 1;
	ts->tv_sec = 0xdeadbeefU;
	ts->tv_nsec = 0xfacefeedU;
	rc = semtimedop(id, sem_b, 1, ts);
	printf("semtimedop(%d, [{0, 1, SEM_UNDO}], 1"
	       ", {tv_sec=%lld, tv_nsec=%llu}) = %s\n",
	       id, (long long) ts->tv_sec,
	       zero_extend_signed_to_ull(ts->tv_nsec), sprintrc(rc));

	sem_b->sem_op = -1;
	ts->tv_sec = (time_t) 0xcafef00ddeadbeefLL;
	ts->tv_nsec = (long) 0xbadc0dedfacefeedLL;
	rc = semtimedop(id, sem_b, 1, ts);
	printf("semtimedop(%d, [{0, -1, SEM_UNDO}], 1"
	       ", {tv_sec=%lld, tv_nsec=%llu}) = %s\n",
	       id, (long long) ts->tv_sec,
	       zero_extend_signed_to_ull(ts->tv_nsec), sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}
