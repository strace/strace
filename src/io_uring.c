/*
 * Copyright (c) 2019 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2019-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include "kernel_time_types.h"
#define UAPI_LINUX_IO_URING_H_SKIP_LINUX_TIME_TYPES_H
#include <linux/io_uring.h>
#include <linux/io_uring/query.h>

#include "xlat/uring_async_cancel_flags.h"
#include "xlat/uring_clone_buffers_flags.h"
#include "xlat/uring_enter_flags.h"
#include "xlat/uring_files_update_fds.h"
#include "xlat/uring_iowq_acct.h"
#include "xlat/uring_op_flags.h"
#include "xlat/uring_ops.h"
#include "xlat/uring_setup_features.h"
#include "xlat/uring_setup_flags.h"
#include "xlat/uring_sqe_flags.h"
#include "xlat/uring_register_opcodes.h"
#include "xlat/uring_register_opcode_flags.h"
#include "xlat/uring_register_rsrc_flags.h"
#include "xlat/uring_restriction_opcodes.h"
#include "xlat/uring_napi_ops.h"
#include "xlat/uring_napi_tracking_strategies.h"
#include "xlat/uring_msg_ring_flags.h"
#include "xlat/uring_zcrx_reg_flags.h"
#include "xlat/uring_zcrx_area_flags.h"
#include "xlat/uring_mem_region_reg_flags.h"
#include "xlat/uring_query_ops.h"
#include "xlat/uring_zcrx_ctrl_ops.h"
#include "xlat/uring_rw_attr_flags.h"
#include "xlat/uring_nop_flags.h"
#include "xlat/uring_fixed_fd_flags.h"
#include "xlat/uring_accept_flags.h"
#include "xlat/uring_recvsend_flags.h"
#include "xlat/uring_socket_ops.h"

static void
print_io_sqring_offsets(const struct io_sqring_offsets *const p)
{
	tprint_struct_begin();
	PRINT_FIELD_U(*p, head);
	tprint_struct_next();
	PRINT_FIELD_U(*p, tail);
	tprint_struct_next();
	PRINT_FIELD_U(*p, ring_mask);
	tprint_struct_next();
	PRINT_FIELD_U(*p, ring_entries);
	tprint_struct_next();
	PRINT_FIELD_U(*p, flags);
	tprint_struct_next();
	PRINT_FIELD_U(*p, dropped);
	tprint_struct_next();
	PRINT_FIELD_U(*p, array);
	if (p->resv1) {
		tprint_struct_next();
		PRINT_FIELD_X(*p, resv1);
	}
	tprint_struct_next();
	PRINT_FIELD_X(*p, user_addr);
	tprint_struct_end();
}

static void
print_io_cqring_offsets(const struct io_cqring_offsets *const p)
{
	tprint_struct_begin();
	PRINT_FIELD_U(*p, head);
	tprint_struct_next();
	PRINT_FIELD_U(*p, tail);
	tprint_struct_next();
	PRINT_FIELD_U(*p, ring_mask);
	tprint_struct_next();
	PRINT_FIELD_U(*p, ring_entries);
	tprint_struct_next();
	PRINT_FIELD_U(*p, overflow);
	tprint_struct_next();
	PRINT_FIELD_U(*p, cqes);
	tprint_struct_next();
	PRINT_FIELD_U(*p, flags);
	if (p->resv1) {
		tprint_struct_next();
		PRINT_FIELD_X(*p, resv1);
	}
	tprint_struct_next();
	PRINT_FIELD_X(*p, user_addr);
	tprint_struct_end();
}

SYS_FUNC(io_uring_setup)
{
	const uint32_t entries = tcp->u_arg[0];
	const kernel_ulong_t params_addr = tcp->u_arg[1];
	struct io_uring_params params;

	if (entering(tcp)) {
		/* entries */
		tprints_arg_name("entries");
		PRINT_VAL_U(entries);

		/* params */
		tprints_arg_next_name("params");
		if (umove_or_printaddr(tcp, params_addr, &params))
			return RVAL_DECODED | RVAL_FD;

		tprint_struct_begin();
		PRINT_FIELD_FLAGS(params, flags, uring_setup_flags,
				  "IORING_SETUP_???");
		tprint_struct_next();
		PRINT_FIELD_X(params, sq_thread_cpu);
		tprint_struct_next();
		PRINT_FIELD_U(params, sq_thread_idle);
		if (params.flags & IORING_SETUP_ATTACH_WQ) {
			tprint_struct_next();
			PRINT_FIELD_FD(params, wq_fd, tcp);
		}
		if (!IS_ARRAY_ZERO(params.resv)) {
			tprint_struct_next();
			PRINT_FIELD_ARRAY(params, resv, tcp,
					  print_xint_array_member);
		}
		return 0;
	}

	/* exiting */
	if (tfetch_mem(tcp, params_addr, sizeof(params), &params)) {
		tprint_struct_next();
		PRINT_FIELD_U(params, sq_entries);
		tprint_struct_next();
		PRINT_FIELD_U(params, cq_entries);
		tprint_struct_next();
		PRINT_FIELD_FLAGS(params, features,
				  uring_setup_features,
				  "IORING_FEAT_???");
		tprint_struct_next();
		PRINT_FIELD_OBJ_PTR(params, sq_off,
				    print_io_sqring_offsets);
		tprint_struct_next();
		PRINT_FIELD_OBJ_PTR(params, cq_off,
				    print_io_cqring_offsets);
	}
	tprint_struct_end();

	return RVAL_DECODED | RVAL_FD;
}

static void
print_io_uring_getevents_arg(struct tcb *tcp, const kernel_ulong_t addr)
{
	struct io_uring_getevents_arg arg;

	CHECK_TYPE_SIZE(struct io_uring_getevents_arg, 24);

	if (umove_or_printaddr(tcp, addr, &arg))
		return;

	tprint_struct_begin();

	tprints_field_name("sigmask");
	print_sigset_addr_len(tcp, arg.sigmask, arg.sigmask_sz);

	tprint_struct_next();
	PRINT_FIELD_U(arg, sigmask_sz);

	tprint_struct_next();
	PRINT_FIELD_U(arg, min_wait_usec);

	tprint_struct_next();
	tprints_field_name("ts");
	print_timespec64(tcp, arg.ts);

	tprint_struct_end();
}

