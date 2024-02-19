/*
 * Check decoding of statmount syscall.
 *
 * Copyright (c) 2024 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/mount.h>

#ifdef INJECT_RETVAL
# define INJ_STR " (INJECTED)\n"
#else
# define INJ_STR "\n"
#endif

#define VALID_STATMOUNT_STR	\
	"STATMOUNT_SB_BASIC|STATMOUNT_MNT_BASIC|STATMOUNT_PROPAGATE_FROM"	\
	"|STATMOUNT_MNT_ROOT|STATMOUNT_MNT_POINT|STATMOUNT_FS_TYPE"
#define INVALID_STATMOUNT	0xffffffffffffffc0
#define INVALID_STATMOUNT_STR	STRINGIFY_VAL(INVALID_STATMOUNT)
#define ALL_STATMOUNT_STR	VALID_STATMOUNT_STR "|" INVALID_STATMOUNT_STR

#define VALID_MOUNT_ATTR_STR	\
	"MOUNT_ATTR_RDONLY|MOUNT_ATTR_NOSUID|MOUNT_ATTR_NODEV"		\
	"|MOUNT_ATTR_NOEXEC|MOUNT_ATTR__ATIME|MOUNT_ATTR_NODIRATIME"	\
	"|MOUNT_ATTR_IDMAP|MOUNT_ATTR_NOSYMFOLLOW"
#define INVALID_MOUNT_ATTR	0xffffffffffcfff00
#define INVALID_MOUNT_ATTR_STR	STRINGIFY_VAL(INVALID_MOUNT_ATTR)
#define ALL_MOUNT_ATTR_STR	VALID_MOUNT_ATTR_STR "|" INVALID_MOUNT_ATTR_STR

#define VALID_SB_FLAGS_STR	\
	"MS_RDONLY|MS_SYNCHRONOUS|MS_DIRSYNC|MS_LAZYTIME"
#define INVALID_SB_FLAGS	0xfdffff6e
#define INVALID_SB_FLAGS_STR	STRINGIFY_VAL(INVALID_SB_FLAGS)
#define ALL_SB_FLAGS_STR	VALID_SB_FLAGS_STR "|" INVALID_SB_FLAGS_STR

#define VALID_MNT_PROPAGATION_STR	\
	"MS_UNBINDABLE|MS_PRIVATE|MS_SLAVE|MS_SHARED"
#define INVALID_MNT_PROPAGATION		0xffffffffffe1ffff
#define INVALID_MNT_PROPAGATION_STR	STRINGIFY_VAL(INVALID_MNT_PROPAGATION)
#define ALL_MNT_PROPAGATION_STR		\
	VALID_MNT_PROPAGATION_STR "|" INVALID_MNT_PROPAGATION_STR

static const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
static const char *errstr;

static long
k_statmount(const void *req, const void *buf,
	    const kernel_ulong_t bufsize, const unsigned int flags)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t arg1 = (uintptr_t) req;
	const kernel_ulong_t arg2 = (uintptr_t) buf;
	const kernel_ulong_t arg3 = bufsize;
	const kernel_ulong_t arg4 = fill | flags;
	const long rc = syscall(__NR_statmount,
				arg1, arg2, arg3, arg4, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	k_statmount(0, 0, 0, 0);
	printf("statmount(NULL, NULL, 0, 0) = %s" INJ_STR, errstr);

	struct mnt_id_req *const req = midtail_alloc(sizeof(*req), 8);
	fill_memory(req, sizeof(*req));
	const void *const bad_req = req + 1;

#define STR_0 "dummy"
#define STR_1 "procfs"
#define STR_2 "/root"
#define STR_3 "/relative"
	static const char str[] =
		STR_0 "\0"
		STR_1 "\0"
		STR_2 "\0"
		STR_3;

	struct statmount *const stm = midtail_alloc(sizeof(*stm), sizeof(str));
	fill_memory(stm, sizeof(*stm));
	const void *const bad_stm = (void *) stm + 1;

	k_statmount(bad_req, bad_stm, bad, -1U);
	printf("statmount(%p, %p, %ju, %#x) = %s" INJ_STR,
	       bad_req, bad_stm, (uintmax_t) bad, -1U, errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(typeof(req->size), size);
	const void *const bad_size = (void *) size + 1;

	k_statmount(bad_size, 0, 0, 0);
	printf("statmount(%p, NULL, 0, 0) = %s" INJ_STR, bad_size, errstr);

	*size = MNT_ID_REQ_SIZE_VER0 - 1;

	k_statmount(size, 0, 0, 0);
	printf("statmount({size=%u}, NULL, 0, 0) = %s" INJ_STR, *size, errstr);

	*size = MNT_ID_REQ_SIZE_VER0;

	k_statmount(size, 0, 0, 0);
	printf("statmount({size=%u, ???}, NULL, 0, 0) = %s" INJ_STR,
	       *size, errstr);

	req->size = sizeof(*req);
	req->param = INVALID_STATMOUNT;

	k_statmount(req, 0, 0, 0);
	printf("statmount({size=%u, spare=%#x, mnt_id=%#jx, param=%#jx"
	       " /* STATMOUNT_??? */}, NULL, 0, 0) = %s" INJ_STR,
	       req->size, req->spare,
	       (uintmax_t) req->mnt_id, (uintmax_t) req->param, errstr);

	req->spare = 0;
	req->param = 0x3f;

	k_statmount(req, 0, 0, 0);
	printf("statmount({size=%u, mnt_id=%#jx, param=%s}, NULL, 0, 0) = %s"
	       INJ_STR, req->size, (uintmax_t) req->mnt_id,
	       VALID_STATMOUNT_STR, errstr);

	req->param = (uint64_t) -1ULL;

	k_statmount(req, 0, 0, 0);
	printf("statmount({size=%u, mnt_id=%#jx, param=%s}, NULL, 0, 0) = %s"
	       INJ_STR, req->size, (uintmax_t) req->mnt_id,
	       ALL_STATMOUNT_STR, errstr);

	++req->size;
	req->param = 0;

	k_statmount(req, 0, 0, 0);
	printf("statmount({size=%u, mnt_id=%#jx, param=0, ???}, NULL, 0, 0)"
	       " = %s" INJ_STR, req->size, (uintmax_t) req->mnt_id, errstr);

	req->size = sizeof(*req) + 8;
	char *p = (char *) req - 8;
	memmove(p, req, sizeof(*req));
	fill_memory(p + sizeof(*req), 8);

	k_statmount(p, 0, 0, 0);
	memmove(req, p, sizeof(*req));
	printf("statmount({size=%u, mnt_id=%#jx, param=0"
	       ", /* bytes %zu..%zu */ \"%s\"}, NULL, 0, 0) = %s" INJ_STR,
	       req->size, (uintmax_t) req->mnt_id,
	       sizeof(*req), sizeof(*req) + 7,
	       "\\x80\\x81\\x82\\x83\\x84\\x85\\x86\\x87", errstr);

	k_statmount(0, stm, sizeof(stm->size) - 1, 0);
	printf("statmount(NULL, %p, %u, 0) = %s" INJ_STR,
	       stm, (unsigned int) sizeof(stm->size) - 1, errstr);

	stm->size = sizeof(*stm);

	k_statmount(0, bad_stm, stm->size, 0);
	printf("statmount(NULL, %p, %u, 0) = %s" INJ_STR,
	       bad_stm, stm->size, errstr);

	if (k_statmount(0, stm, sizeof(stm->size), 0) < 0)
		printf("statmount(NULL, %p, %u, 0) = %s" INJ_STR,
		       stm, (unsigned int) sizeof(stm->size), errstr);
	else
		printf("statmount(NULL, {size=%u, mask=0, sb_dev_major=0"
		       ", sb_dev_minor=0, sb_magic=0, sb_flags=0, fs_type=0"
		       ", mnt_id=0, mnt_parent_id=0, mnt_id_old=0"
		       ", mnt_parent_id_old=0, mnt_attr=0, mnt_propagation=0"
		       ", mnt_peer_group=0, mnt_master=0, propagate_from=0"
		       ", mnt_root=0, mnt_point=0}, %u, 0) = %s" INJ_STR,
		       stm->size, (unsigned int) sizeof(stm->size), errstr);

	stm->size = 0xfacefeed;
	stm->mask = (uint64_t) -1ULL;
	stm->sb_magic = 0x9fa0;
	stm->sb_flags = -1U;
	stm->mnt_attr = (uint64_t) -1ULL;
	stm->mnt_propagation = (uint64_t) -1ULL;

	if (k_statmount(0, stm, sizeof(*stm), 0) < 0)
		printf("statmount(NULL, %p, %u, 0) = %s" INJ_STR,
		       stm, (unsigned int) sizeof(*stm), errstr);
	else
		printf("statmount(NULL, {size=%u, mask=%s, sb_dev_major=%u"
		       ", sb_dev_minor=%u, sb_magic=%s, sb_flags=%s"
		       ", fs_type=%#x, mnt_id=%#jx, mnt_parent_id=%#jx"
		       ", mnt_id_old=%#x, mnt_parent_id_old=%#x, mnt_attr=%s"
		       ", mnt_propagation=%s, mnt_peer_group=%#jx"
		       ", mnt_master=%#jx, propagate_from=%#jx"
		       ", mnt_root=%#x, mnt_point=%#x}, %u, 0) = %s" INJ_STR,
		       stm->size,
		       ALL_STATMOUNT_STR,
		       stm->sb_dev_major,
		       stm->sb_dev_minor,
		       "PROC_SUPER_MAGIC",
		       ALL_SB_FLAGS_STR,
		       stm->fs_type,
		       (uintmax_t) stm->mnt_id,
		       (uintmax_t) stm->mnt_parent_id,
		       stm->mnt_id_old,
		       stm->mnt_parent_id_old,
		       ALL_MOUNT_ATTR_STR,
		       ALL_MNT_PROPAGATION_STR,
		       (uintmax_t) stm->mnt_peer_group,
		       (uintmax_t) stm->mnt_master,
		       (uintmax_t) stm->propagate_from,
		       stm->mnt_root,
		       stm->mnt_point,
		       (unsigned int) sizeof(*stm), errstr);


	*size = sizeof(*stm) + sizeof(str);
	stm->size = *size;
	stm->mask = INVALID_STATMOUNT;
	stm->sb_flags = INVALID_SB_FLAGS;
	stm->mnt_attr = INVALID_MOUNT_ATTR;
	stm->mnt_propagation = INVALID_MNT_PROPAGATION;
	stm->fs_type = sizeof(STR_0);
	stm->mnt_root = sizeof(STR_0) + sizeof(STR_1);
	stm->mnt_point = sizeof(STR_0) + sizeof(STR_1) + sizeof(STR_2);

	if (k_statmount(0, stm, stm->size, 0) < 0)
		printf("statmount(NULL, %p, %u, 0) = %s" INJ_STR,
		       stm, stm->size, errstr);
	else {
		printf("statmount(NULL, {size=%u, mask=%s, sb_dev_major=%u"
		       ", sb_dev_minor=%u, sb_magic=%s, sb_flags=%s"
		       ", fs_type=%#x, mnt_id=%#jx, mnt_parent_id=%#jx"
		       ", mnt_id_old=%#x, mnt_parent_id_old=%#x, mnt_attr=%s"
		       ", mnt_propagation=%s, mnt_peer_group=%#jx"
		       ", mnt_master=%#jx, propagate_from=%#jx"
		       ", mnt_root=%#x, mnt_point=%#x}, %u, 0) = %s"
		       INJ_STR,
		       stm->size,
		       INVALID_STATMOUNT_STR " /* STATMOUNT_??? */",
		       stm->sb_dev_major,
		       stm->sb_dev_minor,
		       "PROC_SUPER_MAGIC",
		       INVALID_SB_FLAGS_STR " /* MS_??? */",
		       stm->fs_type,
		       (uintmax_t) stm->mnt_id,
		       (uintmax_t) stm->mnt_parent_id,
		       stm->mnt_id_old,
		       stm->mnt_parent_id_old,
		       INVALID_MOUNT_ATTR_STR " /* MOUNT_ATTR_??? */",
		       INVALID_MNT_PROPAGATION_STR " /* MS_??? */",
		       (uintmax_t) stm->mnt_peer_group,
		       (uintmax_t) stm->mnt_master,
		       (uintmax_t) stm->propagate_from,
		       stm->mnt_root,
		       stm->mnt_point,
		       stm->size, errstr);
	}

	stm->mask = 0x3f;
	stm->sb_flags = 0x2000091;
	stm->mnt_attr = 0x3000ff;
	stm->mnt_propagation = 0x1e0000;

	p = (char *) stm - sizeof(str);
	memmove(p, stm, sizeof(*stm));
	memcpy(p + sizeof(*stm), str, sizeof(str) - 1);

	if (k_statmount(0, p, *size + 1, 0) < 0)
		printf("statmount(NULL, %p, %u, 0) = %s" INJ_STR,
		       p, *size + 1, errstr);
	else {
		memmove(stm, p, sizeof(*stm));
		printf("statmount(NULL, {size=%u, mask=%s, sb_dev_major=%u"
		       ", sb_dev_minor=%u, sb_magic=%s, sb_flags=%s"
		       ", fs_type=\"%s\", mnt_id=%#jx, mnt_parent_id=%#jx"
		       ", mnt_id_old=%#x, mnt_parent_id_old=%#x, mnt_attr=%s"
		       ", mnt_propagation=%s, mnt_peer_group=%#jx"
		       ", mnt_master=%#jx, propagate_from=%#jx"
		       ", mnt_root=\"%s\", mnt_point=\"%s\"...}, %u, 0) = %s"
		       INJ_STR,
		       stm->size,
		       VALID_STATMOUNT_STR,
		       stm->sb_dev_major,
		       stm->sb_dev_minor,
		       "PROC_SUPER_MAGIC",
		       VALID_SB_FLAGS_STR,
		       STR_1,
		       (uintmax_t) stm->mnt_id,
		       (uintmax_t) stm->mnt_parent_id,
		       stm->mnt_id_old,
		       stm->mnt_parent_id_old,
		       VALID_MOUNT_ATTR_STR,
		       VALID_MNT_PROPAGATION_STR,
		       (uintmax_t) stm->mnt_peer_group,
		       (uintmax_t) stm->mnt_master,
		       (uintmax_t) stm->propagate_from,
		       STR_2,
		       STR_3,
		       stm->size + 1, errstr);
	}

	puts("+++ exited with 0 +++");
	return 0;
}
