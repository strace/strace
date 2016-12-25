/*
 * Copyright (c) 2013 Christian Svensson <blue@cmd.nu>
 * Copyright (c) 2014-2015 Dmitry V. Levin <ldv@altlinux.org>
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

#ifdef OR1K

#define OR1K_ATOMIC_SWAP        1
#define OR1K_ATOMIC_CMPXCHG     2
#define OR1K_ATOMIC_XCHG        3
#define OR1K_ATOMIC_ADD         4
#define OR1K_ATOMIC_DECPOS      5
#define OR1K_ATOMIC_AND         6
#define OR1K_ATOMIC_OR          7
#define OR1K_ATOMIC_UMAX        8
#define OR1K_ATOMIC_UMIN        9

#include "xlat/atomic_ops.h"

SYS_FUNC(or1k_atomic)
{
	printxval64(atomic_ops, tcp->u_arg[0], "???");
	switch(tcp->u_arg[0]) {
	case OR1K_ATOMIC_SWAP:
		tprintf(", 0x%lx, 0x%lx", tcp->u_arg[1], tcp->u_arg[2]);
		break;
	case OR1K_ATOMIC_CMPXCHG:
		tprintf(", 0x%lx, %#lx, %#lx", tcp->u_arg[1], tcp->u_arg[2],
			tcp->u_arg[3]);
		break;

	case OR1K_ATOMIC_XCHG:
	case OR1K_ATOMIC_ADD:
	case OR1K_ATOMIC_AND:
	case OR1K_ATOMIC_OR:
	case OR1K_ATOMIC_UMAX:
	case OR1K_ATOMIC_UMIN:
		tprintf(", 0x%lx, %#lx", tcp->u_arg[1], tcp->u_arg[2]);
		break;

	case OR1K_ATOMIC_DECPOS:
		tprintf(", 0x%lx", tcp->u_arg[1]);
		break;

	default:
		break;
	}

	return RVAL_DECODED | RVAL_HEX;
}

#endif /* OR1K */