SYS_FUNC(io_uring_enter)
{
	const int fd = tcp->u_arg[0];
	const uint32_t to_submit = tcp->u_arg[1];
	const uint32_t min_complete = tcp->u_arg[2];
	const uint32_t flags = tcp->u_arg[3];
	const kernel_ulong_t sigset_addr = tcp->u_arg[4];
	const kernel_ulong_t sigset_size = tcp->u_arg[5];

	/* fd */
	tprints_arg_name("fd");
	printfd(tcp, fd);

	/* to_submit */
	tprints_arg_next_name("to_submit");
	PRINT_VAL_U(to_submit);

	/* min_complete */
	tprints_arg_next_name("min_complete");
	PRINT_VAL_U(min_complete);

	/* flags */
	tprints_arg_next_name("flags");
	printflags(uring_enter_flags, flags, "IORING_ENTER_???");

	/*
	 * Conditional decoding of arguments 4 and 5 based on flags.
	 * Note: IORING_ENTER_EXT_ARG and IORING_ENTER_EXT_ARG_REG are mutually
	 * exclusive.  If both are set, kernel returns -EINVAL, but for backward
	 * compatibility with existing tests, decode as traditional sigset.
	 */
	if ((flags & IORING_ENTER_EXT_ARG_REG) &&
	    !(flags & IORING_ENTER_EXT_ARG)) {
		/*
		 * Registered wait region case: arg[4] is a byte offset,
		 * not a pointer.
		 */
		tprints_arg_next_name("arg");
		PRINT_VAL_U(sigset_addr);
		tprints_arg_next_name("argsz");
		PRINT_VAL_U(sigset_size);
	} else if ((flags & IORING_ENTER_EXT_ARG) &&
		   !(flags & IORING_ENTER_EXT_ARG_REG)) {
		/*
		 * Extended argument case: arg[4] is pointer to
		 * io_uring_getevents_arg.
		 */
		tprints_arg_next_name("arg");
		print_io_uring_getevents_arg(tcp, sigset_addr);
		tprints_arg_next_name("argsz");
		PRINT_VAL_U(sigset_size);
	} else {
		/* Traditional sigset case. */
		tprints_arg_next_name("sigset");
		print_sigset_addr_len(tcp, sigset_addr, sigset_size);

		tprints_arg_next_name("sigsetsize");
		PRINT_VAL_U(sigset_size);
	}

	return RVAL_DECODED;
}

static bool
print_files_update_array_member(struct tcb *tcp, void *elem_buf,
				size_t elem_size, void *data)
{
	int fd = *(int *) elem_buf;

	if (fd < -1)
		printxval_d(uring_files_update_fds, fd, NULL);
	else
		printfd(tcp, fd);

	return true;
}

static void
print_io_uring_files_update(struct tcb *tcp, const kernel_ulong_t addr,
			    const unsigned int nargs)
{
	struct io_uring_files_update arg;
	int buf;

	if (umove_or_printaddr(tcp, addr, &arg))
		return;

	tprint_struct_begin();
	PRINT_FIELD_U(arg, offset);
	if (arg.resv) {
		tprint_struct_next();
		PRINT_FIELD_X(arg, resv);
	}
	tprint_struct_next();
	tprints_field_name("fds");
	print_big_u64_addr(arg.fds);
	print_array(tcp, arg.fds, nargs, &buf, sizeof(buf),
		    tfetch_mem, print_files_update_array_member, NULL);
	tprint_struct_end();
}

static bool
print_io_uring_probe_op(struct tcb *tcp, void *elem_buf, size_t elem_size,
			void *data)
{
	struct io_uring_probe_op *op = (struct io_uring_probe_op *) elem_buf;

	tprint_struct_begin();
	PRINT_FIELD_XVAL_U(*op, op, uring_ops, "IORING_OP_???");
	if (op->resv) {
		tprint_struct_next();
		PRINT_FIELD_X(*op, resv);
	}
	tprint_struct_next();
	PRINT_FIELD_FLAGS(*op, flags, uring_op_flags, "IO_URING_OP_???");
	if (op->resv2) {
		tprint_struct_next();
		PRINT_FIELD_X(*op, resv2);
	}
	tprint_struct_end();

	return true;
}

static int
print_io_uring_probe(struct tcb *tcp, const kernel_ulong_t addr,
		     const unsigned int nargs)
{
	struct io_uring_probe *probe;
	unsigned long printed = exiting(tcp) ? get_tcb_priv_ulong(tcp) : false;

	if (exiting(tcp) && syserror(tcp)) {
		if (!printed)
			printaddr(addr);
		return RVAL_DECODED;
	}
	if (nargs > 256) {
		printaddr(addr);
		return RVAL_DECODED;
	}
	if (printed)
		tprint_value_changed();

	/* Maximum size is 8 * 256 + 16, a bit over 4k */
	size_t probe_sz = sizeof(probe->ops[0]) * nargs + sizeof(*probe);
	probe = alloca(probe_sz);

	/*
	 * So far, the operation doesn't use any data from the arg provided,
	 * but it checks that it is filled with zeroes.
	 */
	if (umoven_or_printaddr(tcp, addr, probe_sz, probe))
		return RVAL_DECODED;
	if (entering(tcp) && is_filled((const char *) probe, 0, probe_sz))
		return 0;
	set_tcb_priv_ulong(tcp, true);

	tprint_struct_begin();
	PRINT_FIELD_XVAL_U(*probe, last_op, uring_ops, "IORING_OP_???");
	tprint_struct_next();
	PRINT_FIELD_U(*probe, ops_len);
	if (probe->resv) {
		tprint_struct_next();
		PRINT_FIELD_X(*probe, resv);
	}
	if (!IS_ARRAY_ZERO(probe->resv2)) {
		tprint_struct_next();
		PRINT_FIELD_ARRAY(*probe, resv2, tcp,
				  print_xint_array_member);
	}
	tprint_struct_next();
	PRINT_FIELD_OBJ_TCB_VAL(*probe, ops, tcp, print_local_array_ex,
			entering(tcp) ? nargs : MIN(probe->ops_len, nargs),
			sizeof(probe->ops[0]), print_io_uring_probe_op, NULL,
			exiting(tcp) && (nargs < probe->ops_len)
						? PAF_ARRAY_TRUNCATED : 0,
			NULL, NULL);
	tprint_struct_end();

	return 0;
}

static bool
print_io_uring_restriction(struct tcb *tcp, void *elem_buf, size_t elem_size,
			   void *data)
{
	struct io_uring_restriction *r =
		(struct io_uring_restriction *) elem_buf;
	CHECK_TYPE_SIZE(*r, 16);
	CHECK_TYPE_SIZE(r->resv2, 12);

	tprint_struct_begin();
	PRINT_FIELD_XVAL(*r, opcode, uring_restriction_opcodes,
			 "IORING_RESTRICTION_???");
	switch (r->opcode) {
	case IORING_RESTRICTION_REGISTER_OP:
		tprint_struct_next();
		PRINT_FIELD_XVAL(*r, register_op, uring_register_opcodes,
				   "IORING_REGISTER_???");
		break;
	case IORING_RESTRICTION_SQE_OP:
		tprint_struct_next();
		PRINT_FIELD_XVAL(*r, sqe_op, uring_ops, "IORING_OP_???");
		break;
	case IORING_RESTRICTION_SQE_FLAGS_ALLOWED:
	case IORING_RESTRICTION_SQE_FLAGS_REQUIRED:
		tprint_struct_next();
		PRINT_FIELD_FLAGS(*r, sqe_flags, uring_sqe_flags, "IOSQE_???");
		break;
	default:
		tprintf_comment("op: %#x", r->register_op);
	}
	if (r->resv) {
		tprint_struct_next();
		PRINT_FIELD_X(*r, resv);
	}
	if (!IS_ARRAY_ZERO(r->resv2)) {
		tprint_struct_next();
		PRINT_FIELD_ARRAY(*r, resv2, tcp, print_xint_array_member);
	}
	tprint_struct_end();

	return true;
}

static void
print_io_uring_restrictions(struct tcb *tcp, const kernel_ulong_t addr,
			    const unsigned int nargs)
{
	struct io_uring_restriction buf;
	print_array(tcp, addr, nargs, &buf, sizeof(buf),
		    tfetch_mem, print_io_uring_restriction, NULL);
}

