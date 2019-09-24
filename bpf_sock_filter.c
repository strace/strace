/*
 * Decoder of socket filter programs.
 *
 * Copyright (c) 2017 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2017-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include "bpf_filter.h"

#include <linux/filter.h>
#include "xlat/skf_ad.h"
#define XLAT_MACROS_ONLY
# include "xlat/skf_off.h"
#undef XLAT_MACROS_ONLY

static bool
print_sock_filter_k(const struct bpf_filter_block *const fp)
{
	if (BPF_CLASS(fp->code) == BPF_LD && BPF_MODE(fp->code) == BPF_ABS) {
		if (fp->k >= (unsigned int) SKF_AD_OFF) {
			print_xlat32(SKF_AD_OFF);
			tprints("+");
			printxval(skf_ad, fp->k - (unsigned int) SKF_AD_OFF,
				  "SKF_AD_???");
			return true;
		} else if (fp->k >= (unsigned int) SKF_NET_OFF) {
			print_xlat32(SKF_NET_OFF);
			tprintf("+%u", fp->k - (unsigned int) SKF_NET_OFF);
			return true;
		} else if (fp->k >= (unsigned int) SKF_LL_OFF) {
			print_xlat32(SKF_LL_OFF);
			tprintf("+%u", fp->k - (unsigned int) SKF_LL_OFF);
			return true;
		}
	}

	return false;
}

void
print_sock_fprog(struct tcb *const tcp, const kernel_ulong_t addr,
		    const unsigned short len)
{
	print_bpf_fprog(tcp, addr, len, print_sock_filter_k);
}

void
decode_sock_fprog(struct tcb *const tcp, const kernel_ulong_t addr)
{
	decode_bpf_fprog(tcp, addr, print_sock_filter_k);
}
