#include <stdio.h>
#include <sys/msg.h>

int
main(void)
{
	int id = msgget(IPC_PRIVATE, 0600);
	if (id < 0)
		return 77;
	printf("msgget\\(IPC_PRIVATE, 0600\\) += %d\n", id);

	int rc = 1;

	struct msqid_ds ds;
	int max = msgctl(0, MSG_INFO, &ds);
	if (max < 0)
		goto fail;
	printf("msgctl\\(0, MSG_INFO, %p\\) += %d\n", &ds, max);

	if (msgctl(id, MSG_STAT, &ds) != id)
		goto fail;
	printf("msgctl\\(%d, MSG_STAT, %p\\) += %d\n", id, &ds, id);

	rc = 0;

fail:
	if (msgctl(id, IPC_RMID, 0) < 0)
		return 1;
	printf("msgctl\\(%d, IPC_RMID, 0\\) += 0\n", id);
	return rc;
}
