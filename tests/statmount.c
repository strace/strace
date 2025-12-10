/*
 * Check decoding of statmount syscall.
 *
 * Copyright (c) 2024-2025 Dmitry V. Levin <ldv@strace.io>
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

#define VALID_STATMOUNT		0x7fff
#define VALID_STATMOUNT_STR							\
	"STATMOUNT_SB_BASIC|STATMOUNT_MNT_BASIC|STATMOUNT_PROPAGATE_FROM"	\
	"|STATMOUNT_MNT_ROOT|STATMOUNT_MNT_POINT|STATMOUNT_FS_TYPE"		\
	"|STATMOUNT_MNT_NS_ID|STATMOUNT_MNT_OPTS|STATMOUNT_FS_SUBTYPE"		\
	"|STATMOUNT_SB_SOURCE|STATMOUNT_OPT_ARRAY|STATMOUNT_OPT_SEC_ARRAY"	\
	"|STATMOUNT_SUPPORTED_MASK|STATMOUNT_MNT_UIDMAP|STATMOUNT_MNT_GIDMAP"	\
	/* End of VALID_STATMOUNT_STR */
#define INVALID_STATMOUNT	0xffffffffffff8000
#define INVALID_STATMOUNT_STR	STRINGIFY_VAL(INVALID_STATMOUNT)
#define ALL_STATMOUNT	((uint64_t) 0xffffffffffffffffULL)
#define ALL_STATMOUNT_STR	VALID_STATMOUNT_STR "|" INVALID_STATMOUNT_STR

#define VALID_SB_MAGIC	0x9fa0
#define VALID_SB_MAGIC_STR	"PROC_SUPER_MAGIC"
#define INVALID_SB_MAGIC	0xfacefeeddeadbeef
#define INVALID_SB_MAGIC_STR	STRINGIFY_VAL(INVALID_SB_MAGIC)

#define VALID_SB_FLAGS	0x2000091
#define VALID_SB_FLAGS_STR	\
	"MS_RDONLY|MS_SYNCHRONOUS|MS_DIRSYNC|MS_LAZYTIME"
#define INVALID_SB_FLAGS	0xfdffff6e
#define INVALID_SB_FLAGS_STR	STRINGIFY_VAL(INVALID_SB_FLAGS)
#define ALL_SB_FLAGS		0xffffffff
#define ALL_SB_FLAGS_STR	VALID_SB_FLAGS_STR "|" INVALID_SB_FLAGS_STR

#define VALID_MOUNT_ATTR	0x3000ff
#define VALID_MOUNT_ATTR_STR	\
	"MOUNT_ATTR_RDONLY|MOUNT_ATTR_NOSUID|MOUNT_ATTR_NODEV"		\
	"|MOUNT_ATTR_NOEXEC|MOUNT_ATTR__ATIME|MOUNT_ATTR_NODIRATIME"	\
	"|MOUNT_ATTR_IDMAP|MOUNT_ATTR_NOSYMFOLLOW"
#define INVALID_MOUNT_ATTR	0xffffffffffcfff00
#define INVALID_MOUNT_ATTR_STR	STRINGIFY_VAL(INVALID_MOUNT_ATTR)
#define ALL_MOUNT_ATTR	0xffffffffffffffff
#define ALL_MOUNT_ATTR_STR	VALID_MOUNT_ATTR_STR "|" INVALID_MOUNT_ATTR_STR

#define VALID_MNT_PROPAGATION	0x1e0000
#define VALID_MNT_PROPAGATION_STR	\
	"MS_UNBINDABLE|MS_PRIVATE|MS_SLAVE|MS_SHARED"
#define INVALID_MNT_PROPAGATION		0xffffffffffe1ffff
#define INVALID_MNT_PROPAGATION_STR	STRINGIFY_VAL(INVALID_MNT_PROPAGATION)
#define ALL_MNT_PROPAGATION	0xffffffffffffffff
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

