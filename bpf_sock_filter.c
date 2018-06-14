/*
 * Decoder of socket filter programs.
 *
 * Copyright (c) 2017 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "defs.h"

#include "bpf_filter.h"

#include <linux/filter.h>
#include "xlat/skf_ad.h"
#include "xlat/skf_off.h"

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
