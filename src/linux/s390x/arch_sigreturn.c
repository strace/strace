/*
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdint.h>

#define PTR_TYPE		uint32_t
#define arch_sigreturn	s390_arch_sigreturn
#include "../s390/arch_sigreturn.c"
#undef arch_sigreturn
#undef PTR_TYPE
#undef SIGNAL_FRAMESIZE

#define SIGNAL_FRAMESIZE	160
#define arch_sigreturn	s390x_arch_sigreturn
#include "../s390/arch_sigreturn.c"
#undef arch_sigreturn

static void
arch_sigreturn(struct tcb *tcp)
{
	if (tcp->currpers == 1)
		s390_arch_sigreturn(tcp);
	else
		s390x_arch_sigreturn(tcp);
}
