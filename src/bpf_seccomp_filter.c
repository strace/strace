/*
 * Decoder of seccomp filter programs.
 *
 * Copyright (c) 2015-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include "bpf_filter.h"

#include <linux/filter.h>
#include <linux/seccomp.h>
#include "xlat/seccomp_ret_action.h"

static bool
print_seccomp_filter_k(const struct bpf_filter_block *const fp)
{
	if (BPF_CLASS(fp->code) == BPF_RET) {
		unsigned int action = SECCOMP_RET_ACTION_FULL & fp->k;
		unsigned int data = fp->k & ~action;

		printxval(seccomp_ret_action, action, "SECCOMP_RET_???");
		if (data) {
			tprints("|");
			PRINT_VAL_X(data);
		}

		return true;
	}

	return false;
}

void
print_seccomp_fprog(struct tcb *const tcp, const kernel_ulong_t addr,
		    const unsigned short len)
{
	print_bpf_fprog(tcp, addr, len, print_seccomp_filter_k);
}

void
decode_seccomp_fprog(struct tcb *const tcp, const kernel_ulong_t addr)
{
	decode_bpf_fprog(tcp, addr, print_seccomp_filter_k);
}
