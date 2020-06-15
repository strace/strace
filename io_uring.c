/*
 * Copyright (c) 2019 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2019-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "print_fields.h"

#include "types/io_uring.h"

#include "xlat/uring_op_flags.h"
#include "xlat/uring_ops.h"
#include "xlat/uring_setup_features.h"
#include "xlat/uring_setup_flags.h"
#include "xlat/uring_enter_flags.h"
#include "xlat/uring_register_opcodes.h"
#include "xlat/uring_cqring_flags.h"

#ifdef HAVE_STRUCT_IO_URING_PARAMS
# ifdef HAVE_STRUCT_IO_URING_PARAMS_RESV
static_assert(offsetof(struct_io_uring_params, resv)
             >= offsetof(struct io_uring_params, resv),
             "struct io_uring_params.resv offset mismatch"
             ", please update the decoder");
static_assert(sizeof_field(struct_io_uring_params, resv)
             <= sizeof_field(struct io_uring_params, resv),
             "struct io_uring_params.resv size mismatch"
             ", please update the decoder");
# else /* !HAVE_STRUCT_IO_URING_PARAMS_RESV */
static_assert(0, "struct io_uring_params.resv is missing"
		 ", please update the decoder");
# endif
#endif /* HAVE_STRUCT_IO_URING_PARAMS */


SYS_FUNC(io_uring_setup)
{
	const uint32_t nentries = tcp->u_arg[0];
	const kernel_ulong_t params_addr = tcp->u_arg[1];
	struct_io_uring_params params;

	if (entering(tcp)) {
		tprintf("%u, ", nentries);

		if (umove_or_printaddr(tcp, params_addr, &params))
			return RVAL_DECODED | RVAL_FD;

		PRINT_FIELD_FLAGS("{", params, flags, uring_setup_flags,
				  "IORING_SETUP_???");
		PRINT_FIELD_X(", ", params, sq_thread_cpu);
		PRINT_FIELD_U(", ", params, sq_thread_idle);
		if (params.flags & IORING_SETUP_ATTACH_WQ)
			PRINT_FIELD_FD(", ", params, wq_fd, tcp);
		if (!IS_ARRAY_ZERO(params.resv)) {
			PRINT_FIELD_ARRAY(", ", params, resv, tcp,
					  print_xint32_array_member);
		}
		return 0;
	}

	/* exiting */
	if (syserror(tcp)) {
		/* The remaining part of params is irrelevant.  */
	} else if (umove(tcp, params_addr, &params)) {
		tprints(", ???");
	} else {
		PRINT_FIELD_U(", ", params, sq_entries);
		PRINT_FIELD_U(", ", params, cq_entries);
		PRINT_FIELD_FLAGS(", ", params, features,
				  uring_setup_features,
				  "IORING_FEAT_???");
		PRINT_FIELD_U(", sq_off={", params.sq_off, head);
		PRINT_FIELD_U(", ", params.sq_off, tail);
		PRINT_FIELD_U(", ", params.sq_off, ring_mask);
		PRINT_FIELD_U(", ", params.sq_off, ring_entries);
		PRINT_FIELD_U(", ", params.sq_off, flags);
		PRINT_FIELD_U(", ", params.sq_off, dropped);
		PRINT_FIELD_U(", ", params.sq_off, array);
		if (params.sq_off.resv1)
			PRINT_FIELD_X(", ", params.sq_off, resv1);
		if (params.sq_off.resv2)
			PRINT_FIELD_X(", ", params.sq_off, resv2);
		PRINT_FIELD_U("}, cq_off={", params.cq_off, head);
		PRINT_FIELD_U(", ", params.cq_off, tail);
		PRINT_FIELD_U(", ", params.cq_off, ring_mask);
		PRINT_FIELD_U(", ", params.cq_off, ring_entries);
		PRINT_FIELD_U(", ", params.cq_off, overflow);
		PRINT_FIELD_U(", ", params.cq_off, cqes);
		PRINT_FIELD_FLAGS(", ", params.cq_off, flags,
				  uring_cqring_flags, "IORING_CQ_???");
		if (params.cq_off.resv1)
			PRINT_FIELD_X(", ", params.cq_off, resv1);
		if (params.cq_off.resv2)
			PRINT_FIELD_X(", ", params.cq_off, resv2);
		tprints("}");
	}
	tprints("}");

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

	printfd(tcp, fd);
	tprintf(", %u, %u, ", to_submit, min_complete);
	printflags(uring_enter_flags, flags, "IORING_ENTER_???");
	tprints(", ");
	print_sigset_addr_len(tcp, sigset_addr, sigset_size);
	tprintf(", %" PRI_klu, sigset_size);

	return RVAL_DECODED;
}

static bool
print_fd_array_member(struct tcb *tcp, void *elem_buf, size_t elem_size,
		      void *data)
{
	printfd(tcp, *(int *) elem_buf);
	return true;
}

static void
print_io_uring_files_update(struct tcb *tcp, const kernel_ulong_t addr,
			    const unsigned int nargs)
{
	struct_io_uring_files_update arg;
	int buf;

	if (umove_or_printaddr(tcp, addr, &arg))
		return;

	PRINT_FIELD_U("{", arg, offset);
	if (arg.resv)
		PRINT_FIELD_X(", ", arg, resv);
	tprints(", fds=");
	print_big_u64_addr(arg.fds);
	print_array(tcp, arg.fds, nargs, &buf, sizeof(buf),
		    tfetch_mem, print_fd_array_member, NULL);
	tprints("}");
}

static bool
print_io_uring_probe_op(struct tcb *tcp, void *elem_buf, size_t elem_size,
			void *data)
{
	struct_io_uring_probe_op *op = (struct_io_uring_probe_op *) elem_buf;

	PRINT_FIELD_XVAL_U("{", *op, op, uring_ops, "IORING_OP_???");
	if (op->resv)
		PRINT_FIELD_X(", ", *op, resv);
	PRINT_FIELD_FLAGS(", ", *op, flags, uring_op_flags, "IO_URING_OP_???");
	if (op->resv2)
		PRINT_FIELD_X(", ", *op, resv2);
	tprints("}");

	return true;
}

static int
print_io_uring_probe(struct tcb *tcp, const kernel_ulong_t addr,
		     const unsigned int nargs)
{
	struct_io_uring_probe *probe;
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
		tprints(" => ");

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

	PRINT_FIELD_XVAL_U("{", *probe, last_op, uring_ops, "IORING_OP_???");
	PRINT_FIELD_U(", ", *probe, ops_len);
	if (probe->resv)
		PRINT_FIELD_X(", ", *probe, resv);
	if (!IS_ARRAY_ZERO(probe->resv2)) {
		PRINT_FIELD_ARRAY(", ", *probe, resv2, tcp,
				  print_xint32_array_member);
	}
	tprints(", ops=");
	print_local_array_ex(tcp, probe->ops,
			     entering(tcp) ? nargs : probe->ops_len,
			     sizeof(probe->ops[0]), print_io_uring_probe_op,
			     NULL, 0, NULL, NULL);
	tprints("}");

	return 0;
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
		printfd(tcp, fd);
		tprints(", ");
		printxval(uring_register_opcodes, opcode,
			  "IORING_REGISTER_???");
		tprints(", ");
	}

	switch (opcode) {
	case IORING_REGISTER_BUFFERS:
		tprint_iov(tcp, nargs, arg, IOV_DECODE_ADDR);
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
	default:
		printaddr(arg);
		break;
	}

	if (rc || exiting(tcp))
		tprintf(", %u", nargs);

	return rc;
}
