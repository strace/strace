/*
 * Copyright (c) 2016-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

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

#ifndef SHM_EXEC
# define SHM_EXEC 0100000
#endif

int
main(void)
{
	static const int bogus_shmid = 0xfdb97531;
	static const void * const bogus_shmaddr =
		(void *) (unsigned long) 0xdec0ded1dec0ded2ULL;
	static const int bogus_shmflg = 0xffffface;

	long rc;

	id = shmget(IPC_PRIVATE, 1, 0600);
	if (id < 0)
		perror_msg_and_skip("shmget");
	atexit(cleanup);

	rc = (long) shmat(bogus_shmid, bogus_shmaddr, bogus_shmflg);
	printf("%s(%d, %p, SHM_RDONLY|SHM_RND|SHM_REMAP|SHM_EXEC|%#x) = %s\n",
	       SHMAT, bogus_shmid, bogus_shmaddr, bogus_shmflg & ~0xf000,
	       sprintrc(rc));

	shmat(id, NULL, SHM_REMAP);
	printf("%s(%d, NULL, SHM_REMAP) = %s\n", SHMAT, id, sprintrc(-1));

	void *shmaddr = shmat(id, NULL, SHM_RDONLY);
	if (shmaddr == (void *)(-1))
		perror_msg_and_skip("shmat SHM_RDONLY");
	printf("%s(%d, NULL, SHM_RDONLY) = %p\n", SHMAT, id, shmaddr);

	rc = shmdt(NULL);
	printf("shmdt(NULL) = %s\n", sprintrc(rc));

	rc = shmdt(shmaddr);
	printf("shmdt(%p) = %s\n", shmaddr, sprintrc(rc));

	++shmaddr;
	void *shmaddr2 = shmat(id, shmaddr, SHM_RND);
	if (shmaddr2 == (void *)(-1))
		printf("%s(%d, %p, SHM_RND) = %s\n",
		       SHMAT, id, shmaddr, sprintrc(-1));
	else {
		printf("%s(%d, %p, SHM_RND) = %p\n",
		       SHMAT, id, shmaddr, shmaddr2);
		rc = shmdt(shmaddr2);
		printf("shmdt(%p) = %s\n", shmaddr2, sprintrc(rc));
	}

	shmaddr = shmat(id, NULL, SHM_RDONLY|SHM_EXEC);
	if (shmaddr == (void *)(-1))
		printf("%s(%d, NULL, SHM_RDONLY|SHM_EXEC) = %s\n",
		       SHMAT, id, sprintrc(-1));
	else {
		printf("%s(%d, NULL, SHM_RDONLY|SHM_EXEC) = %p\n",
		       SHMAT, id, shmaddr);
		rc = shmdt(shmaddr);
		printf("shmdt(%p) = %s\n", shmaddr, sprintrc(rc));
	}

	puts("+++ exited with 0 +++");
	return 0;
}