static void
print_io_uring_rsrc_data(struct tcb *tcp, const uint64_t data,
			 const unsigned int nr, const unsigned int opcode)
{
	int fd_buf;

	switch (opcode) {
	case IORING_REGISTER_FILES2:
	case IORING_REGISTER_BUFFERS2:
	case IORING_REGISTER_BUFFERS_UPDATE:
	case IORING_REGISTER_FILES_UPDATE2:
		tprint_struct_next();
		tprints_field_name("data");
		print_big_u64_addr(data);
		break;
	}

	switch (opcode) {
	case IORING_REGISTER_FILES2:
		print_array(tcp, data, nr, &fd_buf, sizeof(fd_buf),
			    tfetch_mem, print_fd_array_member, NULL);
		break;
	case IORING_REGISTER_FILES_UPDATE2:
		print_array(tcp, data, nr, &fd_buf, sizeof(fd_buf),
			    tfetch_mem, print_files_update_array_member, NULL);
		break;
	case IORING_REGISTER_BUFFERS2:
	case IORING_REGISTER_BUFFERS_UPDATE:
		tprint_iov(tcp, nr, data, iov_decode_addr);
		break;
	}
}

static void
print_io_uring_rsrc_tags(struct tcb *tcp, const uint64_t tags,
			 const unsigned int nr)
{
	uint64_t tag_buf;

	tprint_struct_next();
	tprints_field_name("tags");
	print_big_u64_addr(tags);
	print_array(tcp, tags, nr, &tag_buf, sizeof(tag_buf),
		    tfetch_mem, print_xint_array_member, NULL);
}

static void
print_io_uring_register_rsrc(struct tcb *tcp, const kernel_ulong_t addr,
			     const unsigned int size, const unsigned int opcode)
{
	struct io_uring_rsrc_register arg;
	CHECK_TYPE_SIZE(arg, 32);
	CHECK_TYPE_SIZE(arg.resv2, sizeof(uint64_t));

	if (size < 32) {
		printaddr(addr);
		return;
	}

	if (umove_or_printaddr(tcp, addr, &arg))
		return;

	tprint_struct_begin();
	PRINT_FIELD_U(arg, nr);

	tprint_struct_next();
	PRINT_FIELD_FLAGS(arg, flags, uring_register_rsrc_flags,
			  "IORING_RSRC_REGISTER_???");

	if (arg.resv2) {
		tprint_struct_next();
		PRINT_FIELD_X(arg, resv2);
	}

	print_io_uring_rsrc_data(tcp, arg.data, arg.nr, opcode);

	print_io_uring_rsrc_tags(tcp, arg.tags, arg.nr);

	if (size > sizeof(arg)) {
		print_nonzero_bytes(tcp, tprint_struct_next, addr, sizeof(arg),
				    MIN(size, get_pagesize()), QUOTE_FORCE_HEX);
	}

	tprint_struct_end();
}

static void
print_io_uring_update_rsrc(struct tcb *tcp, const kernel_ulong_t addr,
			   const unsigned int size, const unsigned int opcode)
{
	struct io_uring_rsrc_update2 arg;
	CHECK_TYPE_SIZE(arg, 32);
	CHECK_TYPE_SIZE(arg.resv, sizeof(uint32_t));
	CHECK_TYPE_SIZE(arg.resv2, sizeof(uint32_t));

	if (size < 32) {
		printaddr(addr);
		return;
	}

	if (umove_or_printaddr(tcp, addr, &arg))
		return;

	tprint_struct_begin();
	PRINT_FIELD_U(arg, offset);

	if (arg.resv) {
		tprint_struct_next();
		PRINT_FIELD_X(arg, resv);
	}

	print_io_uring_rsrc_data(tcp, arg.data, arg.nr, opcode);

	print_io_uring_rsrc_tags(tcp, arg.tags, arg.nr);

	tprint_struct_next();
	PRINT_FIELD_U(arg, nr);

	if (arg.resv2) {
		tprint_struct_next();
		PRINT_FIELD_X(arg, resv2);
	}

	if (size > sizeof(arg)) {
		print_nonzero_bytes(tcp, tprint_struct_next, addr, sizeof(arg),
				    MIN(size, get_pagesize()), QUOTE_FORCE_HEX);
	}

	tprint_struct_end();
}

static int
print_io_uring_iowq_acct(struct tcb *tcp, const kernel_ulong_t addr,
		     const unsigned int nargs)
{
	uint32_t val;
	bool ret = print_array_ex(tcp, addr, nargs, &val, sizeof(val),
				  tfetch_mem, print_uint_array_member, NULL,
				  PAF_PRINT_INDICES | XLAT_STYLE_FMT_U,
				  uring_iowq_acct, "IO_WQ_???");

	return ret ? 0 : RVAL_DECODED;
}

static bool
print_ringfd_register_array_member(struct tcb *tcp, void *buf,
				   size_t elem_size, void *data)
{
	/* offset - offset to insert at or -1 for the first free place */
	/* resv - reserved */
	/* data - FD to register */
	struct io_uring_rsrc_update *elem = buf;

	tprint_struct_begin();
	if (elem->offset == -1U)
		PRINT_FIELD_D(*elem, offset);
	else
		PRINT_FIELD_U(*elem, offset);

	if (elem->resv) {
		tprint_struct_next();
		PRINT_FIELD_X(*elem, resv);
	}

	tprint_struct_next();
	PRINT_FIELD_FD(*elem, data, tcp);

	tprint_struct_end();

	return true;
}
static void
print_io_uring_ringfds_register(struct tcb *tcp, const kernel_ulong_t arg,
				const unsigned int nargs)
{
	struct io_uring_rsrc_update buf;
	CHECK_TYPE_SIZE(buf, 16);
	CHECK_TYPE_SIZE(buf.resv, sizeof(uint32_t));

	print_array(tcp, arg, nargs, &buf, sizeof(buf),
		    tfetch_mem, print_ringfd_register_array_member, NULL);
}


static bool
print_ringfd_unregister_array_member(struct tcb *tcp, void *buf,
				     size_t elem_size, void *data)
{
	/* offset - offset to unregister FD */
	/* resv - reserved */
	/* data - unused */
	struct io_uring_rsrc_update *elem = buf;

	tprint_struct_begin();
	PRINT_FIELD_U(*elem, offset);

	if (elem->resv) {
		tprint_struct_next();
		PRINT_FIELD_X(*elem, resv);
	}

	if (elem->data) {
		tprint_struct_next();
		PRINT_FIELD_X(*elem, data);
	}

	tprint_struct_end();

	return true;
}
static void
print_io_uring_ringfds_unregister(struct tcb *tcp, const kernel_ulong_t arg,
				  const unsigned int nargs)
{
	struct io_uring_rsrc_update buf;

	print_array(tcp, arg, nargs, &buf, sizeof(buf),
		    tfetch_mem, print_ringfd_unregister_array_member, NULL);
}

static void
print_io_uring_buf_reg(struct tcb *tcp, const kernel_ulong_t addr,
		       const unsigned int nargs)
{
	struct io_uring_buf_reg arg;
	CHECK_TYPE_SIZE(arg, 40);
	CHECK_TYPE_SIZE(arg.flags, sizeof(uint16_t));
	CHECK_TYPE_SIZE(arg.resv, sizeof(uint64_t) * 3);

	if (nargs != 1) {
		printaddr(addr);
		return;
	}

	if (umove_or_printaddr(tcp, addr, &arg))
		return;

	tprint_struct_begin();
	PRINT_FIELD_ADDR64(arg, ring_addr);

	tprint_struct_next();
	PRINT_FIELD_U(arg, ring_entries);

	tprint_struct_next();
	PRINT_FIELD_U(arg, bgid);

	tprint_struct_next();
	PRINT_FIELD_X(arg, flags);

	if (!IS_ARRAY_ZERO(arg.resv)) {
		tprint_struct_next();
		PRINT_FIELD_ARRAY(arg, resv, tcp, print_xint_array_member);
	}

	tprint_struct_end();
}

