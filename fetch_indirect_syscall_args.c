/*
 * Copyright (c) 2018 The strace developers.
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

/*
 * Fetch indirect syscall arguments that are provided as an array.
 * Return a pointer to a static array of kernel_ulong_t elements,
 * or NULL in case of fetch failure.
 */
kernel_ulong_t *
fetch_indirect_syscall_args(struct tcb *const tcp,
			    const kernel_ulong_t addr,
			    const unsigned int n_args)
{
	static kernel_ulong_t u_arg[MAX_ARGS];

	if (current_wordsize == sizeof(*u_arg)) {
		if (umoven(tcp, addr, sizeof(*u_arg) * n_args, u_arg))
			return NULL;
	} else {
		uint32_t narrow_arg[ARRAY_SIZE(u_arg)];

		if (umoven(tcp, addr, sizeof(*narrow_arg) * n_args, narrow_arg))
			return NULL;
		for (unsigned int i = 0; i < n_args; ++i)
			u_arg[i] = narrow_arg[i];
	}

	return u_arg;
}
