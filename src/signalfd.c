/*
 * Copyright (c) 2008-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "kernel_fcntl.h"
#include "xlat/sfd_flags.h"

static int
do_signalfd(struct tcb *tcp, int flags_arg)
{
	/* fd */
	tprints_arg_name("fd");
	printfd(tcp, tcp->u_arg[0]);

	/* NB: kernel requires arg[2] == NSIG_BYTES */
	/* mask */
	tprints_arg_next_name("mask");
	print_sigset_addr_len(tcp, tcp->u_arg[1], tcp->u_arg[2]);

	/* sizemask */
	tprints_arg_next_name("sizemask");
	PRINT_VAL_U(tcp->u_arg[2]);
	if (flags_arg >= 0) {

		/* flags */
		tprints_arg_next_name("flags");
		printflags(sfd_flags, tcp->u_arg[flags_arg], "SFD_???");
	}

	return RVAL_DECODED | RVAL_FD;
}

SYS_FUNC(signalfd)
{
	return do_signalfd(tcp, -1);
}

SYS_FUNC(signalfd4)
{
	return do_signalfd(tcp, 3);
}