static void
print_io_uring_sync_cancel_reg(struct tcb *tcp, const kernel_ulong_t addr,
			       const unsigned int nargs)
{
	struct io_uring_sync_cancel_reg arg;

	if (nargs != 1) {
		printaddr(addr);
		return;
	}

	if (umove_or_printaddr(tcp, addr, &arg))
		return;

	tprint_struct_begin();
	PRINT_FIELD_ADDR64(arg, addr);

	tprint_struct_next();
	PRINT_FIELD_FD(arg, fd, tcp);

	tprint_struct_next();
	PRINT_FIELD_FLAGS(arg, flags, uring_async_cancel_flags,
			  "IORING_ASYNC_CANCEL_???");

	tprint_struct_next();
	tprints_field_name("timeout");
	tprint_struct_begin();
        PRINT_FIELD_D(arg.timeout, tv_sec);
        tprint_struct_next();
        PRINT_FIELD_D(arg.timeout, tv_nsec);
        tprint_struct_end();

	tprint_struct_next();
	PRINT_FIELD_XVAL(arg, opcode, uring_ops, "IORING_OP_???");

	if (!IS_ARRAY_ZERO(arg.pad)) {
		tprint_struct_next();
		PRINT_FIELD_ARRAY(arg, pad, tcp, print_xint_array_member);
	}

	if (!IS_ARRAY_ZERO(arg.pad2)) {
		tprint_struct_next();
		PRINT_FIELD_ARRAY(arg, pad2, tcp, print_xint_array_member);
	}

	tprint_struct_end();
}

static void
print_io_uring_file_index_range(struct tcb *tcp, const kernel_ulong_t addr,
				const unsigned int nargs)
{
	struct io_uring_file_index_range arg;

	if (nargs != 1) {
		printaddr(addr);
		return;
	}

	if (umove_or_printaddr(tcp, addr, &arg))
		return;

	tprint_struct_begin();
	PRINT_FIELD_U(arg, off);

	tprint_struct_next();
	PRINT_FIELD_U(arg, len);

	if (arg.resv) {
		tprint_struct_next();
		PRINT_FIELD_X(arg, resv);
	}

	tprint_struct_end();
}

static int
print_io_uring_buf_status(struct tcb *tcp, const kernel_ulong_t addr,
			  const unsigned int nargs)
{
	struct io_uring_buf_status arg;

	if (nargs != 1) {
		printaddr(addr);
		return RVAL_DECODED;
	}

	if (entering(tcp))
		return 0;

	if (umove_or_printaddr(tcp, addr, &arg))
		return RVAL_DECODED;

	tprint_struct_begin();
	PRINT_FIELD_X(arg, buf_group);

	tprint_struct_next();
	PRINT_FIELD_X(arg, head);

	if (!IS_ARRAY_ZERO(arg.resv)) {
		tprint_struct_next();
		PRINT_FIELD_ARRAY(arg, resv, tcp, print_xint_array_member);
	}

	tprint_struct_end();

	return RVAL_DECODED;
}

static int
print_io_uring_napi(struct tcb *tcp, const kernel_ulong_t addr)
{
	struct io_uring_napi arg;

	if (umove_or_printaddr(tcp, addr, &arg))
		return RVAL_DECODED;

	tprint_struct_begin();
	PRINT_FIELD_X(arg, busy_poll_to);

	tprint_struct_next();
	PRINT_FIELD_X(arg, prefer_busy_poll);

	tprint_struct_next();
	PRINT_FIELD_XVAL(arg, opcode, uring_napi_ops, "IO_URING_NAPI_???");

	if (!IS_ARRAY_ZERO(arg.pad)) {
		tprint_struct_next();
		PRINT_FIELD_ARRAY(arg, pad, tcp, print_xint_array_member);
	}

	tprint_struct_next();
	if (arg.opcode == IO_URING_NAPI_REGISTER_OP)
		PRINT_FIELD_XVAL(arg, op_param, uring_napi_tracking_strategies,
				 "IO_URING_NAPI_TRACKING_???");
	else
		PRINT_FIELD_X(arg, op_param);

	if (arg.resv) {
		tprint_struct_next();
		PRINT_FIELD_X(arg, resv);
	}

	tprint_struct_end();

	return 0;
}

static int
print_ioring_register_napi(struct tcb *tcp, const kernel_ulong_t addr,
			   const unsigned int nargs)
{
	if (exiting(tcp)) {
		if (syserror(tcp))
			return 0;
		tprint_value_changed();
	}

	if (nargs != 1) {
		printaddr(addr);
		return RVAL_DECODED;
	}

	return print_io_uring_napi(tcp, addr);
}

static int
print_ioring_unregister_napi(struct tcb *tcp, const kernel_ulong_t addr,
			     const unsigned int nargs)
{
	if (nargs != 1) {
		printaddr(addr);
		return RVAL_DECODED;
	}

	if (entering(tcp))
		return 0;

	print_io_uring_napi(tcp, addr);

	return RVAL_DECODED;
}

static void
print_io_uring_clock_register(struct tcb *tcp, const kernel_ulong_t addr)
{
	struct io_uring_clock_register arg;
	CHECK_TYPE_SIZE(arg, 16);
	CHECK_TYPE_SIZE(arg.__resv, 3 * sizeof(uint32_t));

	if (umove_or_printaddr(tcp, addr, &arg))
		return;

	tprint_struct_begin();
	PRINT_FIELD_XVAL(arg, clockid, clocknames, "CLOCK_???");

	if (!IS_ARRAY_ZERO(arg.__resv)) {
		tprint_struct_next();
		PRINT_FIELD_ARRAY(arg, __resv, tcp, print_xint_array_member);
	}

	tprint_struct_end();
}

static int
print_ioring_register_clock(struct tcb *tcp, const kernel_ulong_t addr,
			    const unsigned int nargs)
{
	if (nargs == 0)
		print_io_uring_clock_register(tcp, addr);
	else
		printaddr(addr);

	return RVAL_DECODED;
}

static void
print_io_uring_clone_buffers(struct tcb *tcp, const kernel_ulong_t addr)
{
	struct io_uring_clone_buffers arg;
	CHECK_TYPE_SIZE(arg, 32);
	CHECK_TYPE_SIZE(arg.pad, 3 * sizeof(uint32_t));

	if (umove_or_printaddr(tcp, addr, &arg))
		return;

	tprint_struct_begin();
	PRINT_FIELD_FD(arg, src_fd, tcp);

	tprint_struct_next();
	PRINT_FIELD_FLAGS(arg, flags, uring_clone_buffers_flags,
			  "IORING_REGISTER_???");

	tprint_struct_next();
	PRINT_FIELD_U(arg, src_off);

	tprint_struct_next();
	PRINT_FIELD_U(arg, dst_off);

	tprint_struct_next();
	PRINT_FIELD_U(arg, nr);

	if (!IS_ARRAY_ZERO(arg.pad)) {
		tprint_struct_next();
		PRINT_FIELD_ARRAY(arg, pad, tcp, print_xint_array_member);
	}

	tprint_struct_end();
}

