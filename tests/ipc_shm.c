/*
 * Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2018 The strace developers.
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

#include "tests.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>

#ifndef SHM_HUGE_SHIFT
# define SHM_HUGE_SHIFT 26
#endif

#ifndef SHM_HUGE_MASK
# define SHM_HUGE_MASK 0x3f
#endif

#include "xlat.h"
#include "xlat/shm_resource_flags.h"

#if XLAT_RAW
# define str_ipc_flags "0x2ce1e00"
# define str_shm_huge "21<<26"
# define str_ipc_private "0"
# define str_ipc_rmid "0"
# define str_ipc_set "0x1"
# define str_ipc_stat "0x2"
# define str_shm_stat "0xd"
# define str_shm_info "0xe"
# define str_ipc_64 "0x100"
# define str_bogus_cmd "0xdefaced2"
#elif XLAT_VERBOSE
# define str_ipc_flags \
	"0x2ce1e00 /\\* IPC_CREAT\\|IPC_EXCL\\|SHM_HUGETLB\\|SHM_NORESERVE" \
	"\\|0x2ce0000 \\*/"
# define str_shm_huge "21<<26 /\\* SHM_HUGE_SHIFT \\*/"
# define str_ipc_private "0 /\\* IPC_PRIVATE \\*/"
# define str_ipc_rmid "0 /\\* IPC_RMID \\*/"
# define str_ipc_set "0x1 /\\* IPC_SET \\*/"
# define str_ipc_stat "0x2 /\\* IPC_STAT \\*/"
# define str_shm_stat "0xd /\\* SHM_STAT \\*/"
# define str_shm_info "0xe /\\* SHM_INFO \\*/"
# define str_ipc_64 "0x100 /\\* IPC_64 \\*/"
# define str_bogus_cmd "0xdefaced2 /\\* SHM_\\?\\?\\? \\*/"
#else
# define str_ipc_flags \
	"IPC_CREAT\\|IPC_EXCL\\|SHM_HUGETLB\\|SHM_NORESERVE\\|0x2ce0000"
# define str_shm_huge "21<<SHM_HUGE_SHIFT"
# define str_ipc_private "IPC_PRIVATE"
# define str_ipc_rmid "IPC_RMID"
# define str_ipc_set "IPC_SET"
# define str_ipc_stat "IPC_STAT"
# define str_shm_stat "SHM_STAT"
# define str_shm_info "SHM_INFO"
# define str_ipc_64 "IPC_64"
# define str_bogus_cmd "0xdefaced2 /\\* SHM_\\?\\?\\? \\*/"
#endif

static int id = -1;

static void
cleanup(void)
{
	shmctl(id, IPC_RMID, NULL);
	printf("shmctl\\(%d, (%s\\|)?%s, NULL\\) = 0\n",
	       id, str_ipc_64, str_ipc_rmid);
	id = -1;
}

