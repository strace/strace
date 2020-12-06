/*
 * Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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

#ifndef SHM_STAT_ANY
# define SHM_STAT_ANY 15
#endif

#ifndef SHM_NORESERVE
# define SHM_NORESERVE 010000
#endif

#undef TEST_SHMCTL_BOGUS_ADDR
#undef TEST_SHMCTL_BOGUS_CMD

/*
 * Starting with commit glibc-2.32~80, on every 32-bit architecture
 * where 32-bit time_t support is enabled, glibc tries to retrieve
 * the data provided in the third argument of shmctl call.
 */
#if GLIBC_PREREQ_GE(2, 32) && defined __TIMESIZE && __TIMESIZE != 64
# define TEST_SHMCTL_BOGUS_ADDR 0
#endif

/*
 * Starting with commit glibc-2.32.9000-207-g9ebaabeaac1a96b0d91f,
 * glibc skips shmctl syscall invocations and returns EINVAL
 * for invalid shmctl commands.
 */
#if GLIBC_PREREQ_GE(2, 32)
# define TEST_SHMCTL_BOGUS_CMD 0
#endif

#ifndef TEST_SHMCTL_BOGUS_ADDR
# define TEST_SHMCTL_BOGUS_ADDR 1
#endif
#ifndef TEST_SHMCTL_BOGUS_CMD
# define TEST_SHMCTL_BOGUS_CMD 1
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
# define str_ipc_info "0x3"
# define str_shm_stat "0xd"
# define str_shm_info "0xe"
# define str_shm_stat_any "0xf"
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
# define str_ipc_info "0x3 /\\* IPC_INFO \\*/"
# define str_shm_stat "0xd /\\* SHM_STAT \\*/"
# define str_shm_info "0xe /\\* SHM_INFO \\*/"
# define str_shm_stat_any "0xf /\\* SHM_STAT_ANY \\*/"
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
# define str_ipc_info "IPC_INFO"
# define str_shm_stat "SHM_STAT"
# define str_shm_info "SHM_INFO"
# define str_shm_stat_any "SHM_STAT_ANY"
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

static void
print_shmid_ds(const char *const str_ipc_cmd,
	       const struct shmid_ds *const ds,
	       const int rc)
{
	if (rc < 0) {
		printf("shmctl\\(%d, (%s\\|)?%s, %p\\) = %s\n",
		       id, str_ipc_64, str_ipc_cmd, ds, sprintrc_grep(rc));
		return;
	}
	printf("shmctl\\(%d, (%s\\|)?%s, \\{shm_perm=\\{uid=%u, gid=%u"
		", mode=%#o, key=%u, cuid=%u, cgid=%u\\}, shm_segsz=%u"
		", shm_cpid=%d, shm_lpid=%d, shm_nattch=%u, shm_atime=%u"
		", shm_dtime=%u, shm_ctime=%u\\}\\) = %d\n",
		id,
		str_ipc_64,
		str_ipc_cmd,
		(unsigned) ds->shm_perm.uid,
		(unsigned) ds->shm_perm.gid,
		(unsigned) ds->shm_perm.mode,
		(unsigned) ds->shm_perm.__key,
		(unsigned) ds->shm_perm.cuid,
		(unsigned) ds->shm_perm.cgid,
		(unsigned) ds->shm_segsz,
		(int) ds->shm_cpid,
		(int) ds->shm_lpid,
		(unsigned) ds->shm_nattch,
		(unsigned) ds->shm_atime,
		(unsigned) ds->shm_dtime,
		(unsigned) ds->shm_ctime,
		rc);
}

static void
print_ipc_info(const char *const str_ipc_cmd,
               const struct shminfo *const info,
               const int rc)
{
	if (rc < 0) {
		printf("shmctl\\(%d, (%s\\|)?%s, %p\\) = %s\n",
		       id, str_ipc_64, str_ipc_cmd, info, sprintrc_grep(rc));
		return;
	}

	printf("shmctl\\(%d, (%s\\|)?%s, \\{shmmax=%llu, shmmin=%llu"
	       ", shmmni=%llu, shmseg=%llu, shmall=%llu\\}\\) = %d\n",
	       id,
	       str_ipc_64,
	       str_ipc_cmd,
	       (unsigned long long) info->shmmax,
	       (unsigned long long) info->shmmin,
	       (unsigned long long) info->shmmni,
	       (unsigned long long) info->shmseg,
	       (unsigned long long) info->shmall,
	       rc);
}