static int
print_ioring_register_clone_buffers(struct tcb *tcp, const kernel_ulong_t addr,
				    const unsigned int nargs)
{
	if (nargs == 1)
		print_io_uring_clone_buffers(tcp, addr);
	else
		printaddr(addr);

	return RVAL_DECODED;
}

static void
print_io_uring_attr_pi(struct tcb *tcp, const uint64_t addr)
{
	struct io_uring_attr_pi attr;

	CHECK_TYPE_SIZE(struct io_uring_attr_pi, 32);

	if (umove_or_printaddr64(tcp, addr, &attr))
		return;

	tprint_struct_begin();
	PRINT_FIELD_X(attr, flags);
	tprint_struct_next();
	PRINT_FIELD_X(attr, app_tag);
	tprint_struct_next();
	PRINT_FIELD_U(attr, len);
	tprint_struct_next();
	PRINT_FIELD_X(attr, addr);
	tprint_struct_next();
	PRINT_FIELD_X(attr, seed);
	tprint_struct_next();
	PRINT_FIELD_X(attr, rsvd);
	tprint_struct_end();
}

static void
print_io_uring_attr_ptr(struct tcb *tcp, const uint64_t addr,
			unsigned long attr_type_mask)
{
	if (attr_type_mask & IORING_RW_ATTR_FLAG_PI) {
		/* Decode as io_uring_attr_pi structure. */
		print_io_uring_attr_pi(tcp, addr);
	} else {
		printaddr64(addr);
	}
}

static void
print_io_uring_sqe_flags_union(const struct io_uring_sqe *sqe)
{
	switch (sqe->opcode) {
	case IORING_OP_NOP:
	case IORING_OP_NOP128:
		PRINT_FIELD_FLAGS(*sqe, nop_flags, uring_nop_flags,
				 "IORING_NOP_???");
		break;

	case IORING_OP_FIXED_FD_INSTALL:
		PRINT_FIELD_FLAGS(*sqe, install_fd_flags, uring_fixed_fd_flags,
				 "IORING_FIXED_FD_???");
		break;

	case IORING_OP_MSG_RING:
		PRINT_FIELD_FLAGS(*sqe, msg_ring_flags, uring_msg_ring_flags,
				 "IORING_MSG_RING_???");
		break;

	case IORING_OP_PIPE:
		/*
		 * PIPE flags are in the union, but no flags are currently
		 * defined in the kernel header.  Fall through to default
		 * to print as hex (same as rw_flags since they're in the
		 * same union).
		 */
		ATTRIBUTE_FALLTHROUGH;
	default:
		/*
		 * For other opcodes, use rw_flags (default).
		 */
		PRINT_FIELD_X(*sqe, rw_flags);
		break;
	}
}

static void
print_io_uring_sqe_ioprio(const struct io_uring_sqe *sqe)
{
	switch (sqe->opcode) {
	case IORING_OP_ACCEPT:
		/*
		 * For accept operations, ioprio contains flags.
		 */
		PRINT_FIELD_FLAGS(*sqe, ioprio, uring_accept_flags,
				  "IORING_ACCEPT_???");
		break;

	case IORING_OP_SEND:
	case IORING_OP_SENDMSG:
	case IORING_OP_SEND_ZC:
	case IORING_OP_SENDMSG_ZC:
	case IORING_OP_RECV:
	case IORING_OP_RECVMSG:
	case IORING_OP_RECV_ZC:
		/*
		 * For send/recv operations, ioprio contains flags.
		 */
		PRINT_FIELD_FLAGS(*sqe, ioprio, uring_recvsend_flags,
				  "IORING_RECVSEND_???");
		break;

	default:
		/*
		 * For other operations, ioprio contains the actual
		 * ioprio value.
		 */
		PRINT_FIELD_U(*sqe, ioprio);
		break;
	}
}

static bool
is_128_byte_sqe(const struct io_uring_sqe *sqe)
{
	switch (sqe->opcode) {
	case IORING_OP_NOP128:
	case IORING_OP_URING_CMD128:
		return true;

	default:
		return false;
	}
}

static bool
is_op_cmd_sqe(const struct io_uring_sqe *sqe)
{
	switch (sqe->opcode) {
	case IORING_OP_URING_CMD:
	case IORING_OP_URING_CMD128:
		return true;

	default:
		return false;
	}
}

static void
print_io_uring_sqe_off(struct tcb *tcp, const struct io_uring_sqe *sqe)
{
	if (is_op_cmd_sqe(sqe) && getfdinode(tcp, sqe->fd)) {
		/*
		 * For IORING_OP_URING_CMD or IORING_OP_URING_CMD128 with
		 * a socket fd, print off as a socket operation.
		 */
		PRINT_FIELD_XVAL_U(*sqe, off, uring_socket_ops,
				   "SOCKET_URING_OP_???");
	} else {
		/* Not a socket or not URING_CMD/URING_CMD128, print as offset */
		PRINT_FIELD_X(*sqe, off);
	}
}

static void
print_io_uring_sqe(struct tcb *tcp, const kernel_ulong_t addr)
{
	struct io_uring_sqe sqe;

	if (umove_or_printaddr(tcp, addr, &sqe))
		return;

	CHECK_TYPE_SIZE(struct io_uring_sqe, 64);

	tprint_struct_begin();

	PRINT_FIELD_XVAL_U(sqe, opcode, uring_ops, "IORING_OP_???");
	tprint_struct_next();
	PRINT_FIELD_FLAGS(sqe, flags, uring_sqe_flags, "IOSQE_???");
	tprint_struct_next();
	print_io_uring_sqe_ioprio(&sqe);
	tprint_struct_next();
	PRINT_FIELD_FD(sqe, fd, tcp);
	tprint_struct_next();
	print_io_uring_sqe_off(tcp, &sqe);
	tprint_struct_next();
	PRINT_FIELD_X(sqe, addr);
	tprint_struct_next();
	PRINT_FIELD_U(sqe, len);
	tprint_struct_next();
	print_io_uring_sqe_flags_union(&sqe);
	tprint_struct_next();
	PRINT_FIELD_X(sqe, user_data);
	tprint_struct_next();
	PRINT_FIELD_X(sqe, buf_index);
	tprint_struct_next();
	PRINT_FIELD_U(sqe, personality);
	tprint_struct_next();
	PRINT_FIELD_X(sqe, file_index);
	tprint_struct_next();

	/*
	 * Decode union field based on opcode and attr_type_mask.
	 * Note: attr_type_mask/attr_ptr and optval are in the same union.
	 * attr_type_mask is only used for RW operations that call __io_prep_rw.
	 */
	switch (sqe.opcode) {
	case IORING_OP_READ:
	case IORING_OP_WRITE:
	case IORING_OP_READV:
	case IORING_OP_WRITEV:
	case IORING_OP_READ_FIXED:
	case IORING_OP_WRITE_FIXED:
	case IORING_OP_READV_FIXED:
	case IORING_OP_WRITEV_FIXED:
	case IORING_OP_SEND:
	case IORING_OP_RECV:
	case IORING_OP_SENDMSG:
	case IORING_OP_RECVMSG:
	case IORING_OP_SEND_ZC:
	case IORING_OP_RECV_ZC:
	case IORING_OP_READ_MULTISHOT:
		/*
		 * RW operations with attr_type_mask set use attr_ptr/attr_type_mask.
		 */
		if (sqe.attr_type_mask) {
			PRINT_FIELD_FLAGS(sqe, attr_type_mask, uring_rw_attr_flags,
					 "IORING_RW_ATTR_FLAG_???");
			tprint_struct_next();
			PRINT_FIELD_OBJ_TCB_VAL(sqe, attr_ptr, tcp,
						print_io_uring_attr_ptr,
						sqe.attr_type_mask);
			break;
		}
		/* RW operations without attr_type_mask fall through to optval. */
		ATTRIBUTE_FALLTHROUGH;
	default:
		/*
		 * For non-RW operations or RW operations without attr_type_mask,
		 * decode as optval (backward compatible).
		 */
		tprints_field_name("optval");
		PRINT_VAL_X(sqe.optval);
		break;
	}

	/*
	 * For 128-byte SQEs, decode the cmd[] array (64 bytes starting
	 * at offset 64).
	 *
	 * Error handling: If reading the cmd[] array fails, print_array
	 * will print the address gracefully. The base 64-byte structure
	 * is already decoded, so cmd[] failure doesn't affect it.
	 * print_array with tfetch_mem handles partial reads gracefully,
	 * printing what it can if only partial data is available.
	 */
	if (is_128_byte_sqe(&sqe)) {
		const kernel_ulong_t cmd_addr = addr + 64;
		uint8_t cmd_buf;

		tprint_struct_next();
		tprints_field_name("cmd");
		print_array(tcp, cmd_addr, 64, &cmd_buf, sizeof(cmd_buf),
			    tfetch_mem, print_xint_array_member, NULL);
	}

	tprint_struct_end();
}

