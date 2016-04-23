#include "tests.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>

static int id = -1;

static void
cleanup(void)
{
	shmctl(id, IPC_RMID, NULL);
	id = -1;
}

#ifdef __alpha__
# define SHMAT "osf_shmat"
#else
# define SHMAT "shmat"
#endif

int
main(void)
{
	id = shmget(IPC_PRIVATE, 1, 0600);
	if (id < 0)
		perror_msg_and_skip("shmget");
	atexit(cleanup);

	shmat(id, NULL, SHM_REMAP);
	printf("%s(%d, NULL, SHM_REMAP) = -1 %s (%m)\n",
	       SHMAT, id, errno2name());

	void *shmaddr = shmat(id, NULL, SHM_RDONLY);
	if (shmaddr == (void *)(-1))
		perror_msg_and_skip("shmat SHM_RDONLY");
	printf("%s(%d, NULL, SHM_RDONLY) = %p\n", SHMAT, id, shmaddr);

	if (shmdt(shmaddr))
		perror_msg_and_skip("shmdt");
	printf("shmdt(%p) = 0\n", shmaddr);

	++shmaddr;
	void *shmaddr2 = shmat(id, shmaddr, SHM_RND);
	if (shmaddr2 == (void *)(-1))
		printf("%s(%d, %p, SHM_RND) = -1 %s (%m)\n",
		       SHMAT, id, shmaddr, errno2name());
	else
		printf("%s(%d, %p, SHM_RND) = %p\n",
		       SHMAT, id, shmaddr, shmaddr2);

	puts("+++ exited with 0 +++");
	return 0;
}
