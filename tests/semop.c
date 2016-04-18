#include "tests.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
	id = semget(IPC_PRIVATE, 1, 0600);
	if (id < 0)
		perror_msg_and_skip("semget");
	atexit(cleanup);

	union semun sem_union = { .val = 0 };
	if (semctl(id, 0, SETVAL, sem_union) == -1)
		perror_msg_and_skip("semctl");

	struct sembuf *const sem_b = tail_alloc(sizeof(*sem_b));
	sem_b->sem_num = 0;
	sem_b->sem_op = 1;
	sem_b->sem_flg = SEM_UNDO;

	if (semop(id, sem_b, 1))
		perror_msg_and_skip("semop, 1");
	printf("semop(%d, [{0, 1, SEM_UNDO}], 1) = 0\n", id);

	sem_b->sem_op = -1;
	if (semop(id, sem_b, 1))
		perror_msg_and_skip("semop, -1");
	printf("semop(%d, [{0, -1, SEM_UNDO}], 1) = 0\n", id);

	puts("+++ exited with 0 +++");
	return 0;
}
