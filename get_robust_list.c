/*
 * Copyright (c) 2012-2015 Dmitry V. Levin <ldv@altlinux.org>
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

#include DEF_MPERS_TYPE(struct_robust_list_head)

#include <linux/futex.h>

typedef struct robust_list_head struct_robust_list_head;

#include MPERS_DEFS

#include "print_fields.h"

static void
decode_robust_list(struct tcb *tcp, kernel_ulong_t addr, kernel_ulong_t len)
{
	struct_robust_list_head rl;

	if (len < sizeof(rl)) {
		printaddr(addr);
		return;
	}
	if (umove_or_printaddr(tcp, addr, &rl))
		return;

	PRINT_FIELD_PTR("{list={", rl.list, next);
	PRINT_FIELD_D("}, ", rl, futex_offset);
	PRINT_FIELD_PTR(", ", rl, list_op_pending);

	if (len > sizeof(rl))
		tprints(", /* ??? */");

	tprints("}");

	printaddr_comment(addr);
}

SYS_FUNC(set_robust_list)
{
	decode_robust_list(tcp, tcp->u_arg[0], tcp->u_arg[1]);
	tprintf(", %lu", (unsigned long) tcp->u_arg[1]);

	return RVAL_DECODED;
}

SYS_FUNC(get_robust_list)
{
	if (entering(tcp)) {
		tprintf("%d, ", (int) tcp->u_arg[0]);
	} else {
		mpers_ptr_t rl_ptr;
		mpers_ptr_t len = 0;
		if (!umove_or_printaddr(tcp, tcp->u_arg[1], &rl_ptr)) {
			tprints("[");
			if (!umove(tcp, tcp->u_arg[2], &len))
				decode_robust_list(tcp, rl_ptr, len);
			else
				printaddr(tcp->u_arg[1]);
			tprints("]");
		}
		tprints(", ");
		printnum_ulong(tcp, tcp->u_arg[2]);
	}
	return 0;
}
