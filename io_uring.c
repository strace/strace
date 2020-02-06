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

#include "xlat/uring_setup_features.h"
#include "xlat/uring_setup_flags.h"
#include "xlat/uring_enter_flags.h"
#include "xlat/uring_register_opcodes.h"

#ifdef HAVE_STRUCT_IO_CQRING_OFFSETS
# ifdef HAVE_STRUCT_IO_CQRING_OFFSETS_RESV
static_assert(offsetof(struct_io_cqring_offsets, resv)
             >= offsetof(struct io_cqring_offsets, resv),
             "struct io_cqring_offsets.resv offset mismatch"
             ", please update the decoder");
static_assert(sizeof_field(struct_io_cqring_offsets, resv)
             <= sizeof_field(struct io_cqring_offsets, resv),
             "struct io_cqring_offsets.resv size mismatch"
             ", please update the decoder");
# else /* !HAVE_STRUCT_IO_CQRING_OFFSETS_RESV */
static_assert(0, "struct io_cqring_offsets.resv is missing"
		 ", please update the decoder");
# endif
#endif /* HAVE_STRUCT_IO_CQRING_OFFSETS */
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
	} else {
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
			if (!IS_ARRAY_ZERO(params.cq_off.resv)) {
				PRINT_FIELD_ARRAY(", ", params.cq_off, resv, tcp,
						  print_xint64_array_member);
			}
			tprints("}");
		}
		tprints("}");
	}

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

SYS_FUNC(io_uring_register)
{
	const int fd = tcp->u_arg[0];
	const unsigned int opcode = tcp->u_arg[1];
	const kernel_ulong_t arg = tcp->u_arg[2];
	const unsigned int nargs = tcp->u_arg[3];
	int buf;

	printfd(tcp, fd);
	tprints(", ");
	printxval(uring_register_opcodes, opcode, "IORING_REGISTER_???");
	tprints(", ");
	switch (opcode) {
	case IORING_REGISTER_BUFFERS:
		tprint_iov(tcp, nargs, arg, IOV_DECODE_ADDR);
		break;
	case IORING_REGISTER_FILES:
	case IORING_REGISTER_EVENTFD:
		print_array(tcp, arg, nargs, &buf, sizeof(buf),
			    tfetch_mem, print_fd_array_member, NULL);
		break;
	case IORING_REGISTER_FILES_UPDATE:
		print_io_uring_files_update(tcp, arg, nargs);
		break;
	default:
		printaddr(arg);
		break;
	}
	tprintf(", %u", nargs);

	return RVAL_DECODED;
}
