/*
 * Check decoding of lsm_set_self_attr syscall.
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
#include <linux/lsm.h>

static const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
static const char *errstr;

static long
k_lsm_set_self_attr(const unsigned int attr, const void *p_ctx,
		    const uint32_t size, const uint32_t flags)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t arg1 = fill | attr;
	const kernel_ulong_t arg2 = (uintptr_t) p_ctx;
	const kernel_ulong_t arg3 = fill | size;
	const kernel_ulong_t arg4 = fill | flags;
	const long rc = syscall(__NR_lsm_set_self_attr,
				arg1, arg2, arg3, arg4, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	k_lsm_set_self_attr(0, 0, 0, 0);
	printf("lsm_set_self_attr(LSM_ATTR_UNDEF, NULL, 0, 0) = %s\n", errstr);

	struct lsm_ctx *const ctx =
		midtail_alloc(sizeof(*ctx), 2);
	fill_memory(ctx, sizeof(*ctx));
	const void *const bad_ctx = (void *) ctx + 1;

	const unsigned int bad_attr = 0xfacefed1;
	const uint32_t bad_flags = 0xfacefed2;
	uint32_t size = 0xdeadbeef;

	k_lsm_set_self_attr(bad_attr, bad_ctx, size, bad_flags);
	printf("lsm_set_self_attr(%#x /* LSM_ATTR_??? */, %p, %u, %#x) = %s\n",
	       bad_attr, bad_ctx, size, bad_flags, errstr);

	ctx->ctx_len = 0;
	k_lsm_set_self_attr(LSM_ATTR_CURRENT, ctx, size, 1);
	printf("lsm_set_self_attr(%s, {id=%#jx, flags=%#jx, len=%ju"
	       ", ctx_len=%ju}, %u, 0x1) = %s\n",
	       "LSM_ATTR_CURRENT", (uintmax_t) ctx->id, (uintmax_t) ctx->flags,
	       (uintmax_t) ctx->len, (uintmax_t) ctx->ctx_len,
	       size, errstr);

	ctx->ctx_len = 1;
	k_lsm_set_self_attr(LSM_ATTR_CURRENT, ctx, size, 2);
	printf("lsm_set_self_attr(%s, {id=%#jx, flags=%#jx, len=%ju"
	       ", ctx_len=%ju, ctx=%p}, %u, 0x2) = %s\n",
	       "LSM_ATTR_CURRENT", (uintmax_t) ctx->id, (uintmax_t) ctx->flags,
	       (uintmax_t) ctx->len, (uintmax_t) ctx->ctx_len,
	       ctx->ctx, size, errstr);

	ctx->ctx_len = 2;
	void *ctx2 = (void *) ctx - ctx->ctx_len;
	memmove(ctx2, ctx, sizeof(*ctx));
	fill_memory((void *) (ctx + 1) - 2, 2);
	k_lsm_set_self_attr(LSM_ATTR_CURRENT, ctx2, size, 3);
	memmove(ctx, ctx2, sizeof(*ctx));
	printf("lsm_set_self_attr(%s, {id=%#jx, flags=%#jx, len=%ju"
	       ", ctx_len=%ju, ctx=\"\\x80\\x81\"}, %u, 0x3) = %s\n",
	       "LSM_ATTR_CURRENT", (uintmax_t) ctx->id, (uintmax_t) ctx->flags,
	       (uintmax_t) ctx->len, (uintmax_t) ctx->ctx_len,
	       size, errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
