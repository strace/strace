/*
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "print_fields.h"
#include <linux/rseq.h>
#include "xlat/rseq_cs_flags.h"
#include "xlat/rseq_flags.h"

#define CASE_PRINT_XLAT_D(sym_) \
	case (sym_): print_xlat_ex((sym_), #sym_, XLAT_STYLE_FMT_D); break

static void
print_rseq_cpu_id_u32(const uint32_t cpu_id)
{
	switch (cpu_id) {
		CASE_PRINT_XLAT_D(RSEQ_CPU_ID_UNINITIALIZED);
		CASE_PRINT_XLAT_D(RSEQ_CPU_ID_REGISTRATION_FAILED);
		default:
			PRINT_VAL_U(cpu_id);
			break;
	}
}

/*
 * @param raw: struct rseq::rseq_cs field from user memory (__u64).
 * On 32-bit ISAs only the low-order bits are the user pointer (kernel documents
 * the rest as zero); normalize with truncate_kulong_to_current_wordsize before
 * fetch.  When addr is zero or tfetch_obj fails, print the full 64-bit value with
 * printaddr64(raw).  After a successful decode, annotate with printaddr_comment(addr)
 * (the address that was read).
 */
static void
decode_rseq_cs(struct tcb *tcp, const uint64_t raw)
{
	const kernel_ulong_t addr =
		truncate_kulong_to_current_wordsize(raw);
	struct rseq_cs cs;

	if (!addr) {
		printaddr64(raw);
		return;
	}

	if (!tfetch_obj(tcp, addr, &cs)) {
		printaddr64(raw);
		return;
	}

	tprint_struct_begin();
	PRINT_FIELD_U(cs, version);
	tprint_struct_next();
	PRINT_FIELD_FLAGS(cs, flags, rseq_cs_flags, "RSEQ_CS_FLAG_???");
	tprint_struct_next();
	PRINT_FIELD_X(cs, start_ip);
	tprint_struct_next();
	PRINT_FIELD_X(cs, post_commit_offset);
	tprint_struct_next();
	PRINT_FIELD_X(cs, abort_ip);
	tprint_struct_end();
	printaddr_comment(addr);
}

static void
print_rseq_slice_ctrl(const struct rseq_slice_ctrl *const slice_ctrl)
{
	tprint_struct_begin();
	PRINT_FIELD_U(*slice_ctrl, request);
	tprint_struct_next();
	PRINT_FIELD_U(*slice_ctrl, granted);
	tprint_struct_next();
	PRINT_FIELD_X(*slice_ctrl, __reserved);
	tprint_struct_end();
}

/*
 * The original rseq structure size (including padding) is 32 bytes.
 * Shorter lengths are rejected by the kernel with -EINVAL.
 */
enum { ORIG_RSEQ_SIZE = 32 };

static_assert(offsetofend(struct rseq, slice_ctrl) == ORIG_RSEQ_SIZE,
	      "original rseq allocation covers fields through slice_ctrl");

static void
decode_struct_rseq(struct tcb *const tcp, const kernel_ulong_t addr,
		   const uint32_t ulen)
{
	/*
	 * As the kernel rejects small ulen and unaligned addr,
	 * do not dump user memory.
	 */
	if (ulen < ORIG_RSEQ_SIZE || addr % ORIG_RSEQ_SIZE) {
		printaddr(addr);
		return;
	}

	size_t fetch_len = ulen;
	struct rseq rseq = { 0 };

	if (fetch_len > sizeof(rseq))
		fetch_len = sizeof(rseq);

	if (umoven_or_printaddr(tcp, addr, fetch_len, &rseq))
		return;

	/* Fields through slice_ctrl sit in the guaranteed 32-byte prefix. */
	tprint_struct_begin();
	PRINT_FIELD_OBJ_VAL(rseq, cpu_id_start, print_rseq_cpu_id_u32);
	tprint_struct_next();

	PRINT_FIELD_OBJ_VAL(rseq, cpu_id, print_rseq_cpu_id_u32);
	tprint_struct_next();

	PRINT_FIELD_OBJ_TCB_VAL(rseq, rseq_cs, tcp, decode_rseq_cs);
	tprint_struct_next();

	PRINT_FIELD_FLAGS(rseq, flags, rseq_cs_flags, "RSEQ_CS_FLAG_???");
	tprint_struct_next();

	PRINT_FIELD_U(rseq, node_id);
	tprint_struct_next();

	PRINT_FIELD_U(rseq, mm_cid);
	tprint_struct_next();

	PRINT_FIELD_OBJ_PTR(rseq, slice_ctrl, print_rseq_slice_ctrl);

	if (fetch_len >= offsetof(struct rseq, __reserved)) {
		tprint_struct_next();
		PRINT_FIELD_X(rseq, __reserved);
	}
	tprint_struct_end();
}

SYS_FUNC(rseq)
{
	const kernel_ulong_t rseq = tcp->u_arg[0];
	const uint32_t rseq_len = (uint32_t) tcp->u_arg[1];
	const unsigned int flags = (unsigned int) tcp->u_arg[2];
	const uint32_t sig = (uint32_t) tcp->u_arg[3];

	tprints_arg_name("rseq");
	decode_struct_rseq(tcp, rseq, rseq_len);

	tprints_arg_next_name("rseq_len");
	PRINT_VAL_U(rseq_len);

	tprints_arg_next_name("flags");
	printflags(rseq_flags, flags, "RSEQ_FLAG_???");

	tprints_arg_next_name("sig");
	PRINT_VAL_X(sig);
	return RVAL_DECODED;
}