static int
print_ioring_register_send_msg_ring(struct tcb *tcp, const kernel_ulong_t addr,
				    const unsigned int nargs)
{
	if (nargs == 1)
		print_io_uring_sqe(tcp, addr);
	else
		printaddr(addr);

	return RVAL_DECODED;
}

static void
print_io_uring_zcrx_offsets(struct tcb *tcp,
			    const struct io_uring_zcrx_offsets *const offsets)
{
	tprint_struct_begin();

	PRINT_FIELD_U(*offsets, head);
	tprint_struct_next();
	PRINT_FIELD_U(*offsets, tail);
	tprint_struct_next();
	PRINT_FIELD_U(*offsets, rqes);

	if (offsets->__resv2) {
		tprint_struct_next();
		PRINT_FIELD_X(*offsets, __resv2);
	}

	if (!IS_ARRAY_ZERO(offsets->__resv)) {
		tprint_struct_next();
		PRINT_FIELD_ARRAY(*offsets, __resv, tcp, print_xint_array_member);
	}

	tprint_struct_end();
}

static void
print_io_uring_zcrx_ifq_reg(struct tcb *tcp, const kernel_ulong_t addr)
{
	struct io_uring_zcrx_ifq_reg arg;

	CHECK_TYPE_SIZE(struct io_uring_zcrx_ifq_reg, 96);

	if (umove_or_printaddr(tcp, addr, &arg))
		return;

	tprint_struct_begin();

	PRINT_FIELD_U(arg, if_idx);
	tprint_struct_next();
	PRINT_FIELD_U(arg, if_rxq);
	tprint_struct_next();
	PRINT_FIELD_U(arg, rq_entries);
	tprint_struct_next();
	PRINT_FIELD_FLAGS(arg, flags, uring_zcrx_reg_flags, "ZCRX_REG_???");
	tprint_struct_next();
	PRINT_FIELD_ADDR64(arg, area_ptr);
	tprint_struct_next();
	PRINT_FIELD_ADDR64(arg, region_ptr);
	tprint_struct_next();
	tprints_field_name("offsets");
	print_io_uring_zcrx_offsets(tcp, &arg.offsets);
	tprint_struct_next();
	PRINT_FIELD_U(arg, zcrx_id);

	if (arg.__resv2) {
		tprint_struct_next();
		PRINT_FIELD_X(arg, __resv2);
	}

	if (!IS_ARRAY_ZERO(arg.__resv)) {
		tprint_struct_next();
		PRINT_FIELD_ARRAY(arg, __resv, tcp, print_xint_array_member);
	}

	tprint_struct_end();
}

static int
print_ioring_register_zcrx_ifq(struct tcb *tcp, const kernel_ulong_t addr,
				const unsigned int nargs)
{
	if (nargs == 1)
		print_io_uring_zcrx_ifq_reg(tcp, addr);
	else
		printaddr(addr);

	return RVAL_DECODED;
}

static void
print_zcrx_ctrl_flush_rq(struct tcb *tcp,
			 const struct zcrx_ctrl_flush_rq *const flush)
{
	tprint_struct_begin();

	/* All fields are reserved, only print if non-zero */
	if (!IS_ARRAY_ZERO(flush->__resv)) {
		PRINT_FIELD_ARRAY(*flush, __resv, tcp, print_xint_array_member);
	}

	tprint_struct_end();
}

static void
print_zcrx_ctrl_export(struct tcb *tcp,
		      const struct zcrx_ctrl_export *const export)
{
	tprint_struct_begin();

	/* zcrx_fd is an output field: kernel fills it with new fd on output */
	PRINT_FIELD_FD(*export, zcrx_fd, tcp);

	/* Reserved fields only printed if non-zero */
	if (!IS_ARRAY_ZERO(export->__resv1)) {
		tprint_struct_next();
		PRINT_FIELD_ARRAY(*export, __resv1, tcp, print_xint_array_member);
	}

	tprint_struct_end();
}

static int
print_io_uring_zcrx_ctrl(struct tcb *tcp, const kernel_ulong_t addr)
{
	struct zcrx_ctrl arg;

	/* Compile-time size check - catches ABI mismatches */
	CHECK_TYPE_SIZE(struct zcrx_ctrl, 72);

	if (exiting(tcp)) {
		/*
		 * We only get here if op == ZCRX_CTRL_EXPORT,
		 * other ops return RVAL_DECODED on entry.
		 */
		if (tfetch_obj(tcp, addr, &arg)) {
			tprint_struct_next();
			tprints_field_name("zc_export");
			print_zcrx_ctrl_export(tcp, &arg.zc_export);
		}
		tprint_struct_end();
		return RVAL_DECODED;
	}

	/* On entry, print input fields */
	/* Copy structure from tracee's memory */
	if (umove_or_printaddr(tcp, addr, &arg))
		return RVAL_DECODED;  /* Invalid address, already printed */

	/* On entry, print input fields */
	tprint_struct_begin();

	/* zcrx_id: ZCRX resource identifier */
	PRINT_FIELD_U(arg, zcrx_id);

	/* op: Control operation type */
	tprint_struct_next();
	PRINT_FIELD_XVAL(arg, op, uring_zcrx_ctrl_ops, "ZCRX_CTRL_???");
	/* If op is unknown, prints as hex with "ZCRX_CTRL_???" fallback */

	/* Reserved fields: only print if non-zero */
	if (!IS_ARRAY_ZERO(arg.__resv)) {
		tprint_struct_next();
		PRINT_FIELD_ARRAY(arg, __resv, tcp, print_xint_array_member);
	}

	/* Union member: decode based on op value */
	switch (arg.op) {
	case ZCRX_CTRL_FLUSH_RQ:
		tprint_struct_next();
		tprints_field_name("zc_flush");
		print_zcrx_ctrl_flush_rq(tcp, &arg.zc_flush);
		/* No output fields, nothing to decode on exit */
		tprint_struct_end();
		return RVAL_DECODED;

	case ZCRX_CTRL_EXPORT:
		/* On entry, skip zc_export (zcrx_fd is 0, no meaningful input) */
		/* Don't close structure - it will be closed on exit after adding zc_export */
		/* Return 0 to indicate we need to decode on exit */
		return 0;

	default:
		/* Unknown opcode - skip union printing */
		tprint_struct_end();
		/* Unknown opcode, no output fields to decode on exit */
		return RVAL_DECODED;
	}
}