static void
print_shm_info(const char *const str_ipc_cmd,
              const struct shm_info *const info,
              const int rc)
{
	if (rc < 0) {
		printf("shmctl\\(%d, (%s\\|)?%s, %p\\) = %s\n",
		       id, str_ipc_64, str_ipc_cmd, info, sprintrc_grep(rc));
		return;
	}

	printf("shmctl\\(%d, (%s\\|)?%s, \\{used_ids=%d, shm_tot=%llu"
	       ", shm_rss=%llu, shm_swp=%llu, swap_attempts=%llu"
	       ", swap_successes=%llu\\}\\) = %d\n",
	       id,
	       str_ipc_64,
	       str_ipc_cmd,
	       info->used_ids,
	       (unsigned long long) info->shm_tot,
	       (unsigned long long) info->shm_rss,
	       (unsigned long long) info->shm_swp,
	       (unsigned long long) info->swap_attempts,
	       (unsigned long long) info->swap_successes,
	       rc);
}

int
main(void)
{
	static const key_t private_key =
		(key_t) (0xffffffff00000000ULL | IPC_PRIVATE);
	static const key_t bogus_key = (key_t) 0xeca86420fdb97531ULL;
#if TEST_SHMCTL_BOGUS_CMD || TEST_SHMCTL_BOGUS_ADDR
	static const int bogus_id = 0xdefaced1;
#endif
#if TEST_SHMCTL_BOGUS_CMD
	static const int bogus_cmd = 0xdefaced2;
#endif
#if TEST_SHMCTL_BOGUS_ADDR
	static void * const bogus_addr = (void *) -1L;
#endif
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
	union {
		struct shmid_ds ds;
		struct shminfo ipc_info;
		struct shm_info shm_info;
	} buf;

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

#if TEST_SHMCTL_BOGUS_CMD
	rc = shmctl(bogus_id, bogus_cmd, NULL);
	printf("shmctl\\(%d, (%s\\|)?%s, NULL\\) = %s\n",
	       bogus_id, str_ipc_64, str_bogus_cmd, sprintrc_grep(rc));
#endif

#if TEST_SHMCTL_BOGUS_ADDR
	rc = shmctl(bogus_id, IPC_STAT, bogus_addr);
	printf("shmctl\\(%d, (%s\\|)?%s, %p\\) = %s\n",
	       bogus_id, str_ipc_64, str_ipc_stat, bogus_addr,
	       sprintrc_grep(rc));
#endif

	rc = shmctl(id, IPC_STAT, &buf.ds);
	if (rc < 0)
		perror_msg_and_skip("shmctl IPC_STAT");
	print_shmid_ds(str_ipc_stat, &buf.ds, rc);

	if (shmctl(id, IPC_SET, &buf.ds))
		perror_msg_and_skip("shmctl IPC_SET");
	printf("shmctl\\(%d, (%s\\|)?%s, \\{shm_perm=\\{uid=%u, gid=%u"
	       ", mode=%#o\\}\\}\\) = 0\n",
	       id, str_ipc_64, str_ipc_set,
	       (unsigned) buf.ds.shm_perm.uid,
	       (unsigned) buf.ds.shm_perm.gid,
	       (unsigned) buf.ds.shm_perm.mode);

	rc = shmctl(id, IPC_INFO, &buf.ds);
	print_ipc_info(str_ipc_info, &buf.ipc_info, rc);

	rc = shmctl(id, SHM_INFO, &buf.ds);
	print_shm_info(str_shm_info, &buf.shm_info, rc);

	rc = shmctl(id, SHM_STAT, &buf.ds);
	print_shmid_ds(str_shm_stat, &buf.ds, rc);

	rc = shmctl(id, SHM_STAT_ANY, &buf.ds);
	print_shmid_ds(str_shm_stat_any, &buf.ds, rc);

	return 0;
}
