/*
 * Copyright (c) 2024 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/lsm.h>
#include "xlat/lsm_attrs.h"
#include "xlat/lsm_flags.h"
#include "xlat/lsm_ids.h"

static int
print_args_size_flags(struct tcb *const tcp, const kernel_ulong_t p_size,
		      const uint32_t flags, const bool raw_flags)
{
	tprint_arg_next();
	printnum_int(tcp, p_size, "%u");

	tprint_arg_next();
	if (raw_flags)
		PRINT_VAL_X(flags);
	else
		printflags(lsm_flags, flags, "LSM_FLAG_???");

	return RVAL_DECODED;
}

static bool
fetch_lsm_ctx_header(struct tcb *const tcp, struct lsm_ctx *const ctx,
		     const kernel_ulong_t addr, const uint32_t len,
		     const bool is_sequence)
{
	if (len >= sizeof(*ctx) && tfetch_obj(tcp, addr, ctx))
		return true;

	if (is_sequence) {
		tprint_more_data_follows();
		printaddr_comment(addr);
	} else {
		printaddr(addr);
	}

	return false;
}

static void
decode_lsm_ctx(struct tcb *const tcp, const struct lsm_ctx *const ctx,
	       const kernel_ulong_t addr, const uint32_t len)
{
	tprint_struct_begin();
	PRINT_FIELD_XVAL(*ctx, id, lsm_ids, NULL);

	tprint_struct_next();
	PRINT_FIELD_X(*ctx, flags);

	tprint_struct_next();
	PRINT_FIELD_U(*ctx, len);

	tprint_struct_next();
	PRINT_FIELD_U(*ctx, ctx_len);

	const unsigned int msg_len = MIN(ctx->len, len);
	const unsigned int ctx_offset = offsetof(struct lsm_ctx, ctx);
	const unsigned int msg_ctx_len =
		msg_len > ctx_offset ? msg_len - ctx_offset : 0;
	unsigned int ctx_len = MIN(ctx->ctx_len, msg_ctx_len);

	if (ctx_len > 0) {
		const kernel_ulong_t ctx_addr =
			addr + offsetof(struct lsm_ctx, ctx);
		tprint_struct_next();
		tprints_field_name("ctx");
		printstr_ex(tcp, ctx_addr, ctx_len, QUOTE_FORCE_HEX);
	}

	tprint_struct_end();
}

static void
decode_lsm_ctx_sequence(struct tcb *const tcp, kernel_ulong_t addr, uint32_t len)
{
	struct lsm_ctx ctx;
	bool is_sequence = false;

	for (unsigned int elt = 0;
	     fetch_lsm_ctx_header(tcp, &ctx, addr, len, is_sequence);
	     ++elt) {
		/* elt starts with 0, hence elt + 1 */
		if (sequence_truncation_needed(tcp, elt + 1)) {
			tprint_more_data_follows();
			break;
		}

		kernel_ulong_t next_addr = 0;
		uint32_t next_len = 0;

		if (ctx.len >= sizeof(ctx)) {
			if (len >= ctx.len)
				next_len = len - ctx.len;

			if (next_len > 0 && addr + ctx.len > addr)
				next_addr = addr + ctx.len;
		}

		if (!is_sequence) {
			tprint_array_begin();
			is_sequence = true;
		}

		decode_lsm_ctx(tcp, &ctx, addr, len);

		if (!next_addr)
			break;

		tprint_array_next();
		addr = next_addr;
		len = next_len;
	}

	if (is_sequence)
		tprint_array_end();
}

SYS_FUNC(lsm_get_self_attr)
{
	const unsigned int attr = tcp->u_arg[0];
	const kernel_ulong_t p_ctx = tcp->u_arg[1];
	const kernel_ulong_t p_size = tcp->u_arg[2];
	const uint32_t flags = tcp->u_arg[3];

	const bool single = flags & LSM_FLAG_SINGLE;
	uint32_t size;
	struct lsm_ctx ctx;

	if (entering(tcp)) {
		printxval(lsm_attrs, attr, "LSM_ATTR_???");
		tprint_arg_next();

		if (!tfetch_obj(tcp, p_size, &size) ||
		    (single && !tfetch_obj(tcp, p_ctx, &ctx))) {
			printaddr(p_ctx);
			return print_args_size_flags(tcp, p_size, flags, false);
		}
		set_tcb_priv_ulong(tcp, size);

		if (single) {
			tprint_struct_begin();
			PRINT_FIELD_XVAL(ctx, id, lsm_ids, NULL);
			tprint_struct_end();
		}

		return 0;
	}

	if (!tfetch_obj_ignore_syserror(tcp, p_size, &size)) {
		if (!single)
			printaddr(p_ctx);

		return print_args_size_flags(tcp, p_size, flags, false);
	}

	if (single)
		tprint_value_changed();
	decode_lsm_ctx_sequence(tcp, p_ctx, size);
	tprint_arg_next();

	const uint32_t saved_size = get_tcb_priv_ulong(tcp);
	tprint_indirect_begin();
	PRINT_VAL_U(saved_size);
	if (saved_size != size) {
		tprint_value_changed();
		PRINT_VAL_U(size);
	}
	tprint_indirect_end();

	tprint_arg_next();
	printflags(lsm_flags, flags, "LSM_FLAG_???");

	return 0;
}

SYS_FUNC(lsm_set_self_attr)
{
	const unsigned int attr = tcp->u_arg[0];
	const kernel_ulong_t p_ctx = tcp->u_arg[1];
	const uint32_t size = tcp->u_arg[2];
	const uint32_t flags = tcp->u_arg[3];

	struct lsm_ctx ctx;

	printxval(lsm_attrs, attr, "LSM_ATTR_???");
	tprint_arg_next();

	if (fetch_lsm_ctx_header(tcp, &ctx, p_ctx, size, false))
		decode_lsm_ctx(tcp, &ctx, p_ctx, size);

	tprint_arg_next();
	PRINT_VAL_U(size);

	tprint_arg_next();
	PRINT_VAL_X(flags);

	return RVAL_DECODED;
}

static bool
print_lsm_id_array_member(struct tcb *tcp, void *elem_buf, size_t elem_size,
			  void *data)
{
	const uint64_t *p_id = elem_buf;
	printxval64(lsm_ids, *p_id, "LSM_ID_???");

	return true;
}

SYS_FUNC(lsm_list_modules)
{
	const kernel_ulong_t p_ids = tcp->u_arg[0];
	const kernel_ulong_t p_size = tcp->u_arg[1];
	const uint32_t flags = tcp->u_arg[2];
	uint32_t size;

	if (entering(tcp)) {
		if (!tfetch_obj(tcp, p_size, &size)) {
			printaddr(p_ids);
			return print_args_size_flags(tcp, p_size, flags, true);
		}

		set_tcb_priv_ulong(tcp, size);

		return 0;
	}

	if (!tfetch_obj_ignore_syserror(tcp, p_size, &size)) {
		printaddr(p_ids);
		return print_args_size_flags(tcp, p_size, flags, true);
	}

	const uint32_t saved_size = get_tcb_priv_ulong(tcp);
	uint64_t elem;

	print_array(tcp, p_ids, (kernel_ulong_t) tcp->u_rval, &elem,
		    sizeof(elem), tfetch_mem, print_lsm_id_array_member, 0);

	tprint_arg_next();
	tprint_indirect_begin();
	PRINT_VAL_U(saved_size);
	if (saved_size != size) {
		tprint_value_changed();
		PRINT_VAL_U(size);
	}
	tprint_indirect_end();

	tprint_arg_next();
	PRINT_VAL_X(flags);

	return 0;
}