static int
print_ioring_register_zcrx_ctrl(struct tcb *tcp, const kernel_ulong_t addr,
				const unsigned int nargs)
{
	if (nargs != 1) {
		printaddr(addr);  /* Invalid nargs, just print address */
		return RVAL_DECODED;
	}

	return print_io_uring_zcrx_ctrl(tcp, addr);
}

static void
print_io_uring_params_input(struct tcb *tcp, const kernel_ulong_t addr)
{
	struct io_uring_params params;

	if (umove_or_printaddr(tcp, addr, &params))
		return;

	CHECK_TYPE_SIZE(struct io_uring_params, 120);

	tprint_struct_begin();
	PRINT_FIELD_U(params, sq_entries);
	tprint_struct_next();
	PRINT_FIELD_U(params, cq_entries);
	tprint_struct_next();
	PRINT_FIELD_FLAGS(params, flags, uring_setup_flags,
			  "IORING_SETUP_???");

	/* Print user_addr fields if non-zero (only meaningful if
	 * IORING_SETUP_NO_MMAP was set during original io_uring_setup(),
	 * but we can't know that from the register call alone) */
	if (params.sq_off.user_addr) {
		tprint_struct_next();
		tprints_field_name("sq_off");
		tprint_struct_begin();
		PRINT_FIELD_X(params.sq_off, user_addr);
		tprint_struct_end();
	}
	if (params.cq_off.user_addr) {
		tprint_struct_next();
		tprints_field_name("cq_off");
		tprint_struct_begin();
		PRINT_FIELD_X(params.cq_off, user_addr);
		tprint_struct_end();
	}

	if (!IS_ARRAY_ZERO(params.resv)) {
		tprint_struct_next();
		PRINT_FIELD_ARRAY(params, resv, tcp, print_xint_array_member);
	}

	tprint_struct_end();
}

static int
print_ioring_register_resize_rings(struct tcb *tcp, const kernel_ulong_t addr,
				    const unsigned int nargs)
{
	if (nargs == 1)
		print_io_uring_params_input(tcp, addr);
	else
		printaddr(addr);

	return RVAL_DECODED;
}

static void
print_io_uring_mem_region_reg(struct tcb *tcp, const kernel_ulong_t addr)
{
	struct io_uring_mem_region_reg arg;

	CHECK_TYPE_SIZE(struct io_uring_mem_region_reg, 32);

	if (umove_or_printaddr(tcp, addr, &arg))
		return;

	tprint_struct_begin();

	PRINT_FIELD_ADDR64(arg, region_uptr);
	tprint_struct_next();
	PRINT_FIELD_FLAGS(arg, flags, uring_mem_region_reg_flags,
			  "IORING_MEM_REGION_REG_???");

	if (!IS_ARRAY_ZERO(arg.__resv)) {
		tprint_struct_next();
		PRINT_FIELD_ARRAY(arg, __resv, tcp, print_xint_array_member);
	}

	tprint_struct_end();
}

static int
print_ioring_register_mem_region(struct tcb *tcp, const kernel_ulong_t addr,
				 const unsigned int nargs)
{
	if (nargs == 1)
		print_io_uring_mem_region_reg(tcp, addr);
	else
		printaddr(addr);

	return RVAL_DECODED;
}

static void
print_io_uring_query_opcode(struct tcb *tcp, const kernel_ulong_t addr)
{
	struct io_uring_query_opcode opcode;

	if (umove_or_printaddr(tcp, addr, &opcode))
		return;

	CHECK_TYPE_SIZE(struct io_uring_query_opcode, 48);

	tprint_struct_begin();
	PRINT_FIELD_U(opcode, nr_request_opcodes);
	tprint_struct_next();
	PRINT_FIELD_U(opcode, nr_register_opcodes);
	tprint_struct_next();
	PRINT_FIELD_FLAGS(opcode, feature_flags, uring_setup_features,
			  "IORING_FEAT_???");
	tprint_struct_next();
	PRINT_FIELD_FLAGS(opcode, ring_setup_flags, uring_setup_flags,
			  "IORING_SETUP_???");
	tprint_struct_next();
	PRINT_FIELD_FLAGS(opcode, enter_flags, uring_enter_flags,
			  "IORING_ENTER_???");
	tprint_struct_next();
	PRINT_FIELD_FLAGS(opcode, sqe_flags, uring_sqe_flags,
			  "IOSQE_???");
	tprint_struct_next();
	PRINT_FIELD_U(opcode, nr_query_opcodes);

	if (opcode.__pad) {
		tprint_struct_next();
		PRINT_FIELD_X(opcode, __pad);
	}

	tprint_struct_end();
}

static void
print_io_uring_query_zcrx(struct tcb *tcp, const kernel_ulong_t addr)
{
	struct io_uring_query_zcrx zcrx;

	if (umove_or_printaddr(tcp, addr, &zcrx))
		return;

	CHECK_TYPE_SIZE(struct io_uring_query_zcrx, 40);

	tprint_struct_begin();
	PRINT_FIELD_FLAGS(zcrx, register_flags, uring_zcrx_reg_flags,
			  "ZCRX_REG_???");
	tprint_struct_next();
	PRINT_FIELD_FLAGS(zcrx, area_flags, uring_zcrx_area_flags,
			  "IORING_ZCRX_AREA_???");
	tprint_struct_next();
	PRINT_FIELD_U(zcrx, nr_ctrl_opcodes);
	tprint_struct_next();
	PRINT_FIELD_U(zcrx, rq_hdr_size);
	tprint_struct_next();
	PRINT_FIELD_U(zcrx, rq_hdr_alignment);

	if (zcrx.__resv1) {
		tprint_struct_next();
		PRINT_FIELD_X(zcrx, __resv1);
	}

	if (zcrx.__resv2) {
		tprint_struct_next();
		PRINT_FIELD_X(zcrx, __resv2);
	}

	tprint_struct_end();
}

static void
print_io_uring_query_scq(struct tcb *tcp, const kernel_ulong_t addr)
{
	struct io_uring_query_scq scq;

	if (umove_or_printaddr(tcp, addr, &scq))
		return;

	CHECK_TYPE_SIZE(struct io_uring_query_scq, 16);

	tprint_struct_begin();
	PRINT_FIELD_U(scq, hdr_size);
	tprint_struct_next();
	PRINT_FIELD_U(scq, hdr_alignment);
	tprint_struct_end();
}

