#include <stdio.h>
#include <sys/sem.h>

int
main(void)
{
	int id = semget(IPC_PRIVATE, 1, 0600);
	if (id < 0)
		return 77;
	printf("semget\\(IPC_PRIVATE, 1, 0600\\) += %d\n", id);

	int rc = 1;

	struct seminfo info;
	int max = semctl(0, 0, SEM_INFO, &info);
	if (max < 0)
		goto fail;
	printf("semctl\\(0, 0, SEM_INFO, %p\\) += %d\n", &info, max);

	struct semid_ds ds;
	if (semctl(id, 0, SEM_STAT, &ds) != id)
		goto fail;
	printf("semctl\\(%d, 0, SEM_STAT, %p\\) += %d\n", id, &ds, id);

	rc = 0;

fail:
	if (semctl(id, 0, IPC_RMID, 0) < 0)
		return 1;
	printf("semctl\\(%d, 0, IPC_RMID, 0\\) += 0\n", id);

	return rc;
}
