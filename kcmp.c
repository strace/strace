/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "print_fields.h"
#include "xlat/kcmp_types.h"

struct strace_kcmp_epoll_slot {
	uint32_t efd;
	uint32_t tfd;
	uint32_t toff;
};

static void
printpidfd(struct tcb *tcp, pid_t pid, int fd)
{
	/*
	 * XXX We want to use printfd here, but we should figure out which
	 *     process in strace's PID NS is referred to first.
	 */
	tprintf("%d", fd);
}

#define PRINT_FIELD_PIDFD(prefix_, where_, field_, tcp_, pid_)		\
	do {								\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);		\
		printpidfd((tcp_), (pid_), (where_).field_);		\
	} while (0)

SYS_FUNC(kcmp)
{
	pid_t pid1 = tcp->u_arg[0];
	pid_t pid2 = tcp->u_arg[1];
	int type = tcp->u_arg[2];
	kernel_ulong_t idx1 = tcp->u_arg[3];
	kernel_ulong_t idx2 = tcp->u_arg[4];

	tprintf("%d, %d, ", pid1, pid2);
	printxval(kcmp_types, type, "KCMP_???");

	switch (type) {
		case KCMP_FILE:
			tprints(", ");
			printpidfd(tcp, pid1, idx1);
			tprints(", ");
			printpidfd(tcp, pid1, idx2);

			break;

		case KCMP_EPOLL_TFD: {
			struct strace_kcmp_epoll_slot slot;

			tprints(", ");
			printpidfd(tcp, pid1, idx1);
			tprints(", ");

			if (umove_or_printaddr(tcp, idx2, &slot))
				break;

			PRINT_FIELD_PIDFD("{",  slot, efd, tcp, pid2);
			PRINT_FIELD_PIDFD(", ", slot, tfd, tcp, pid2);
			PRINT_FIELD_U(", ", slot, toff);
			tprints("}");

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
