/*
 * Copyright (c) 2017-2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include DEF_MPERS_TYPE(struct_rt_sigframe)

#include "rt_sigframe.h"

#include MPERS_DEFS

#ifndef OFFSETOF_SIGMASK_IN_RT_SIGFRAME
# define OFFSETOF_SIGMASK_IN_RT_SIGFRAME	\
		offsetof(struct_rt_sigframe, uc.uc_sigmask)
#endif

SYS_FUNC(rt_sigreturn)
{
	const kernel_ulong_t sf_addr = get_rt_sigframe_addr(tcp);

	if (sf_addr) {
		const kernel_ulong_t sm_addr =
			sf_addr + OFFSETOF_SIGMASK_IN_RT_SIGFRAME;
		tprints("{mask=");
		print_sigset_addr(tcp, sm_addr);
		tprints("}");
	}

	return RVAL_DECODED;
}
