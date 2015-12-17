/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <errno.h>
#include <sys/shm.h>

int
main(void)
{
	int rc, id;
	struct shmid_ds ds;

	id = shmget(IPC_PRIVATE, 1, 0600);
	if (id < 0)
		return 77;
	printf("shmget\\(IPC_PRIVATE, 1, 0600\\) += %d\n", id);

	if (shmctl(id, IPC_STAT, &ds))
		goto fail;
	printf("shmctl\\(%d, (IPC_64\\|)?IPC_STAT, \\{shm_perm=\\{uid=%u, gid=%u, "
		"mode=%#o, key=%u, cuid=%u, cgid=%u\\}, shm_segsz=%u, shm_cpid=%u, "
		"shm_lpid=%u, shm_nattch=%u, shm_atime=%u, shm_dtime=%u, "
		"shm_ctime=%u\\}\\) += 0\n",
		id, (unsigned) ds.shm_perm.uid, (unsigned) ds.shm_perm.gid,
		(unsigned) ds.shm_perm.mode, (unsigned) ds.shm_perm.__key,
		(unsigned) ds.shm_perm.cuid, (unsigned) ds.shm_perm.cgid,
		(unsigned) ds.shm_segsz, (unsigned) ds.shm_cpid,
		(unsigned) ds.shm_lpid, (unsigned) ds.shm_nattch,
		(unsigned) ds.shm_atime, (unsigned) ds.shm_dtime,
		(unsigned) ds. shm_ctime);

	int max = shmctl(0, SHM_INFO, &ds);
	if (max < 0)
		goto fail;
	printf("shmctl\\(0, (IPC_64\\|)?SHM_INFO, %p\\) += %d\n", &ds, max);

	rc = shmctl(id, SHM_STAT, &ds);
	if (rc != id) {
		/*
		 * In linux < v2.6.24-rc1 the first argument must be
		 * an index in the kernel's internal array.
		 */
		if (-1 != rc || EINVAL != errno)
			goto fail;
		printf("shmctl\\(%d, (IPC_64\\|)?SHM_STAT, %p\\) += -1 EINVAL \\(Invalid argument\\)\n", id, &ds);
	} else {
		printf("shmctl\\(%d, (IPC_64\\|)?SHM_STAT, %p\\) += %d\n", id, &ds, id);
	}

	rc = 0;
done:
	if (shmctl(id, IPC_RMID, NULL) < 0)
		return 1;
	printf("shmctl\\(%d, (IPC_64\\|)?IPC_RMID, NULL\\) += 0\n", id);
	return rc;

fail:
	rc = 1;
	goto done;
}