static void
test_req(void)
{
	struct mnt_id_req *const req = midtail_alloc(sizeof(*req), 8);
	fill_memory(req, sizeof(*req));
	const void *const bad_req = req + 1;

	k_statmount(bad_req, 0, bad, -1U);
	printf("statmount(%p, NULL, %ju, %#x) = %s" INJ_STR,
	       bad_req, (uintmax_t) bad, -1U, errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(typeof(req->size), req_size);
	const void *const bad_size = (void *) req_size + 1;

	k_statmount(bad_size, 0, 0, 0);
	printf("statmount(%p, NULL, 0, 0) = %s" INJ_STR, bad_size, errstr);

	*req_size = MNT_ID_REQ_SIZE_VER0 - 1;

	k_statmount(req_size, 0, 0, 0);
	printf("statmount({size=%u}, NULL, 0, 0) = %s" INJ_STR, *req_size, errstr);

	*req_size = MNT_ID_REQ_SIZE_VER0;

	k_statmount(req_size, 0, 0, 0);
	printf("statmount({size=%u, ???}, NULL, 0, 0) = %s" INJ_STR,
	       *req_size, errstr);

	req->size = MNT_ID_REQ_SIZE_VER0;
	req->param = INVALID_STATMOUNT;

	k_statmount(req, 0, 0, 0);
	printf("statmount({size=%u, mnt_ns_fd=%d, mnt_id=%#jx"
	       ", param=%#jx /* STATMOUNT_??? */}, NULL, 0, 0) = %s" INJ_STR,
	       req->size, req->mnt_ns_fd,
	       (uintmax_t) req->mnt_id, (uintmax_t) req->param, errstr);

	req->size = MNT_ID_REQ_SIZE_VER1;
	req->mnt_ns_fd = 0;
	req->param = VALID_STATMOUNT;

	k_statmount(req, 0, 0, 0);
	printf("statmount({size=%u, mnt_ns_fd=%d, mnt_id=%#jx, param=%s"
	       ", mnt_ns_id=%#jx}, NULL, 0, 0) = %s" INJ_STR,
	       req->size, req->mnt_ns_fd, (uintmax_t) req->mnt_id,
	       VALID_STATMOUNT_STR, (uintmax_t) req->mnt_ns_id, errstr);

	req->size = sizeof(*req);
	req->param = ALL_STATMOUNT;

	k_statmount(req, 0, 0, 0);
	printf("statmount({size=%u, mnt_ns_fd=%d, mnt_id=%#jx, param=%s"
	       ", mnt_ns_id=%#jx}, NULL, 0, 0) = %s" INJ_STR,
	       req->size, req->mnt_ns_fd, (uintmax_t) req->mnt_id,
	       ALL_STATMOUNT_STR, (uintmax_t) req->mnt_ns_id, errstr);

	++req->size;
	req->param = 0;

	k_statmount(req, 0, 0, 0);
	printf("statmount({size=%u, mnt_ns_fd=%d, mnt_id=%#jx, param=0"
	       ", mnt_ns_id=%#jx, ???}, NULL, 0, 0) = %s" INJ_STR, req->size,
	       req->mnt_ns_fd, (uintmax_t) req->mnt_id,
	       (uintmax_t) req->mnt_ns_id, errstr);

	req->size = sizeof(*req) + 8;
	char *p = (char *) req - 8;
	memmove(p, req, sizeof(*req));
	fill_memory(p + sizeof(*req), 8);

	k_statmount(p, 0, 0, 0);
	memmove(req, p, sizeof(*req));
	printf("statmount({size=%u, mnt_ns_fd=%d, mnt_id=%#jx, param=0"
	       ", mnt_ns_id=%#jx, /* bytes %zu..%zu */ \"%s\"}, NULL, 0, 0)"
	       " = %s" INJ_STR, req->size, req->mnt_ns_fd,
	       (uintmax_t) req->mnt_id, (uintmax_t) req->mnt_ns_id,
	       sizeof(*req), sizeof(*req) + 7,
	       "\\x80\\x81\\x82\\x83\\x84\\x85\\x86\\x87", errstr);
}

static void
test_stm_bad(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(struct statmount, stm);
	const void *const bad_stm = (void *) stm + 1;

	k_statmount(0, bad_stm, bad, -1U);
	printf("statmount(NULL, %p, %ju, %#x) = %s" INJ_STR,
	       bad_stm, (uintmax_t) bad, -1U, errstr);

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
		printf("statmount(NULL, {size=%u, mask=0}, %u, 0) = %s" INJ_STR,
		       stm->size, (unsigned int) sizeof(stm->size), errstr);
}

struct stm_ops_t {
	uint64_t val;
	const char *str;
	const char *exp;
	struct statmount stm;
};
#define STM_ARG_STR(arg_) .val = (arg_), .str = #arg_

static void
test_stm_all_ops(void)
{
	static const struct stm_ops_t all_ops[] = {
		{
			STM_ARG_STR(0)
		}, {
			STM_ARG_STR(STATMOUNT_SB_BASIC),
			.stm = {
				.sb_dev_major = 2475856272,
				.sb_dev_minor = 2543228308,
				.sb_magic = VALID_SB_MAGIC,
				.sb_flags = INVALID_SB_FLAGS,
			},
			.exp = ", sb_dev_major=2475856272"
			       ", sb_dev_minor=2543228308"
			       ", sb_magic=" VALID_SB_MAGIC_STR
			       ", sb_flags=" INVALID_SB_FLAGS_STR " /* MS_??? */",
		}, {
			STM_ARG_STR(STATMOUNT_SB_BASIC),
			.stm = {
				.size = 0xfacefeed,
				.sb_dev_major = 2475856272,
				.sb_dev_minor = 2543228308,
				.sb_magic = INVALID_SB_MAGIC,
				.sb_flags = VALID_SB_FLAGS,
			},
			.exp = ", sb_dev_major=2475856272"
			       ", sb_dev_minor=2543228308"
			       ", sb_magic=" INVALID_SB_MAGIC_STR
			       ", sb_flags=" VALID_SB_FLAGS_STR,
		}, {
			STM_ARG_STR(STATMOUNT_MNT_BASIC),
			.stm = {
				.mnt_id = 0xafaeadacabaaa9a8,
				.mnt_parent_id = 0xb7b6b5b4b3b2b1b0,
				.mnt_id_old = 0xbbbab9b8,
				.mnt_parent_id_old = 0xbfbebdbc,
				.mnt_attr = VALID_MOUNT_ATTR,
				.mnt_propagation = INVALID_MNT_PROPAGATION,
				.mnt_peer_group = 0xd7d6d5d4d3d2d1d0,
				.mnt_master = 0xdfdedddcdbdad9d8,
			},
			.exp = ", mnt_id=0xafaeadacabaaa9a8"
			       ", mnt_parent_id=0xb7b6b5b4b3b2b1b0"
			       ", mnt_id_old=0xbbbab9b8"
			       ", mnt_parent_id_old=0xbfbebdbc"
			       ", mnt_attr=" VALID_MOUNT_ATTR_STR
			       ", mnt_propagation=" INVALID_MNT_PROPAGATION_STR " /* MS_??? */"
			       ", mnt_peer_group=0xd7d6d5d4d3d2d1d0"
			       ", mnt_master=0xdfdedddcdbdad9d8",
		}, {
			STM_ARG_STR(STATMOUNT_MNT_BASIC),
			.stm = {
				.size = 0xfacefeed,
				.mnt_id = 0xafaeadacabaaa9a8,
				.mnt_parent_id = 0xb7b6b5b4b3b2b1b0,
				.mnt_id_old = 0xbbbab9b8,
				.mnt_parent_id_old = 0xbfbebdbc,
				.mnt_attr = INVALID_MOUNT_ATTR,
				.mnt_propagation = VALID_MNT_PROPAGATION,
				.mnt_peer_group = 0xd7d6d5d4d3d2d1d0,
				.mnt_master = 0xdfdedddcdbdad9d8,
			},
			.exp = ", mnt_id=0xafaeadacabaaa9a8"
			       ", mnt_parent_id=0xb7b6b5b4b3b2b1b0"
			       ", mnt_id_old=0xbbbab9b8"
			       ", mnt_parent_id_old=0xbfbebdbc"
			       ", mnt_attr=" INVALID_MOUNT_ATTR_STR " /* MOUNT_ATTR_??? */"
			       ", mnt_propagation=" VALID_MNT_PROPAGATION_STR
			       ", mnt_peer_group=0xd7d6d5d4d3d2d1d0"
			       ", mnt_master=0xdfdedddcdbdad9d8",
		}, {
			STM_ARG_STR(STATMOUNT_PROPAGATE_FROM),
			.stm = { .propagate_from = 0xe7e6e5e4e3e2e1e0 },
			.exp = ", propagate_from=0xe7e6e5e4e3e2e1e0",
		}, {
			STM_ARG_STR(STATMOUNT_MNT_ROOT),
			.stm = { .mnt_root = 0xebeae9e8 },
			.exp = ", mnt_root=0xebeae9e8",
		}, {
			STM_ARG_STR(STATMOUNT_MNT_POINT),
			.stm = { .mnt_point = 0xefeeedec },
			.exp = ", mnt_point=0xefeeedec",
		}, {
			STM_ARG_STR(STATMOUNT_FS_TYPE),
			.stm = { .fs_type = 0xa7a6a5a4 },
			.exp = ", fs_type=0xa7a6a5a4",
		}, {
			STM_ARG_STR(STATMOUNT_MNT_NS_ID),
			.stm = { .mnt_ns_id = 0xf7f6f5f4f3f2f1f0 },
			.exp = ", mnt_ns_id=0xf7f6f5f4f3f2f1f0",
		}, {
			STM_ARG_STR(STATMOUNT_MNT_OPTS),
			.stm = { .mnt_opts = 0x87868584 },
			.exp = ", mnt_opts=0x87868584",
		}, {
			STM_ARG_STR(STATMOUNT_FS_SUBTYPE),
			.stm = { .fs_subtype = 0xfbfaf9f8 },
			.exp = ", fs_subtype=0xfbfaf9f8",
		}, {
			STM_ARG_STR(STATMOUNT_SB_SOURCE),
			.stm = { .sb_source = 0xfffefdfc },
			.exp = ", sb_source=0xfffefdfc",
		}, {
			STM_ARG_STR(STATMOUNT_OPT_ARRAY),
			.stm = {
				.opt_num = 2206368128,
				.opt_array = 0x87868584,
			},
			.exp = ", opt_num=2206368128, opt_array=0x87868584",
		}, {
			STM_ARG_STR(STATMOUNT_OPT_SEC_ARRAY),
			.stm = {
				.opt_sec_num = 2341112200,
				.opt_sec_array = 0x8f8e8d8c,
			},
			.exp = ", opt_sec_num=2341112200"
			       ", opt_sec_array=0x8f8e8d8c"
		}, {
			STM_ARG_STR(STATMOUNT_SUPPORTED_MASK),
			.stm = { .supported_mask = VALID_STATMOUNT },
			.exp = ", supported_mask=" VALID_STATMOUNT_STR
		}, {
			STM_ARG_STR(STATMOUNT_SUPPORTED_MASK),
			.stm = {
				.size = 0xfacefeed,
				.supported_mask = ALL_STATMOUNT
			},
			.exp = ", supported_mask=" ALL_STATMOUNT_STR
		}, {
			STM_ARG_STR(STATMOUNT_MNT_UIDMAP),
			.stm = {
				.mnt_uidmap_num = 2610600344,
				.mnt_uidmap = 0x9f9e9d9c,
			},
			.exp = ", mnt_uidmap_num=2610600344"
			       ", mnt_uidmap=0x9f9e9d9c"
		}, {
			STM_ARG_STR(STATMOUNT_MNT_GIDMAP),
			.stm = {
				.mnt_gidmap_num = 2745344416,
				.mnt_gidmap = 0xa7a6a5a4,
			},
			.exp = ", mnt_gidmap_num=2745344416"
			       ", mnt_gidmap=0xa7a6a5a4"
		}, {
			STM_ARG_STR(STATMOUNT_SB_BASIC|STATMOUNT_MNT_BASIC),
			.stm = {
				.sb_dev_major = 2475856272,
				.sb_dev_minor = 2543228308,
				.sb_magic = VALID_SB_MAGIC,
				.sb_flags = ALL_SB_FLAGS,
				.mnt_id = 0xafaeadacabaaa9a8,
				.mnt_parent_id = 0xb7b6b5b4b3b2b1b0,
				.mnt_id_old = 0xbbbab9b8,
				.mnt_parent_id_old = 0xbfbebdbc,
				.mnt_attr = ALL_MOUNT_ATTR,
				.mnt_propagation = ALL_MNT_PROPAGATION,
				.mnt_peer_group = 0xd7d6d5d4d3d2d1d0,
				.mnt_master = 0xdfdedddcdbdad9d8,
			},
			.exp = ", sb_dev_major=2475856272"
			       ", sb_dev_minor=2543228308"
			       ", sb_magic=" VALID_SB_MAGIC_STR
			       ", sb_flags=" ALL_SB_FLAGS_STR
			       ", mnt_id=0xafaeadacabaaa9a8"
			       ", mnt_parent_id=0xb7b6b5b4b3b2b1b0"
			       ", mnt_id_old=0xbbbab9b8"
			       ", mnt_parent_id_old=0xbfbebdbc"
			       ", mnt_attr=" ALL_MOUNT_ATTR_STR
			       ", mnt_propagation=" ALL_MNT_PROPAGATION_STR
			       ", mnt_peer_group=0xd7d6d5d4d3d2d1d0"
			       ", mnt_master=0xdfdedddcdbdad9d8",
		}, {
			.val = INVALID_STATMOUNT,
			.str = INVALID_STATMOUNT_STR " /* STATMOUNT_??? */"
		}
	};

	TAIL_ALLOC_OBJECT_CONST_PTR(struct statmount, stm);

	for (unsigned int i = 0; i < ARRAY_SIZE(all_ops); ++i) {
		*stm = all_ops[i].stm;
		if (!stm->size)
			stm->size = sizeof(*stm);
		stm->mask = all_ops[i].val;
		if (k_statmount(0, stm, stm->size, 0) < 0) {
			printf("statmount(NULL, %p, %u, 0) = %s" INJ_STR,
			       stm, stm->size, errstr);
		} else {
			printf("statmount(NULL, {size=%u", stm->size);

			if (all_ops[i].exp && stm->mask == STATMOUNT_MNT_OPTS)
				printf("%s", all_ops[i].exp);

			printf(", mask=%s", all_ops[i].str);

			if (all_ops[i].exp && stm->mask != STATMOUNT_MNT_OPTS)
				printf("%s", all_ops[i].exp);

			printf("}, %u, 0) = %s" INJ_STR, stm->size, errstr);
		}
	}
}

static void
test_stm_str_ops(void)
{
#define STR_0 "dummy"
#define STR_1 "bar"

	static const struct stm_ops_t str_ops[] = {
		{
			STM_ARG_STR(STATMOUNT_MNT_ROOT),
			.stm = { .mnt_root = sizeof(STR_0) },
			.exp = ", mnt_root=\"" STR_1 "\"",
		}, {
			STM_ARG_STR(STATMOUNT_MNT_POINT),
			.stm = { .mnt_point = sizeof(STR_0) },
			.exp = ", mnt_point=\"" STR_1 "\"",
		}, {
			STM_ARG_STR(STATMOUNT_FS_TYPE),
			.stm = { .fs_type = sizeof(STR_0) },
			.exp = ", fs_type=\"" STR_1 "\"",
		}, {
			STM_ARG_STR(STATMOUNT_MNT_OPTS),
			.stm = { .mnt_opts = sizeof(STR_0) },
			.exp = ", mnt_opts=\"" STR_1 "\"",
		}, {
			STM_ARG_STR(STATMOUNT_FS_SUBTYPE),
			.stm = { .fs_subtype = sizeof(STR_0) },
			.exp = ", fs_subtype=\"" STR_1 "\"",
		}, {
			STM_ARG_STR(STATMOUNT_SB_SOURCE),
			.stm = { .sb_source = sizeof(STR_0) },
			.exp = ", sb_source=\"" STR_1 "\"",
		}
	};

	static const char str[] = STR_0 "\0" STR_1;
	struct statmount *const stm = midtail_alloc(sizeof(*stm), sizeof(str));
	const unsigned int stm_alloc_size = sizeof(*stm) + sizeof(str);
	const unsigned int buf_size = stm_alloc_size + 1;

	for (unsigned int i = 0; i < ARRAY_SIZE(str_ops); ++i) {
		*stm = str_ops[i].stm;
		stm->size = stm_alloc_size;
		stm->mask = str_ops[i].val;

		char *p = (char *) stm - sizeof(str);
		memmove(p, stm, sizeof(*stm));
		memcpy(p + sizeof(*stm), str, sizeof(str));

		if (k_statmount(0, p, buf_size, 0) < 0) {
			printf("statmount(NULL, %p, %u, 0) = %s" INJ_STR,
			       p, buf_size, errstr);
		} else {
			printf("statmount(NULL, {size=%u", stm_alloc_size);

			if (str_ops[i].exp && str_ops[i].val == STATMOUNT_MNT_OPTS)
				printf("%s", str_ops[i].exp);

			printf(", mask=%s", str_ops[i].str);

			if (str_ops[i].exp && str_ops[i].val != STATMOUNT_MNT_OPTS)
				printf("%s", str_ops[i].exp);

			printf("}, %u, 0) = %s" INJ_STR, buf_size, errstr);
		}

		p[stm_alloc_size - 1] = '!';

		if (k_statmount(0, p, buf_size, 0) < 0) {
			printf("statmount(NULL, %p, %u, 0) = %s" INJ_STR,
			       p, buf_size, errstr);
		} else {
			printf("statmount(NULL, {size=%u", stm_alloc_size);

			if (str_ops[i].exp && str_ops[i].val == STATMOUNT_MNT_OPTS)
				printf("%s...", str_ops[i].exp);

			printf(", mask=%s", str_ops[i].str);

			if (str_ops[i].exp && str_ops[i].val != STATMOUNT_MNT_OPTS)
				printf("%s...", str_ops[i].exp);

			printf("}, %u, 0) = %s" INJ_STR, buf_size, errstr);
		}
	}

#undef STR_1
#undef STR_0
}

static void
test_stm_array_ops(void)
{
#define STR_0	"A\0B\0C\0D\0E\0F\0G\0H\0I\0J\0K\0L\0M\0N\0O\0P\0" \
		"Q\0R\0S\0T\0U\0V\0W\0X\0Y\0Z\0a\0b\0c\0d\0e"
#define STR_1	"first"
#define STR_2	"second"

	static const struct {
		uint64_t val;
		const char *str;
		const char *exp;
		const char *exp2;
		struct statmount stm;
	} array_ops[] = {
		{
			STM_ARG_STR(STATMOUNT_OPT_ARRAY),
			.stm = {
				.opt_num = 2,
				.opt_array = sizeof(STR_0),
			},
			.exp  = ", opt_num=2"
				", opt_array=[\"" STR_1 "\", \"" STR_2 "\"]",
			.exp2 =	", opt_num=2"
				", opt_array=[\"" STR_1 "\", \"" STR_2 "\"...]",
		}, {
			STM_ARG_STR(STATMOUNT_OPT_ARRAY),
			.stm = {
				.opt_num = 3,
				.opt_array = sizeof(STR_0),
			},
			.exp  = ", opt_num=3"
				", opt_array=[\"" STR_1 "\", \"" STR_2 "\", ???]",
			.exp2 =	", opt_num=3"
				", opt_array=[\"" STR_1 "\", \"" STR_2 "\"..., ???]",
		}, {
			STM_ARG_STR(STATMOUNT_OPT_SEC_ARRAY),
			.stm = {
				.opt_sec_num = 2,
				.opt_sec_array = sizeof(STR_0),
			},
			.exp  = ", opt_sec_num=2"
				", opt_sec_array=[\"" STR_1 "\", \"" STR_2 "\"]",
			.exp2 = ", opt_sec_num=2"
				", opt_sec_array=[\"" STR_1 "\", \"" STR_2 "\"...]",
		}, {
			STM_ARG_STR(STATMOUNT_OPT_SEC_ARRAY),
			.stm = {
				.opt_sec_num = 0,
				.opt_sec_array = sizeof(STR_0),
			},
			.exp  = ", opt_sec_num=0"
				", opt_sec_array=[]",
			.exp2 = ", opt_sec_num=0"
				", opt_sec_array=[]",
		}, {
			STM_ARG_STR(STATMOUNT_MNT_UIDMAP),
			.stm = {
				.mnt_uidmap_num = 2,
				.mnt_uidmap = sizeof(STR_0),
			},
			.exp  =	", mnt_uidmap_num=2"
				", mnt_uidmap=[\"" STR_1 "\", \"" STR_2 "\"]",
			.exp2 =	", mnt_uidmap_num=2"
				", mnt_uidmap=[\"" STR_1 "\", \"" STR_2 "\"...]",
		}, {
			STM_ARG_STR(STATMOUNT_MNT_UIDMAP),
			.stm = {
				.mnt_uidmap_num = 33,
				.mnt_uidmap = 0,
			},
			.exp  =	", mnt_uidmap_num=33"
				", mnt_uidmap="
				"[\"A\", \"B\", \"C\", \"D\""
				", \"E\", \"F\", \"G\", \"H\""
				", \"I\", \"J\", \"K\", \"L\""
				", \"M\", \"N\", \"O\", \"P\""
				", \"Q\", \"R\", \"S\", \"T\""
				", \"U\", \"V\", \"W\", \"X\""
				", \"Y\", \"Z\", \"a\", \"b\""
				", \"c\", \"d\", \"e\", \"" STR_1 "\", ...]",
			.exp2 =	", mnt_uidmap_num=33"
				", mnt_uidmap="
				"[\"A\", \"B\", \"C\", \"D\""
				", \"E\", \"F\", \"G\", \"H\""
				", \"I\", \"J\", \"K\", \"L\""
				", \"M\", \"N\", \"O\", \"P\""
				", \"Q\", \"R\", \"S\", \"T\""
				", \"U\", \"V\", \"W\", \"X\""
				", \"Y\", \"Z\", \"a\", \"b\""
				", \"c\", \"d\", \"e\", \"" STR_1 "\", ...]",
		}, {
			STM_ARG_STR(STATMOUNT_MNT_GIDMAP),
			.stm = {
				.mnt_gidmap_num = 2,
				.mnt_gidmap = sizeof(STR_0),
			},
			.exp  =	", mnt_gidmap_num=2"
				", mnt_gidmap=[\"" STR_1 "\", \"" STR_2 "\"]",
			.exp2 =	", mnt_gidmap_num=2"
				", mnt_gidmap=[\"" STR_1 "\", \"" STR_2 "\"...]",
		}
	};

	static const char str[] = STR_0 "\0" STR_1 "\0" STR_2;
	struct statmount *const stm = midtail_alloc(sizeof(*stm), sizeof(str));
	const unsigned int stm_alloc_size = sizeof(*stm) + sizeof(str);
	const unsigned int buf_size = stm_alloc_size + 1;

	for (unsigned int i = 0; i < ARRAY_SIZE(array_ops); ++i) {
		*stm = array_ops[i].stm;
		stm->size = stm_alloc_size;
		stm->mask = array_ops[i].val;

		char *p = (char *) stm - sizeof(str);
		memmove(p, stm, sizeof(*stm));
		memcpy(p + sizeof(*stm), str, sizeof(str));

		if (k_statmount(0, p, buf_size, 0) < 0) {
			printf("statmount(NULL, %p, %u, 0) = %s" INJ_STR,
			       p, buf_size, errstr);
		} else {
			printf("statmount(NULL, {size=%u, mask=%s%s}, %u, 0)"
			       " = %s" INJ_STR,
			       stm_alloc_size, array_ops[i].str,
			       array_ops[i].exp, buf_size, errstr);
		}

		if (!array_ops[i].exp2)
			continue;

		p[stm_alloc_size - 1] = '!';

		if (k_statmount(0, p, buf_size, 0) < 0) {
			printf("statmount(NULL, %p, %u, 0) = %s" INJ_STR,
			       p, buf_size, errstr);
		} else {
			printf("statmount(NULL, {size=%u, mask=%s%s}, %u, 0)"
			       " = %s" INJ_STR,
			       stm_alloc_size, array_ops[i].str,
			       array_ops[i].exp2, buf_size, errstr);
		}

	}

#undef STR_2
#undef STR_1
#undef STR_0
}

int
main(void)
{
	k_statmount(0, 0, 0, 0);
	printf("statmount(NULL, NULL, 0, 0) = %s" INJ_STR, errstr);

	test_req();
	test_stm_bad();
	test_stm_all_ops();
	test_stm_str_ops();
	test_stm_array_ops();

	puts("+++ exited with 0 +++");
	return 0;
}
