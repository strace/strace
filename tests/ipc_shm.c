#include <stdio.h>
#include <sys/shm.h>

int
main(void)
{
	int id = shmget(IPC_PRIVATE, 1, 0600);
	if (id < 0)
		return 77;
	printf("shmget\\(IPC_PRIVATE, 1, 0600\\) += %d\n", id);

	int rc = 1;

	struct shmid_ds ds;
	int max = shmctl(0, SHM_INFO, &ds);
	if (max < 0)
		goto fail;
	printf("shmctl\\(0, SHM_INFO, %p\\) += %d\n", &ds, max);

	if (shmctl(id, SHM_STAT, &ds) != id)
		goto fail;
	printf("shmctl\\(%d, SHM_STAT, %p\\) += %d\n", id, &ds, id);

	rc = 0;

fail:
	if (shmctl(id, IPC_RMID, 0) < 0)
		return 1;
	printf("shmctl\\(%d, IPC_RMID, 0\\) += 0\n", id);
	return rc;
}