static void
print_io_uring_query_list(struct tcb *tcp, kernel_ulong_t addr)
{
#define IO_MAX_QUERY_ENTRIES 1000
	struct io_uring_query_hdr hdr;
	CHECK_TYPE_SIZE(hdr, 40);

	unsigned int count = 0;

	for (;;) {
		if (umove_or_printaddr(tcp, addr, &hdr))
			break;

		if (count >= IO_MAX_QUERY_ENTRIES ||
		    sequence_truncation_needed(tcp, count + 1)) {
			printaddr(addr);
			tprintf_comment("...");
			break;
		}

		tprint_struct_begin();
		++count;

		/* Print all fields of current entry (except next_entry) */
		PRINT_FIELD_ADDR64(hdr, query_data);
		tprint_struct_next();
		PRINT_FIELD_XVAL(hdr, query_op, uring_query_ops,
				 "IO_URING_QUERY_???");
		tprint_struct_next();
		PRINT_FIELD_U(hdr, size);
		tprint_struct_next();
		PRINT_FIELD_D(hdr, result);

		if (!IS_ARRAY_ZERO(hdr.__resv)) {
			tprint_struct_next();
			PRINT_FIELD_ARRAY(hdr, __resv, tcp, print_xint_array_member);
		}

		/* Decode query_data if present */
		if (hdr.query_data && hdr.size > 0) {
			tprint_struct_next();
			tprints_field_name("query_data");
			switch (hdr.query_op) {
			case IO_URING_QUERY_OPCODES:
				print_io_uring_query_opcode(tcp, hdr.query_data);
				break;
			case IO_URING_QUERY_ZCRX:
				print_io_uring_query_zcrx(tcp, hdr.query_data);
				break;
			case IO_URING_QUERY_SCQ:
				print_io_uring_query_scq(tcp, hdr.query_data);
				break;
			default:
				printaddr(hdr.query_data);
				break;
			}
		}

		/* Handle next_entry field */
		tprint_struct_next();
		tprints_field_name("next_entry");

		addr = (kernel_ulong_t) hdr.next_entry;
	}

	/* Close any remaining open structures */
	while (count > 0) {
		tprint_struct_end();
		--count;
	}
}

static int
print_ioring_register_query(struct tcb *tcp, const kernel_ulong_t addr,
			    const unsigned int nargs)
{
	if (nargs != 0) {
		printaddr(addr);
		return RVAL_DECODED;
	}

	print_io_uring_query_list(tcp, addr);
	return RVAL_DECODED;
}

static void
print_io_uring_register_opcode(struct tcb *tcp, const unsigned int opcode,
			       const unsigned int flags)
{
	if (flags)
		tprint_flags_begin();

	printxval(uring_register_opcodes, opcode, "IORING_REGISTER_???");

	if (flags) {
		tprint_flags_or();
		printflags_in(uring_register_opcode_flags, flags, NULL);
		tprint_flags_end();
	}

}

SYS_FUNC(io_uring_register)
{
	const int fd = tcp->u_arg[0];
	unsigned int opcode = tcp->u_arg[1];
	const kernel_ulong_t arg = tcp->u_arg[2];
	const unsigned int nargs = tcp->u_arg[3];
	const unsigned int opcode_flags =
		opcode & IORING_REGISTER_USE_REGISTERED_RING;
	opcode &= ~IORING_REGISTER_USE_REGISTERED_RING;
	int rc = RVAL_DECODED;
	int buf;

	if (entering(tcp)) {
		/* fd */
		tprints_arg_name("fd");
		if (opcode_flags)
			PRINT_VAL_U(fd);
		else
			printfd(tcp, fd);

		/* opcode */
		tprints_arg_next_name("opcode");
		print_io_uring_register_opcode(tcp, opcode, opcode_flags);

		tprints_arg_next_name("arg");
	}

	/* arg */
	switch (opcode) {
	case IORING_REGISTER_BUFFERS:
		tprint_iov(tcp, nargs, arg, iov_decode_addr);
		break;
	case IORING_REGISTER_FILES:
	case IORING_REGISTER_EVENTFD:
	case IORING_REGISTER_EVENTFD_ASYNC:
		print_array(tcp, arg, nargs, &buf, sizeof(buf),
			    tfetch_mem, print_fd_array_member, NULL);
		break;
	case IORING_REGISTER_FILES_UPDATE:
		print_io_uring_files_update(tcp, arg, nargs);
		break;
	case IORING_REGISTER_PROBE:
		rc = print_io_uring_probe(tcp, arg, nargs);
		break;
	case IORING_REGISTER_RESTRICTIONS:
		print_io_uring_restrictions(tcp, arg, nargs);
		break;
	case IORING_REGISTER_FILES2:
	case IORING_REGISTER_BUFFERS2:
		print_io_uring_register_rsrc(tcp, arg, nargs, opcode);
		break;
	case IORING_REGISTER_FILES_UPDATE2:
	case IORING_REGISTER_BUFFERS_UPDATE:
		print_io_uring_update_rsrc(tcp, arg, nargs, opcode);
		break;
	case IORING_REGISTER_IOWQ_AFF:
		print_affinitylist(tcp, arg, nargs);
		break;
	case IORING_REGISTER_IOWQ_MAX_WORKERS:
		rc = print_io_uring_iowq_acct(tcp, arg, nargs);
		if (entering(tcp) && !rc)
			tprint_value_changed();
		break;
	case IORING_REGISTER_RING_FDS:
		print_io_uring_ringfds_register(tcp, arg, nargs);
		break;
	case IORING_UNREGISTER_RING_FDS:
		print_io_uring_ringfds_unregister(tcp, arg, nargs);
		break;
	case IORING_REGISTER_PBUF_RING:
	case IORING_UNREGISTER_PBUF_RING:
		print_io_uring_buf_reg(tcp, arg, nargs);
		break;
	case IORING_REGISTER_SYNC_CANCEL:
		print_io_uring_sync_cancel_reg(tcp, arg, nargs);
		break;
	case IORING_REGISTER_FILE_ALLOC_RANGE:
		print_io_uring_file_index_range(tcp, arg, nargs);
		break;
	case IORING_REGISTER_PBUF_STATUS:
		rc = print_io_uring_buf_status(tcp, arg, nargs);
		break;
	case IORING_REGISTER_NAPI:
		rc = print_ioring_register_napi(tcp, arg, nargs);
		break;
	case IORING_UNREGISTER_NAPI:
		rc = print_ioring_unregister_napi(tcp, arg, nargs);
		break;
	case IORING_REGISTER_CLOCK:
		rc = print_ioring_register_clock(tcp, arg, nargs);
		break;
	case IORING_REGISTER_CLONE_BUFFERS:
		rc = print_ioring_register_clone_buffers(tcp, arg, nargs);
		break;
	case IORING_REGISTER_SEND_MSG_RING:
		rc = print_ioring_register_send_msg_ring(tcp, arg, nargs);
		break;
	case IORING_REGISTER_ZCRX_IFQ:
		rc = print_ioring_register_zcrx_ifq(tcp, arg, nargs);
		break;
	case IORING_REGISTER_ZCRX_CTRL:
		rc = print_ioring_register_zcrx_ctrl(tcp, arg, nargs);
		break;
	case IORING_REGISTER_RESIZE_RINGS:
		rc = print_ioring_register_resize_rings(tcp, arg, nargs);
		break;
	case IORING_REGISTER_MEM_REGION:
		rc = print_ioring_register_mem_region(tcp, arg, nargs);
		break;
	case IORING_REGISTER_QUERY:
		rc = print_ioring_register_query(tcp, arg, nargs);
		break;
	case IORING_UNREGISTER_BUFFERS:
	case IORING_UNREGISTER_FILES:
	case IORING_UNREGISTER_EVENTFD:
	case IORING_REGISTER_PERSONALITY:
	case IORING_UNREGISTER_PERSONALITY:
	case IORING_REGISTER_ENABLE_RINGS:
	case IORING_UNREGISTER_IOWQ_AFF:
	default:
		printaddr(arg);
		break;
	}

	if (rc || exiting(tcp)) {
		/* nr_args */
		tprints_arg_next_name("nr_args");
		PRINT_VAL_U(nargs);
	}

	return rc;
}
