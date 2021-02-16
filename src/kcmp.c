/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "xlat/kcmp_types.h"

struct strace_kcmp_epoll_slot {
	uint32_t efd;
	uint32_t tfd;
	uint32_t toff;
};

#define PRINT_FIELD_PIDFD(where_, field_, tcp_, pid_)			\
	do {								\
		tprints_field_name(#field_);				\
		printfd_pid_tracee_ns((tcp_), (pid_), (where_).field_);	\
	} while (0)

SYS_FUNC(kcmp)
{
	pid_t pid1 = tcp->u_arg[0];
	pid_t pid2 = tcp->u_arg[1];
	int type = tcp->u_arg[2];
	kernel_ulong_t idx1 = tcp->u_arg[3];
	kernel_ulong_t idx2 = tcp->u_arg[4];

	printpid(tcp, pid1, PT_TGID);
	tprints(", ");
	printpid(tcp, pid2, PT_TGID);
	tprints(", ");
	printxval(kcmp_types, type, "KCMP_???");

	switch (type) {
		case KCMP_FILE:
			tprints(", ");
			printfd_pid_tracee_ns(tcp, pid1, idx1);
			tprints(", ");
			printfd_pid_tracee_ns(tcp, pid2, idx2);

			break;

		case KCMP_EPOLL_TFD: {
			struct strace_kcmp_epoll_slot slot;

			tprints(", ");
			printfd_pid_tracee_ns(tcp, pid1, idx1);
			tprints(", ");

			if (umove_or_printaddr(tcp, idx2, &slot))
				break;

			tprint_struct_begin();
			PRINT_FIELD_PIDFD(slot, efd, tcp, pid2);
			tprint_struct_next();
			PRINT_FIELD_PIDFD(slot, tfd, tcp, pid2);
			tprint_struct_next();
			PRINT_FIELD_U(slot, toff);
			tprint_struct_end();

			break;
		}

		case KCMP_FILES:
		case KCMP_FS:
		case KCMP_IO:
		case KCMP_SIGHAND:
		case KCMP_SYSVSEM:
		case KCMP_VM:
			break;
		default:
			tprintf(", %#" PRI_klx ", %#" PRI_klx, idx1, idx2);
	}

	return RVAL_DECODED;
}
