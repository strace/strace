/*
 * Copyright (c) 1999-2000 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2002-2005 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2008 Jan Kratochvil <jan.kratochvil@redhat.com>
 * Copyright (c) 2009-2013 Denys Vlasenko <dvlasenk@redhat.com>
 * Copyright (c) 2006-2015 Dmitry V. Levin <ldv@altlinux.org>
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
#include <sched.h>
#include <asm/unistd.h>

#ifndef CSIGNAL
# define CSIGNAL 0x000000ff
#endif

#include "xlat/clone_flags.h"
#include "xlat/setns_types.h"
#include "xlat/unshare_flags.h"

#if defined IA64
# define ARG_FLAGS	0
# define ARG_STACK	1
# define ARG_STACKSIZE	(tcp->scno == __NR_clone2 ? 2 : -1)
# define ARG_PTID	(tcp->scno == __NR_clone2 ? 3 : 2)
# define ARG_CTID	(tcp->scno == __NR_clone2 ? 4 : 3)
# define ARG_TLS	(tcp->scno == __NR_clone2 ? 5 : 4)
#elif defined S390 || defined S390X || defined CRISV10 || defined CRISV32
# define ARG_STACK	0
# define ARG_FLAGS	1
# define ARG_PTID	2
# define ARG_CTID	3
# define ARG_TLS	4
#elif defined X86_64 || defined X32
/* x86 personality processes have the last two arguments flipped. */
# define ARG_FLAGS	0
# define ARG_STACK	1
# define ARG_PTID	2
# define ARG_CTID	((current_personality != 1) ? 3 : 4)
# define ARG_TLS	((current_personality != 1) ? 4 : 3)
#elif defined ALPHA || defined TILE || defined OR1K || defined RISCV
# define ARG_FLAGS	0
# define ARG_STACK	1
# define ARG_PTID	2
# define ARG_CTID	3
# define ARG_TLS	4
#else
# define ARG_FLAGS	0
# define ARG_STACK	1
# define ARG_PTID	2
# define ARG_TLS	3
# define ARG_CTID	4
#endif

static void
print_tls_arg(struct tcb *const tcp, const kernel_ulong_t addr)
{
#ifdef HAVE_STRUCT_USER_DESC
# if SUPPORTED_PERSONALITIES > 1
	if (current_personality == 1)
# endif
	{
		print_user_desc(tcp, tcp->u_arg[ARG_TLS]);
	}
# if SUPPORTED_PERSONALITIES > 1
	else
# endif
#endif /* HAVE_STRUCT_USER_DESC */
	{
		printaddr(tcp->u_arg[ARG_TLS]);
	}
}

SYS_FUNC(clone)
{
	if (exiting(tcp)) {
		const char *sep = "|";
		kernel_ulong_t flags = tcp->u_arg[ARG_FLAGS];
		tprints("child_stack=");
		printaddr(tcp->u_arg[ARG_STACK]);
		tprints(", ");
#ifdef ARG_STACKSIZE
		if (ARG_STACKSIZE != -1)
			tprintf("stack_size=%#" PRI_klx ", ",
				tcp->u_arg[ARG_STACKSIZE]);
#endif
		tprints("flags=");
		if (!printflags64(clone_flags, flags &~ CSIGNAL, NULL))
			sep = "";
		if ((flags & CSIGNAL) != 0)
			tprintf("%s%s", sep, signame(flags & CSIGNAL));
		if ((flags & (CLONE_PARENT_SETTID|CLONE_CHILD_SETTID
			      |CLONE_CHILD_CLEARTID|CLONE_SETTLS)) == 0)
			return 0;
		if (flags & CLONE_PARENT_SETTID) {
			tprints(", parent_tidptr=");
			printaddr(tcp->u_arg[ARG_PTID]);
		}
		if (flags & CLONE_SETTLS) {
			tprints(", tls=");
			print_tls_arg(tcp, tcp->u_arg[ARG_TLS]);
		}
		if (flags & (CLONE_CHILD_SETTID|CLONE_CHILD_CLEARTID)) {
			tprints(", child_tidptr=");
			printaddr(tcp->u_arg[ARG_CTID]);
		}
	}
	/* TODO on syscall entry:
	 * We can clear CLONE_PTRACE here since it is an ancient hack
	 * to allow us to catch children, and we use another hack for that.
	 * But CLONE_PTRACE can conceivably be used by malicious programs
	 * to subvert us. By clearing this bit, we can defend against it:
	 * in untraced execution, CLONE_PTRACE should have no effect.
	 *
	 * We can also clear CLONE_UNTRACED, since it allows to start
	 * children outside of our control. At the moment
	 * I'm trying to figure out whether there is a *legitimate*
	 * use of this flag which we should respect.
	 */
	return 0;
}

SYS_FUNC(setns)
{
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	printxval(setns_types, tcp->u_arg[1], "CLONE_NEW???");

	return RVAL_DECODED;
}

SYS_FUNC(unshare)
{
	printflags64(unshare_flags, tcp->u_arg[0], "CLONE_???");
	return RVAL_DECODED;
}

SYS_FUNC(fork)
{
	return RVAL_DECODED | RVAL_UDECIMAL;
}
