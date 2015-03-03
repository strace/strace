#include <stdio.h>
#include <errno.h>
#include <sys/sem.h>

int
main(void)
{
	int rc, id;
	struct semid_ds ds;
	struct seminfo info;

	id = semget(IPC_PRIVATE, 1, 0600);
	if (id < 0)
		return 77;
	printf("semget\\(IPC_PRIVATE, 1, 0600\\) += %d\n", id);

	if (semctl(id, 0, IPC_STAT, &ds))
		goto fail;
	printf("semctl\\(%d, 0, IPC_STAT, %p\\) += 0\n", id, &ds);

	int max = semctl(0, 0, SEM_INFO, &info);
	if (max < 0)
		goto fail;
	printf("semctl\\(0, 0, SEM_INFO, %p\\) += %d\n", &info, max);

	rc = semctl(id, 0, SEM_STAT, &ds);
	if (rc != id) {
		/*
		 * In linux < v2.6.24-rc1 the first argument must be
		 * an index in the kernel's internal array.
		 */
		if (-1 != rc || EINVAL != errno)
			goto fail;
		printf("semctl\\(%d, 0, SEM_STAT, %p\\) += -1 EINVAL \\(Invalid argument\\)\n", id, &ds);
	} else {
		printf("semctl\\(%d, 0, SEM_STAT, %p\\) += %d\n", id, &ds, id);
	}

	rc = 0;
done:
	if (semctl(id, 0, IPC_RMID, 0) < 0)
		return 1;
	printf("semctl\\(%d, 0, IPC_RMID, 0\\) += 0\n", id);
	return rc;

fail:
	/*
	 * If the kernel failed, SKIP the test.  We want to ignore
	 * such failures as they're out of scope for this project.
	 */
	rc = errno == EFAULT ? 77 : 1;
	goto done;
}
