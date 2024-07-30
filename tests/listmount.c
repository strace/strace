/*
 * Check decoding of listmount syscall.
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

static const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
static const char *errstr;

static long
k_listmount(const void *req, const void *mnt_ids,
	    const kernel_ulong_t nr_mnt_ids, const unsigned int flags)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t arg1 = (uintptr_t) req;
	const kernel_ulong_t arg2 = (uintptr_t) mnt_ids;
	const kernel_ulong_t arg3 = nr_mnt_ids;
	const kernel_ulong_t arg4 = fill | flags;
	const long rc = syscall(__NR_listmount,
				arg1, arg2, arg3, arg4, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	k_listmount(0, 0, 0, 0);
	printf("listmount(NULL, NULL, 0, 0) = %s" INJ_STR, errstr);

	struct mnt_id_req *const req = midtail_alloc(sizeof(*req), 8);
	fill_memory(req, sizeof(*req));
	const void *const bad_req = req + 1;

	TAIL_ALLOC_OBJECT_CONST_ARR(uint64_t, mnt_ids, 2);
	fill_memory(mnt_ids, sizeof(*mnt_ids) * 2);
	const void *const bad_mnt_ids = mnt_ids + 2;

	k_listmount(bad_req, bad_mnt_ids, bad, -1U);
	printf("listmount(%p, %p, %ju, %s|%#x) = %s" INJ_STR,
	       bad_req, bad_mnt_ids, (uintmax_t) bad, "LISTMOUNT_REVERSE",
	       -1U & ~LISTMOUNT_REVERSE, errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(typeof(req->size), size);
	const void *const bad_size = (void *) size + 1;

	k_listmount(bad_size, 0, 0, 0);
	printf("listmount(%p, NULL, 0, 0) = %s" INJ_STR, bad_size, errstr);

	*size = MNT_ID_REQ_SIZE_VER0 - 1;
	k_listmount(size, 0, 0, 0);
	printf("listmount({size=%u}, NULL, 0, 0) = %s" INJ_STR, *size, errstr);

	*size = MNT_ID_REQ_SIZE_VER0;
	k_listmount(size, 0, 0, 0);
	printf("listmount({size=%u, ???}, NULL, 0, 0) = %s" INJ_STR,
	       *size, errstr);

	req->size = MNT_ID_REQ_SIZE_VER0;
	k_listmount(req, 0, 0, 0);
	printf("listmount({size=%u, spare=%#x, mnt_id=%#jx, param=%#jx}"
	       ", NULL, 0, 0) = %s" INJ_STR, req->size, req->spare,
	       (uintmax_t) req->mnt_id, (uintmax_t) req->param, errstr);

	req->size = MNT_ID_REQ_SIZE_VER1;
	k_listmount(req, 0, 0, 0);
	printf("listmount({size=%u, spare=%#x, mnt_id=%#jx, param=%#jx"
	       ", mnt_ns_id=%#jx}, NULL, 0, 0) = %s" INJ_STR, req->size,
	       req->spare, (uintmax_t) req->mnt_id, (uintmax_t) req->param,
	       (uintmax_t) req->mnt_ns_id, errstr);

	req->size = sizeof(*req);
	k_listmount(req, 0, 0, 0);
	printf("listmount({size=%u, spare=%#x, mnt_id=%#jx, param=%#jx"
	       ", mnt_ns_id=%#jx}, NULL, 0, 0) = %s" INJ_STR, req->size,
	       req->spare, (uintmax_t) req->mnt_id, (uintmax_t) req->param,
	       (uintmax_t) req->mnt_ns_id, errstr);

	req->spare = 0;
	k_listmount(req, 0, 0, 0);
	printf("listmount({size=%u, mnt_id=%#jx, param=%#jx, mnt_ns_id=%#jx}"
	       ", NULL, 0, 0) = %s" INJ_STR, req->size, (uintmax_t) req->mnt_id,
	       (uintmax_t) req->param, (uintmax_t) req->mnt_ns_id, errstr);

	req->mnt_id = LSMT_ROOT;
	k_listmount(req, 0, 0, 0);
	printf("listmount({size=%u, mnt_id=%s, param=%#jx, mnt_ns_id=%#jx}"
	       ", NULL, 0, 0) = %s" INJ_STR, req->size, "LSMT_ROOT",
	       (uintmax_t) req->param, (uintmax_t) req->mnt_ns_id, errstr);

	++req->size;
	k_listmount(req, 0, 0, 0);
	printf("listmount({size=%u, mnt_id=%s, param=%#jx, mnt_ns_id=%#jx, ???}"
	       ", NULL, 0, 0) = %s" INJ_STR, req->size, "LSMT_ROOT",
	       (uintmax_t) req->param, (uintmax_t) req->mnt_ns_id, errstr);

	req->size = sizeof(*req) + 8;
	char *p = (char *) req - 8;
	memmove(p, req, sizeof(*req));
	fill_memory(p + sizeof(*req), 8);
	k_listmount(p, 0, 0, 0);
	memmove(req, p, sizeof(*req));
	printf("listmount({size=%u, mnt_id=%s, param=%#jx, mnt_ns_id=%#jx"
	       ", /* bytes %zu..%zu */ \"%s\"}, NULL, 0, 0) = %s" INJ_STR,
	       req->size, "LSMT_ROOT", (uintmax_t) req->param,
	       (uintmax_t) req->mnt_ns_id, sizeof(*req), sizeof(*req) + 7,
	       "\\x80\\x81\\x82\\x83\\x84\\x85\\x86\\x87", errstr);

	k_listmount(0, mnt_ids, 0, 0);
	printf("listmount(NULL, [], 0, 0) = %s" INJ_STR, errstr);

	if (k_listmount(0, mnt_ids, 1, 0) < 0)
		printf("listmount(NULL, %p, 1, 0) = %s" INJ_STR,
		       mnt_ids, errstr);
	else
		printf("listmount(NULL, [%#jx], 1, 0) = %s" INJ_STR,
		       (uintmax_t) mnt_ids[0], errstr);

	if (k_listmount(0, mnt_ids, 2, 0) < 0)
		printf("listmount(NULL, %p, 2, 0) = %s" INJ_STR,
		       mnt_ids, errstr);
	else
		printf("listmount(NULL, [%#jx, %#jx], 2, 0) = %s" INJ_STR,
		       (uintmax_t) mnt_ids[0], (uintmax_t) mnt_ids[1], errstr);

	if (k_listmount(0, mnt_ids, 3, 0) < 0)
		printf("listmount(NULL, %p, 3, 0) = %s" INJ_STR,
		       mnt_ids, errstr);
	else
		printf("listmount(NULL, [%#jx, %#jx, ... /* %p */], 3, 0) = %s"
		       INJ_STR, (uintmax_t) mnt_ids[0], (uintmax_t) mnt_ids[1],
		       bad_mnt_ids, errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
