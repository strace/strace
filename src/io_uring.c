/*
 * Copyright (c) 2019 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2019-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <linux/io_uring.h>

#include "xlat/uring_cqring_flags.h"
#include "xlat/uring_enter_flags.h"
#include "xlat/uring_files_update_fds.h"
#include "xlat/uring_op_flags.h"
#include "xlat/uring_ops.h"
#include "xlat/uring_setup_features.h"
#include "xlat/uring_setup_flags.h"
#include "xlat/uring_sqe_flags.h"
#include "xlat/uring_register_opcodes.h"
#include "xlat/uring_restriction_opcodes.h"

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
	if (p->resv2) {
		tprint_struct_next();
		PRINT_FIELD_X(*p, resv2);
	}
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
	PRINT_FIELD_FLAGS(*p, flags, uring_cqring_flags, "IORING_CQ_???");
	if (p->resv1) {
		tprint_struct_next();
		PRINT_FIELD_X(*p, resv1);
	}
	if (p->resv2) {
		tprint_struct_next();
		PRINT_FIELD_X(*p, resv2);
	}
	tprint_struct_end();
}

SYS_FUNC(io_uring_setup)
{
	const uint32_t entries = tcp->u_arg[0];
	const kernel_ulong_t params_addr = tcp->u_arg[1];
	struct io_uring_params params;

	if (entering(tcp)) {
		/* entries */
		PRINT_VAL_U(entries);
		tprint_arg_next();

		/* params */
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

SYS_FUNC(io_uring_enter)
{
	const int fd = tcp->u_arg[0];
	const uint32_t to_submit = tcp->u_arg[1];
	const uint32_t min_complete = tcp->u_arg[2];
	const uint32_t flags = tcp->u_arg[3];
	const kernel_ulong_t sigset_addr = tcp->u_arg[4];
	const kernel_ulong_t sigset_size = tcp->u_arg[5];

	/* fd */
	printfd(tcp, fd);
	tprint_arg_next();

	/* to_submit */
	PRINT_VAL_U(to_submit);
	tprint_arg_next();

	/* min_complete */
	PRINT_VAL_U(min_complete);
	tprint_arg_next();

	/* flags */
	printflags(uring_enter_flags, flags, "IORING_ENTER_???");
	tprint_arg_next();

	/* sigset */
	print_sigset_addr_len(tcp, sigset_addr, sigset_size);
	tprint_arg_next();

	/* sigsetsize */
	PRINT_VAL_U(sigset_size);

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
			entering(tcp) ? nargs : probe->ops_len,
			sizeof(probe->ops[0]), print_io_uring_probe_op,
			NULL, 0, NULL, NULL);
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
	CHECK_TYPE_SIZE(arg.resv, sizeof(uint32_t));
	CHECK_TYPE_SIZE(arg.resv2, sizeof(uint64_t));

	if (size < 32) {
		printaddr(addr);
		return;
	}

	if (umove_or_printaddr(tcp, addr, &arg))
		return;

	tprint_struct_begin();
	PRINT_FIELD_U(arg, nr);

	if (arg.resv) {
		tprint_struct_next();
		PRINT_FIELD_X(arg, resv);
	}

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

SYS_FUNC(io_uring_register)
{
	const int fd = tcp->u_arg[0];
	const unsigned int opcode = tcp->u_arg[1];
	const kernel_ulong_t arg = tcp->u_arg[2];
	const unsigned int nargs = tcp->u_arg[3];
	int rc = RVAL_DECODED;
	int buf;

	if (entering(tcp)) {
		/* fd */
		printfd(tcp, fd);
		tprint_arg_next();

		/* opcode */
		printxval(uring_register_opcodes, opcode,
			  "IORING_REGISTER_???");
		tprint_arg_next();
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
		tprint_arg_next();
		/* nr_args */
		PRINT_VAL_U(nargs);
	}

	return rc;
}
