/*
 * Check decoding of lsm_get_self_attr syscall.
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

#ifdef INJECT_RETVAL
# define INJ_STR " (INJECTED)\n"
#else
# define INJ_STR "\n"
#endif

static const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
static const char *errstr;

static long
k_lsm_get_self_attr(const unsigned int attr, const void *p_ctx,
		    const void *p_size, const uint32_t flags)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t arg1 = fill | attr;
	const kernel_ulong_t arg2 = (uintptr_t) p_ctx;
	const kernel_ulong_t arg3 = (uintptr_t) p_size;
	const kernel_ulong_t arg4 = fill | flags;
	const long rc = syscall(__NR_lsm_get_self_attr,
				arg1, arg2, arg3, arg4, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	k_lsm_get_self_attr(0, 0, 0, 0);
	printf("lsm_get_self_attr(LSM_ATTR_UNDEF, NULL, NULL, 0) = %s" INJ_STR,
	       errstr);

	struct lsm_ctx *const ctx =
		midtail_alloc(sizeof(*ctx), 2 * sizeof(*ctx));
	fill_memory(ctx, sizeof(*ctx));
	const void *const bad_ctx = (void *) ctx + 1;

	TAIL_ALLOC_OBJECT_CONST_PTR(uint32_t, size);
	const void *const bad_size = (void *) size + 1;

	const unsigned int bad_attr = 0xfacefed1;
	const uint32_t bad_flags = 0xfacefed2;

	k_lsm_get_self_attr(bad_attr, bad_ctx, bad_size, bad_flags);
	printf("lsm_get_self_attr(%#x /* LSM_ATTR_??? */, %p, %p"
	       ", %#x /* LSM_FLAG_??? */) = %s" INJ_STR,
	       bad_attr, bad_ctx, bad_size, bad_flags, errstr);

	k_lsm_get_self_attr(LSM_ATTR_CURRENT, ctx, bad_size, 0);
	printf("lsm_get_self_attr(%s, %p, %p, 0) = %s" INJ_STR,
	       "LSM_ATTR_CURRENT", ctx, bad_size, errstr);

	*size = 0xdeadbeef;
	k_lsm_get_self_attr(LSM_ATTR_EXEC, bad_ctx, size, 1);
	printf("lsm_get_self_attr(%s, %p, [%u], %s) = %s" INJ_STR,
	       "LSM_ATTR_EXEC", bad_ctx, *size, "LSM_FLAG_SINGLE", errstr);

	k_lsm_get_self_attr(LSM_ATTR_FSCREATE, bad_ctx, size, 2);
	printf("lsm_get_self_attr(%s, %p, [%u], %s) = %s" INJ_STR,
	       "LSM_ATTR_FSCREATE", bad_ctx, *size, "0x2 /* LSM_FLAG_??? */",
	       errstr);

	*size = sizeof(*ctx) - 1;
	k_lsm_get_self_attr(LSM_ATTR_KEYCREATE, ctx, size, 3);
	printf("lsm_get_self_attr(%s, {id=%#jx} => %p, [%u], %s) = %s" INJ_STR,
	       "LSM_ATTR_KEYCREATE", (uintmax_t) ctx->id, ctx, *size,
	       "LSM_FLAG_SINGLE|0x2", errstr);

	*size = sizeof(*ctx) + 1;
	ctx->id = 0;
	if (k_lsm_get_self_attr(LSM_ATTR_PREV, ctx, size, 4) < 0)
		printf("lsm_get_self_attr(%s, %p, [%u], %s) = %s" INJ_STR,
		       "LSM_ATTR_PREV", ctx, *size, "0x4 /* LSM_FLAG_??? */",
		       errstr);
	else
		printf("lsm_get_self_attr(%s, [{id=%s, flags=%#jx, len=%ju"
		       ", ctx_len=%ju, ctx=%p}], [%u], %s) = %s" INJ_STR,
		       "LSM_ATTR_PREV", "LSM_ID_UNDEF", (uintmax_t) ctx->flags,
		       (uintmax_t) ctx->len, (uintmax_t) ctx->ctx_len, ctx->ctx,
		       *size, "0x4 /* LSM_FLAG_??? */", errstr);

	*size = sizeof(*ctx);
	ctx->id = LSM_ID_CAPABILITY;
	if (k_lsm_get_self_attr(LSM_ATTR_SOCKCREATE, ctx, size, 5) < 0)
		printf("lsm_get_self_attr(%s, {id=%s} => %p, [%u], %s) = %s"
		       INJ_STR, "LSM_ATTR_SOCKCREATE", "LSM_ID_CAPABILITY",
		       ctx, *size, "LSM_FLAG_SINGLE|0x4", errstr);
	else
		printf("lsm_get_self_attr(%s, {id=%s} => [{id=%s, flags=%#jx"
		       ", len=%ju, ctx_len=%ju}], [%u], %s) = %s" INJ_STR,
		       "LSM_ATTR_SOCKCREATE", "LSM_ID_CAPABILITY",
		       "LSM_ID_CAPABILITY", (uintmax_t) ctx->flags,
		       (uintmax_t) ctx->len, (uintmax_t) ctx->ctx_len,
		       *size, "LSM_FLAG_SINGLE|0x4", errstr);

	*size = 3 * sizeof(*ctx) + 1;
	ctx->id = LSM_ID_SELINUX;
	ctx->len = sizeof(*ctx) + 1;
	if (k_lsm_get_self_attr(LSM_ATTR_CURRENT, ctx, size, 6) < 0)
		printf("lsm_get_self_attr(%s, %p, [%u], %s) = %s"
		       INJ_STR, "LSM_ATTR_CURRENT", ctx, *size,
		       "0x6 /* LSM_FLAG_??? */", errstr);
	else
		printf("lsm_get_self_attr(%s, [{id=%s, flags=%#jx"
		       ", len=%ju, ctx_len=%ju, ctx=%p}, ... /* %p */], [%u], %s) = %s"
		       INJ_STR, "LSM_ATTR_CURRENT", "LSM_ID_SELINUX",
		       (uintmax_t) ctx->flags, (uintmax_t) ctx->len,
		       (uintmax_t) ctx->ctx_len, ctx->ctx,
		       (void *) (ctx + 1) + 1, *size,
		       "0x6 /* LSM_FLAG_??? */", errstr);

	ctx->len = *size + 1;
	if (k_lsm_get_self_attr(LSM_ATTR_CURRENT, ctx, size, 6) < 0)
		printf("lsm_get_self_attr(%s, %p, [%u], %s) = %s"
		       INJ_STR, "LSM_ATTR_CURRENT", ctx, *size,
		       "0x6 /* LSM_FLAG_??? */", errstr);
	else
		printf("lsm_get_self_attr(%s, [{id=%s, flags=%#jx"
		       ", len=%ju, ctx_len=%ju, ctx=%p}], [%u], %s) = %s"
		       INJ_STR, "LSM_ATTR_CURRENT", "LSM_ID_SELINUX",
		       (uintmax_t) ctx->flags, (uintmax_t) ctx->len,
		       (uintmax_t) ctx->ctx_len, ctx->ctx,
		       *size, "0x6 /* LSM_FLAG_??? */", errstr);

	ctx->len = -1;
	memcpy(ctx - 1, ctx, sizeof(*ctx));
	fill_memory(ctx, sizeof(*ctx));
	if (k_lsm_get_self_attr(LSM_ATTR_CURRENT, ctx - 1, size, 8) < 0)
		printf("lsm_get_self_attr(%s, %p, [%u], %s) = %s"
		       INJ_STR, "LSM_ATTR_CURRENT", ctx - 1, *size,
		       "0x8 /* LSM_FLAG_??? */", errstr);
	else
		printf("lsm_get_self_attr(%s, [{id=%s, flags=%#jx"
		       ", len=%ju, ctx_len=%ju, ctx=\"\\x%x\\x%x\"...}]"
		       ", [%u], %s) = %s" INJ_STR,
		       "LSM_ATTR_CURRENT", "LSM_ID_SELINUX",
		       (uintmax_t) (ctx - 1)->flags, (uintmax_t) (ctx - 1)->len,
		       (uintmax_t) (ctx - 1)->ctx_len, * (unsigned char *) ctx,
		       * ((unsigned char *) ctx + 1), *size,
		       "0x8 /* LSM_FLAG_??? */", errstr);

	(ctx - 1)->len = sizeof(*ctx) - 1;
	if (k_lsm_get_self_attr(LSM_ATTR_CURRENT, ctx - 1, size, 16) < 0)
		printf("lsm_get_self_attr(%s, %p, [%u], %s) = %s"
		       INJ_STR, "LSM_ATTR_CURRENT", ctx - 1, *size,
		       "0x10 /* LSM_FLAG_??? */", errstr);
	else
		printf("lsm_get_self_attr(%s, [{id=%s, flags=%#jx"
		       ", len=%ju, ctx_len=%ju}], [%u], %s) = %s" INJ_STR,
		       "LSM_ATTR_CURRENT", "LSM_ID_SELINUX",
		       (uintmax_t) (ctx - 1)->flags, (uintmax_t) (ctx - 1)->len,
		       (uintmax_t) (ctx - 1)->ctx_len, *size,
		       "0x10 /* LSM_FLAG_??? */", errstr);

	fill_memory(ctx - 2, 3 * sizeof(*ctx));
	(ctx - 2)->id = LSM_ID_CAPABILITY;
	(ctx - 2)->len = sizeof(*ctx);
	ctx->id = LSM_ID_SELINUX;
	(ctx - 1)->id = LSM_ID_SELINUX;
	(ctx - 1)->len = sizeof(*ctx);
	if (k_lsm_get_self_attr(LSM_ATTR_CURRENT, ctx - 2, size, 32) < 0)
		printf("lsm_get_self_attr(%s, %p, [%u], %s) = %s"
		       INJ_STR, "LSM_ATTR_CURRENT", ctx - 2, *size,
		       "0x20 /* LSM_FLAG_??? */", errstr);
	else
		printf("lsm_get_self_attr(%s"
		       ", [{id=%s, flags=%#jx, len=%ju, ctx_len=%ju}"
		       ", {id=%s, flags=%#jx, len=%ju, ctx_len=%ju}"
		       ", ...], [%u], %s) = %s" INJ_STR,
		       "LSM_ATTR_CURRENT",
		       "LSM_ID_CAPABILITY", (uintmax_t) (ctx - 2)->flags,
		       (uintmax_t) (ctx - 2)->len,
		       (uintmax_t) (ctx - 2)->ctx_len,
		       "LSM_ID_SELINUX", (uintmax_t) (ctx - 1)->flags,
		       (uintmax_t) (ctx - 1)->len,
		       (uintmax_t) (ctx - 1)->ctx_len,
		       *size, "0x20 /* LSM_FLAG_??? */", errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
