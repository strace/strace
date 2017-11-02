/*
 * Copyright (c) 2015-2017 Dmitry V. Levin <ldv@altlinux.org>
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

SYS_FUNC(seccomp)
{
	unsigned int op = tcp->u_arg[0];
	unsigned int flags = tcp->u_arg[1];
	unsigned int act;

	printxval(seccomp_ops, op, "SECCOMP_SET_MODE_???");
	tprints(", ");

	switch (op) {
	case SECCOMP_GET_ACTION_AVAIL:
		tprintf("%u, ", flags);
		if (!umove_or_printaddr(tcp, tcp->u_arg[2], &act)) {
			tprints("[");
			printxval(seccomp_ret_action, act, "SECCOMP_RET_???");
			tprints("]");
		}
		break;

	case SECCOMP_SET_MODE_FILTER:
		printflags(seccomp_filter_flags, flags,
			   "SECCOMP_FILTER_FLAG_???");
		tprints(", ");
		decode_seccomp_fprog(tcp, tcp->u_arg[2]);
		break;

	case SECCOMP_SET_MODE_STRICT:
	default:
		tprintf("%u, ", flags);
		printaddr(tcp->u_arg[2]);
		break;
	}

	return RVAL_DECODED;
}
