/*
 * Copyright (c) 2019 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#ifdef HAVE_LINUX_IO_URING_H
# include "print_fields.h"
# include <linux/io_uring.h>
#endif

#include "xlat/uring_setup_flags.h"
#include "xlat/uring_enter_flags.h"
#include "xlat/uring_register_opcodes.h"

SYS_FUNC(io_uring_setup)
{
	const uint32_t nentries = tcp->u_arg[0];
	const kernel_ulong_t params_addr = tcp->u_arg[1];

#ifdef HAVE_LINUX_IO_URING_H
	struct io_uring_params params;

	if (entering(tcp)) {
		tprintf("%u, ", nentries);

		if (umove_or_printaddr(tcp, params_addr, &params))
			return RVAL_DECODED | RVAL_FD;

		PRINT_FIELD_FLAGS("{", params, flags, uring_setup_flags,
				  "IORING_SETUP_???");
		PRINT_FIELD_X(", ", params, sq_thread_cpu);
		PRINT_FIELD_U(", ", params, sq_thread_idle);
		for (unsigned int i = 0; i < ARRAY_SIZE(params.resv); ++i) {
			if (params.resv[i]) {
				for (i = 0; i < ARRAY_SIZE(params.resv); ++i)
					tprintf("%s%#x",
						(i ? ", " : ", resv={"),
						params.resv[i]);
				tprints("}");
				break;
			}
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
			PRINT_FIELD_U(", sq_off={", params.sq_off, head);
			PRINT_FIELD_U(", ", params.sq_off, tail);
			PRINT_FIELD_U(", ", params.sq_off, ring_mask);
			PRINT_FIELD_U(", ", params.sq_off, ring_entries);
			PRINT_FIELD_U(", ", params.sq_off, flags);
			PRINT_FIELD_U(", ", params.sq_off, dropped);
			PRINT_FIELD_U(", ", params.sq_off, array);
			PRINT_FIELD_U("}, cq_off={", params.cq_off, head);
			PRINT_FIELD_U(", ", params.cq_off, tail);
			PRINT_FIELD_U(", ", params.cq_off, ring_mask);
			PRINT_FIELD_U(", ", params.cq_off, ring_entries);
			PRINT_FIELD_U(", ", params.cq_off, overflow);
			PRINT_FIELD_U(", ", params.cq_off, cqes);
			tprints("}");
		}
		tprints("}");
	}
#else /* !HAVE_LINUX_IO_URING_H */
	tprintf("%u, ", nentries);
	printaddr(params_addr);
#endif

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
			print_array(tcp, arg, nargs, &buf, sizeof(buf),
				    tfetch_mem, print_fd_array_member, NULL);
			break;
		default:
			printaddr(arg);
			break;
	}
	tprintf(", %u", nargs);

	return RVAL_DECODED;
}
