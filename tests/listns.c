/*
 * Check decoding of listns syscall.
 *
 * Copyright (c) 2024-2026 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/nsfs.h>

#ifdef INJECT_RETVAL
# define INJ_STR " (INJECTED)\n"
#else
# define INJ_STR "\n"
#endif

static const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
static const char *errstr;

static long
k_listns(const void *req, const void *ns_ids,
	 const kernel_ulong_t nr_ns_ids, const unsigned int flags)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t arg1 = (uintptr_t) req;
	const kernel_ulong_t arg2 = (uintptr_t) ns_ids;
	const kernel_ulong_t arg3 = nr_ns_ids;
	const kernel_ulong_t arg4 = fill | flags;
	const long rc = syscall(__NR_listns,
				arg1, arg2, arg3, arg4, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	k_listns(0, 0, 0, 0);
	printf("listns(NULL, NULL, 0, 0) = %s" INJ_STR, errstr);

	struct ns_id_req *const req = midtail_alloc(sizeof(*req), 8);
	fill_memory(req, sizeof(*req));
	const void *const bad_req = req + 1;

	TAIL_ALLOC_OBJECT_CONST_ARR(uint64_t, ns_ids, 2);
	fill_memory(ns_ids, sizeof(*ns_ids) * 2);
	const void *const bad_ns_ids = ns_ids + 2;

	k_listns(bad_req, bad_ns_ids, bad, -1U);
	printf("listns(%p, %p, %ju, %#x) = %s" INJ_STR,
	       bad_req, bad_ns_ids, (uintmax_t) bad, -1U, errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(typeof(req->size), size);
	const void *const bad_size = (void *) size + 1;

	k_listns(bad_size, 0, 0, 0);
	printf("listns(%p, NULL, 0, 0) = %s" INJ_STR, bad_size, errstr);

	*size = NS_ID_REQ_SIZE_VER0 - 1;
	k_listns(size, 0, 0, 0);
	printf("listns({size=%u}, NULL, 0, 0) = %s" INJ_STR, *size, errstr);

	*size = NS_ID_REQ_SIZE_VER0;
	k_listns(size, 0, 0, 0);
	printf("listns({size=%u, ???}, NULL, 0, 0) = %s" INJ_STR,
	       *size, errstr);

	req->size = NS_ID_REQ_SIZE_VER0;
	req->ns_type = -1;
	static const struct strval32 valid_ns_types =
		{ ARG_STR(TIME_NS|MNT_NS|CGROUP_NS|UTS_NS|IPC_NS|USER_NS|PID_NS|NET_NS) };
	k_listns(req, 0, 0, 0);
	printf("listns({size=%u, spare=%#x, ns_id=%#jx, ns_type=%s|%#x"
	       ", spare2=%#x, user_ns_id=%#jx}, NULL, 0, 0) = %s" INJ_STR,
	       req->size, req->spare, (uintmax_t) req->ns_id,
	       valid_ns_types.str, (-1U & ~valid_ns_types.val),
	       req->spare2, (uintmax_t) req->user_ns_id, errstr);

	req->spare = 0;
	req->ns_type = 0;
	req->spare2 = 0;
	req->user_ns_id = 0;
	k_listns(req, 0, 0, 0);
	printf("listns({size=%u, ns_id=%#jx, ns_type=0, user_ns_id=0}"
	       ", NULL, 0, 0) = %s" INJ_STR, req->size,
	       (uintmax_t) req->ns_id, errstr);

	req->ns_type = TIME_NS;
	k_listns(req, 0, 0, 0);
	printf("listns({size=%u, ns_id=%#jx, ns_type=TIME_NS, user_ns_id=0}"
	       ", NULL, 0, 0) = %s" INJ_STR, req->size,
	       (uintmax_t) req->ns_id, errstr);

	req->ns_type = valid_ns_types.val;
	k_listns(req, 0, 0, 0);
	printf("listns({size=%u, ns_id=%#jx, ns_type=%s, user_ns_id=0}"
	       ", NULL, 0, 0) = %s" INJ_STR,
	       req->size, (uintmax_t) req->ns_id, valid_ns_types.str, errstr);

	req->ns_type = ~valid_ns_types.val;
	req->user_ns_id = LISTNS_CURRENT_USER;
	k_listns(req, 0, 0, 0);
	printf("listns({size=%u, ns_id=%#jx, ns_type=%#x /* ???_NS */"
	       ", user_ns_id=LISTNS_CURRENT_USER}, NULL, 0, 0) = %s" INJ_STR,
	       req->size, (uintmax_t) req->ns_id, req->ns_type, errstr);

	req->ns_type = MNT_NS | NET_NS;
	req->user_ns_id = 0x123456789abcdef0ULL;
	k_listns(req, 0, 0, 0);
	printf("listns({size=%u, ns_id=%#jx, ns_type=MNT_NS|NET_NS"
	       ", user_ns_id=%#jx}, NULL, 0, 0) = %s" INJ_STR,
	       req->size, (uintmax_t) req->ns_id,
	       (uintmax_t) req->user_ns_id, errstr);

	++req->size;
	k_listns(req, 0, 0, 0);
	printf("listns({size=%u, ns_id=%#jx, ns_type=MNT_NS|NET_NS"
	       ", user_ns_id=%#jx, ???}, NULL, 0, 0) = %s" INJ_STR,
	       req->size, (uintmax_t) req->ns_id,
	       (uintmax_t) req->user_ns_id, errstr);

	req->size = sizeof(*req) + 8;
	char *p = (char *) req - 8;
	memmove(p, req, sizeof(*req));
	fill_memory(p + sizeof(*req), 8);
	k_listns(p, 0, 0, 0);
	memmove(req, p, sizeof(*req));
	printf("listns({size=%u, ns_id=%#jx, ns_type=MNT_NS|NET_NS"
	       ", user_ns_id=%#jx, /* bytes %zu..%zu */ \"%s\"}, NULL, 0, 0)"
	       " = %s" INJ_STR,
	       req->size, (uintmax_t) req->ns_id,
	       (uintmax_t) req->user_ns_id, sizeof(*req), sizeof(*req) + 7,
	       "\\x80\\x81\\x82\\x83\\x84\\x85\\x86\\x87", errstr);

	k_listns(0, ns_ids, 0, 0);
	printf("listns(NULL, [], 0, 0) = %s" INJ_STR, errstr);

	if (k_listns(0, ns_ids, 1, 0) < 0)
		printf("listns(NULL, %p, 1, 0) = %s" INJ_STR,
		       ns_ids, errstr);
	else
		printf("listns(NULL, [%#jx], 1, 0) = %s" INJ_STR,
		       (uintmax_t) ns_ids[0], errstr);

	if (k_listns(0, ns_ids, 2, 0) < 0)
		printf("listns(NULL, %p, 2, 0) = %s" INJ_STR,
		       ns_ids, errstr);
	else
		printf("listns(NULL, [%#jx, %#jx], 2, 0) = %s" INJ_STR,
		       (uintmax_t) ns_ids[0], (uintmax_t) ns_ids[1], errstr);

	if (k_listns(0, ns_ids, 3, 0) < 0)
		printf("listns(NULL, %p, 3, 0) = %s" INJ_STR,
		       ns_ids, errstr);
	else
		printf("listns(NULL, [%#jx, %#jx, ... /* %p */], 3, 0) = %s"
		       INJ_STR, (uintmax_t) ns_ids[0], (uintmax_t) ns_ids[1],
		       bad_ns_ids, errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
