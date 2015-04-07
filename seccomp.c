/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
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

#ifdef HAVE_LINUX_SECCOMP_H
# include <linux/seccomp.h>
#endif
#include "xlat/seccomp_ops.h"
#include "xlat/seccomp_filter_flags.h"

#ifdef HAVE_LINUX_FILTER_H
# include <linux/filter.h>
# include "xlat/bpf_class.h"
# include "xlat/bpf_miscop.h"
# include "xlat/bpf_mode.h"
# include "xlat/bpf_op_alu.h"
# include "xlat/bpf_op_jmp.h"
# include "xlat/bpf_rval.h"
# include "xlat/bpf_size.h"
# include "xlat/bpf_src.h"

# ifndef SECCOMP_RET_ACTION
#  define SECCOMP_RET_ACTION 0x7fff0000U
# endif
# include "xlat/seccomp_ret_action.h"
#endif

struct bpf_filter {
	uint16_t code;
	uint8_t jt;
	uint8_t jf;
	uint32_t k;
};

#ifdef HAVE_LINUX_FILTER_H

static void
decode_bpf_code(uint16_t code)
{
	uint16_t i = code & ~BPF_CLASS(code);

	printxval(bpf_class, BPF_CLASS(code), "BPF_???");
	switch (BPF_CLASS(code)) {
		case BPF_LD:
		case BPF_LDX:
			tprints(" | ");
			printxval(bpf_size, BPF_SIZE(code), "BPF_???");
			tprints(" | ");
			printxval(bpf_mode, BPF_MODE(code), "BPF_???");
			break;
		case BPF_ST:
		case BPF_STX:
			if (i)
				tprintf(" | %#x /* %s */", i, "BPF_???");
			break;
		case BPF_ALU:
			tprints(" | ");
			printxval(bpf_src, BPF_SRC(code), "BPF_???");
			tprints(" | ");
			printxval(bpf_op_alu, BPF_OP(code), "BPF_???");
			break;
		case BPF_JMP:
			tprints(" | ");
			printxval(bpf_src, BPF_SRC(code), "BPF_???");
			tprints(" | ");
			printxval(bpf_op_jmp, BPF_OP(code), "BPF_???");
			break;
		case BPF_RET:
			tprints(" | ");
			printxval(bpf_rval, BPF_RVAL(code), "BPF_???");
			i &= ~BPF_RVAL(code);
			if (i)
				tprintf(" | %#x /* %s */", i, "BPF_???");
			break;
		case BPF_MISC:
			tprints(" | ");
			printxval(bpf_miscop, BPF_MISCOP(code), "BPF_???");
			i &= ~BPF_MISCOP(code);
			if (i)
				tprintf(" | %#x /* %s */", i, "BPF_???");
			break;
	}

}

static void
decode_bpf_stmt(const struct bpf_filter *filter)
{
#ifdef HAVE_LINUX_FILTER_H
	tprints("BPF_STMT(");
	decode_bpf_code(filter->code);
	tprints(", ");
	if (BPF_CLASS(filter->code) == BPF_RET) {
		unsigned int action = SECCOMP_RET_ACTION & filter->k;
		unsigned int data = filter->k & ~action;

		printxval(seccomp_ret_action, action, "SECCOMP_RET_???");
		if (data)
			tprintf(" | %#x)", data);
		else
			tprints(")");
	} else {
		tprintf("%#x)", filter->k);
	}
#else
	tprintf("BPF_STMT(%#x, %#x)", filter->code, filter->k);
#endif /* HAVE_LINUX_FILTER_H */
}

static void
decode_bpf_jump(const struct bpf_filter *filter)
{
#ifdef HAVE_LINUX_FILTER_H
	tprints("BPF_JUMP(");
	decode_bpf_code(filter->code);
	tprintf(", %#x, %#x, %#x)",
		filter->k, filter->jt, filter->jf);
#else
	tprintf("BPF_JUMP(%#x, %#x, %#x, %#x)",
		filter->code, filter->k, filter->jt, filter->jf);
#endif /* HAVE_LINUX_FILTER_H */
}

static void
decode_filter(const struct bpf_filter *filter)
{
	if (filter->jt || filter->jf)
		decode_bpf_jump(filter);
	else
		decode_bpf_stmt(filter);
}

#endif /* HAVE_LINUX_FILTER_H */

#ifndef BPF_MAXINSNS
# define BPF_MAXINSNS 4096
#endif

static void
decode_fprog(struct tcb *tcp, unsigned short len, unsigned long addr)
{
	if (!len || abbrev(tcp)) {
		tprintf("{len = %u, filter = %#lx}", len, addr);
	} else {
		unsigned int i = 0;

		tprints("[");
		while (i < len && i < BPF_MAXINSNS) {
			struct bpf_filter filter;

			if (umove(tcp, addr, &filter) < 0)
				break;
			if (i)
				tprints(", ");
			decode_filter(&filter);

			addr += sizeof(filter);
			++i;
		}
		if (i < len)
			tprints("...");
		tprints("]");
	}
}

void
print_seccomp_filter(struct tcb *tcp, unsigned long addr)
{
	if (addr) {
#if SUPPORTED_PERSONALITIES > 1 && SIZEOF_LONG > 4
		if (current_wordsize == 4) {
			struct {
				unsigned short len;
				uint32_t filter;
			} fprog;
			if (umove(tcp, addr, &fprog) < 0)
				tprintf("%#lx", addr);
			else
				decode_fprog(tcp, fprog.len, fprog.filter);
		} else {
#endif
			struct {
				unsigned short len;
				unsigned long filter;
			} fprog;
			if (umove(tcp, addr, &fprog) < 0)
				tprintf("%#lx", addr);
			else
				decode_fprog(tcp, fprog.len, fprog.filter);
#if SUPPORTED_PERSONALITIES > 1 && SIZEOF_LONG > 4
		}
#endif
	} else {
		tprints("NULL");
	}
}

static void
decode_seccomp_set_mode_strict(unsigned int flags, unsigned long addr)
{
	tprintf("%u, ", flags);
	if (addr)
		tprintf("%#lx", addr);
	else
		tprints("NULL");
}

SYS_FUNC(seccomp)
{
	if (entering(tcp)) {
		unsigned int op = tcp->u_arg[0];

		printxval(seccomp_ops, op, "SECCOMP_SET_MODE_???");
		tprints(", ");

		if (op == SECCOMP_SET_MODE_FILTER) {
			printflags(seccomp_filter_flags, tcp->u_arg[1],
				   "SECCOMP_FILTER_FLAG_???");
			tprints(", ");
			print_seccomp_filter(tcp, tcp->u_arg[2]);
		} else {
			decode_seccomp_set_mode_strict(tcp->u_arg[1],
						       tcp->u_arg[2]);
		}
	}
	return 0;
}
