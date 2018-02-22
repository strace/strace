/*
 * Decoder of classic BPF programs.
 *
 * Copyright (c) 2015-2017 Dmitry V. Levin <ldv@altlinux.org>
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
#include "bpf_fprog.h"

#include <linux/filter.h>

#include "xlat/bpf_class.h"
#include "xlat/bpf_miscop.h"
#include "xlat/bpf_mode.h"
#include "xlat/bpf_op_alu.h"
#include "xlat/bpf_op_jmp.h"
#include "xlat/bpf_rval.h"
#include "xlat/bpf_size.h"
#include "xlat/bpf_src.h"

#include "xlat/ebpf_class.h"
#include "xlat/ebpf_mode.h"
#include "xlat/ebpf_op_alu.h"
#include "xlat/ebpf_op_jmp.h"
#include "xlat/ebpf_size.h"

void
print_bpf_filter_code(const uint16_t code, bool extended)
{
	const struct xlat *class = extended ? ebpf_class : bpf_class;
	const struct xlat *mode = extended ? ebpf_mode : bpf_mode;

	uint16_t i = code & ~BPF_CLASS(code);

	printxval(class, BPF_CLASS(code), "BPF_???");
	switch (BPF_CLASS(code)) {
	case BPF_ST:
	case BPF_STX:
		if (!extended) {
			if (i) {
				tprintf("|%#x", i);
				tprints_comment("BPF_???");
			}
			break;
		}
		ATTRIBUTE_FALLTHROUGH; /* extended == true */

	case BPF_LD:
	case BPF_LDX:
		tprints("|");
		printxvals(BPF_SIZE(code), "BPF_???",
			   bpf_size, extended ? ebpf_size : NULL, NULL);
		tprints("|");
		printxval(mode, BPF_MODE(code), "BPF_???");
		break;

	case BPF_MISC: /* BPF_ALU64 in eBPF */
		if (!extended) {
			tprints("|");
			printxval(bpf_miscop, BPF_MISCOP(code), "BPF_???");
			i &= ~BPF_MISCOP(code);
			if (i) {
				tprintf("|%#x", i);
				tprints_comment("BPF_???");
			}
			break;
		}
		ATTRIBUTE_FALLTHROUGH; /* extended == true */

	case BPF_ALU:
		tprints("|");
		printxval(bpf_src, BPF_SRC(code), "BPF_???");
		tprints("|");
		printxvals(BPF_OP(code), "BPF_???",
			   bpf_op_alu,
			   extended ? ebpf_op_alu : NULL, NULL);
		break;

	case BPF_JMP:
		tprints("|");
		printxval(bpf_src, BPF_SRC(code), "BPF_???");
		tprints("|");
		printxvals(BPF_OP(code), "BPF_???",
			   bpf_op_jmp, extended ? ebpf_op_jmp : NULL, NULL);
		break;

	case BPF_RET: /* Reserved in eBPF */
		if (!extended) {
			tprints("|");
			printxval(bpf_rval, BPF_RVAL(code), "BPF_???");
			i &= ~BPF_RVAL(code);
		}

		if (i) {
			tprintf("|%#x", i);
			tprints_comment("BPF_???");
		}

		break;
	}
}

static void
print_bpf_filter_stmt(const struct bpf_filter_block *const filter,
		      const print_bpf_filter_fn print_k)
{
	tprints("BPF_STMT(");
	print_bpf_filter_code(filter->code, false);
	tprints(", ");
	if (!print_k || !print_k(filter))
		tprintf("%#x", filter->k);
	tprints(")");
}

static void
print_bpf_filter_jump(const struct bpf_filter_block *const filter)
{
	tprints("BPF_JUMP(");
	print_bpf_filter_code(filter->code, false);
	tprintf(", %#x, %#x, %#x)", filter->k, filter->jt, filter->jf);
}

struct bpf_filter_block_data {
	const print_bpf_filter_fn fn;
	unsigned int count;
};

static bool
print_bpf_filter_block(struct tcb *const tcp, void *const elem_buf,
		       const size_t elem_size, void *const data)
{
	const struct bpf_filter_block *const filter = elem_buf;
	struct bpf_filter_block_data *const fbd = data;

	if (fbd->count++ >= BPF_MAXINSNS) {
		tprints("...");
		return false;
	}

	if (filter->jt || filter->jf)
		print_bpf_filter_jump(filter);
	else
		print_bpf_filter_stmt(filter, fbd->fn);

	return true;
}

void
print_bpf_fprog(struct tcb *const tcp, const kernel_ulong_t addr,
		const unsigned short len, const print_bpf_filter_fn print_k)
{
	if (abbrev(tcp)) {
		printaddr(addr);
	} else {
		struct bpf_filter_block_data fbd = { .fn = print_k };
		struct bpf_filter_block filter;

		print_array(tcp, addr, len, &filter, sizeof(filter),
			    umoven_or_printaddr, print_bpf_filter_block, &fbd);
	}
}

void
decode_bpf_fprog(struct tcb *const tcp, const kernel_ulong_t addr,
		 const print_bpf_filter_fn print_k)
{
	struct bpf_fprog fprog;

	if (fetch_bpf_fprog(tcp, addr, &fprog)) {
		tprintf("{len=%hu, filter=", fprog.len);
		print_bpf_fprog(tcp, fprog.filter, fprog.len, print_k);
		tprints("}");
	}
}