int
main(void)
{
	static const key_t private_key =
		(key_t) (0xffffffff00000000ULL | IPC_PRIVATE);
	static const key_t bogus_key = (key_t) 0xeca86420fdb97531ULL;
	static const int bogus_id = 0xdefaced1;
	static const int bogus_cmd = 0xdefaced2;
	static void * const bogus_addr = (void *) -1L;
	static const size_t bogus_size =
	/*
	 * musl sets size to SIZE_MAX if size argument is greater than
	 * PTRDIFF_MAX - musl/src/ipc/shmget.c
	 */
	#ifdef __GLIBC__
		(size_t) 0xdec0ded1dec0ded2ULL;
	#else
		(size_t) 0x1e55c0de5dec0dedULL;
	#endif
	static const unsigned int bogus_ipc_shm_flags =
		IPC_CREAT | IPC_EXCL | SHM_HUGETLB | SHM_NORESERVE;
	static const unsigned int huge_mask = SHM_HUGE_MASK << SHM_HUGE_SHIFT;
	static const unsigned int huge_flags = 21 << SHM_HUGE_SHIFT;
	int bogus_flags;
	int rc;
	struct shmid_ds ds;

	rc = shmget(bogus_key, bogus_size, 0);
	printf("shmget\\(%#llx, %zu, 000\\) = %s\n",
	       zero_extend_signed_to_ull(bogus_key), bogus_size,
	       sprintrc_grep(rc));

	rc = shmget(bogus_key, bogus_size, huge_flags);
	printf("shmget\\(%#llx, %zu, %s\\|%#03o\\) = %s\n",
	       zero_extend_signed_to_ull(bogus_key), bogus_size,
	       str_shm_huge, 0, sprintrc_grep(rc));

	bogus_flags = 0xface1e55 & ~(bogus_ipc_shm_flags | huge_mask);
	rc = shmget(bogus_key, bogus_size, bogus_flags);
	printf("shmget\\(%#llx, %zu, %#x\\|%#03o\\) = %s\n",
	       zero_extend_signed_to_ull(bogus_key), bogus_size,
	       bogus_flags & ~0777,
	       bogus_flags & 0777, sprintrc_grep(rc));

	bogus_flags |= bogus_ipc_shm_flags;
	rc = shmget(bogus_key, bogus_size, bogus_flags);
	printf("shmget\\(%#llx, %zu, %s\\|%#03o\\) = %s\n",
	       zero_extend_signed_to_ull(bogus_key), bogus_size,
	       str_ipc_flags,
	       bogus_flags & 0777, sprintrc_grep(rc));

	bogus_flags |= huge_flags;
	rc = shmget(bogus_key, bogus_size, bogus_flags);
	printf("shmget\\(%#llx, %zu, %s\\|%s\\|%#03o\\) = %s\n",
	       zero_extend_signed_to_ull(bogus_key), bogus_size,
	       str_ipc_flags, str_shm_huge,
	       bogus_flags & 0777, sprintrc_grep(rc));

	bogus_flags &= ~bogus_ipc_shm_flags;
	rc = shmget(bogus_key, bogus_size, bogus_flags);
	printf("shmget\\(%#llx, %zu, %#x\\|%s\\|%#03o\\) = %s\n",
	       zero_extend_signed_to_ull(bogus_key), bogus_size,
	       bogus_flags & ~(0777 | huge_mask),
	       str_shm_huge,
	       bogus_flags & 0777, sprintrc_grep(rc));

	id = shmget(private_key, 1, 0600);
	if (id < 0)
		perror_msg_and_skip("shmget");
	printf("shmget\\(%s, 1, 0600\\) = %d\n", str_ipc_private, id);
	atexit(cleanup);

	rc = shmctl(bogus_id, bogus_cmd, NULL);
	printf("shmctl\\(%d, (%s\\|)?%s, NULL\\) = %s\n",
	       bogus_id, str_ipc_64, str_bogus_cmd, sprintrc_grep(rc));

	rc = shmctl(bogus_id, IPC_STAT, bogus_addr);
	printf("shmctl\\(%d, (%s\\|)?%s, %p\\) = %s\n",
	       bogus_id, str_ipc_64, str_ipc_stat, bogus_addr,
	       sprintrc_grep(rc));

	if (shmctl(id, IPC_STAT, &ds))
		perror_msg_and_skip("shmctl IPC_STAT");
	printf("shmctl\\(%d, (%s\\|)?%s, \\{shm_perm=\\{uid=%u, gid=%u, "
		"mode=%#o, key=%u, cuid=%u, cgid=%u\\}, shm_segsz=%u, shm_cpid=%u, "
		"shm_lpid=%u, shm_nattch=%u, shm_atime=%u, shm_dtime=%u, "
		"shm_ctime=%u\\}\\) = 0\n",
		id, str_ipc_64, str_ipc_stat,
		(unsigned) ds.shm_perm.uid, (unsigned) ds.shm_perm.gid,
		(unsigned) ds.shm_perm.mode, (unsigned) ds.shm_perm.__key,
		(unsigned) ds.shm_perm.cuid, (unsigned) ds.shm_perm.cgid,
		(unsigned) ds.shm_segsz, (unsigned) ds.shm_cpid,
		(unsigned) ds.shm_lpid, (unsigned) ds.shm_nattch,
		(unsigned) ds.shm_atime, (unsigned) ds.shm_dtime,
		(unsigned) ds. shm_ctime);

	if (shmctl(id, IPC_SET, &ds))
		perror_msg_and_skip("shmctl IPC_SET");
	printf("shmctl\\(%d, (%s\\|)?%s, \\{shm_perm=\\{uid=%u, gid=%u"
	       ", mode=%#o\\}, ...\\}\\) = 0\n",
	       id, str_ipc_64, str_ipc_set,
	       (unsigned) ds.shm_perm.uid, (unsigned) ds.shm_perm.gid,
	       (unsigned) ds.shm_perm.mode);

	rc = shmctl(0, SHM_INFO, &ds);
	printf("shmctl\\(0, (%s\\|)?%s, %p\\) = %s\n",
	       str_ipc_64, str_shm_info, &ds, sprintrc_grep(rc));

	rc = shmctl(id, SHM_STAT, &ds);
	printf("shmctl\\(%d, (%s\\|)?%s, %p\\) = %s\n",
	       id, str_ipc_64, str_shm_stat, &ds, sprintrc_grep(rc));

	return 0;
}
