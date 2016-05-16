/*
 * Copyright (c) 1994-1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-2000 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2005-2007 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2008-2015 Dmitry V. Levin <ldv@altlinux.org>
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

#include <sys/prctl.h>

#include "xlat/prctl_options.h"
#include "xlat/pr_cap_ambient.h"
#include "xlat/pr_mce_kill.h"
#include "xlat/pr_mce_kill_policy.h"
#include "xlat/pr_set_mm.h"
#include "xlat/pr_tsc.h"
#include "xlat/pr_unalign_flags.h"

#ifndef TASK_COMM_LEN
# define TASK_COMM_LEN 16
#endif

#ifdef HAVE_LINUX_SECCOMP_H
# include <linux/seccomp.h>
#endif
#include "xlat/seccomp_mode.h"

#ifdef HAVE_LINUX_SECUREBITS_H
# include <linux/securebits.h>
#endif
#include "xlat/secbits.h"

/* these constants are the same as in <linux/capability.h> */
enum {
#include "caps0.h"
#include "caps1.h"
};

#include "xlat/cap.h"

static void
print_prctl_args(struct tcb *tcp, const unsigned int first)
{
	unsigned int i;

	for (i = first; i < tcp->s_ent->nargs; ++i)
		tprintf(", %#lx", tcp->u_arg[i]);
}

SYS_FUNC(prctl)
{
	const unsigned int option = tcp->u_arg[0];
	unsigned int i;

	if (entering(tcp))
		printxval(prctl_options, option, "PR_???");

	switch (option) {
	case PR_GET_DUMPABLE:
	case PR_GET_KEEPCAPS:
	case PR_GET_SECCOMP:
	case PR_GET_TIMERSLACK:
	case PR_GET_TIMING:
		return RVAL_DECODED;

	case PR_GET_CHILD_SUBREAPER:
	case PR_GET_ENDIAN:
	case PR_GET_FPEMU:
	case PR_GET_FPEXC:
		if (entering(tcp))
			tprints(", ");
		else
			printnum_int(tcp, tcp->u_arg[1], "%u");
		break;

	case PR_GET_NAME:
		if (entering(tcp))
			tprints(", ");
		else {
			if (syserror(tcp))
				printaddr(tcp->u_arg[1]);
			else
				printstr(tcp, tcp->u_arg[1], -1);
		}
		break;

	case PR_GET_PDEATHSIG:
		if (entering(tcp))
			tprints(", ");
		else if (!umove_or_printaddr(tcp, tcp->u_arg[1], &i)) {
			tprints("[");
			tprints(signame(i));
			tprints("]");
		}
		break;

	case PR_GET_SECUREBITS:
		if (entering(tcp))
			break;
		if (syserror(tcp) || tcp->u_rval == 0)
			return 0;
		tcp->auxstr = sprintflags("", secbits,
					  (unsigned long) tcp->u_rval);
		return RVAL_STR;

	case PR_GET_TID_ADDRESS:
		if (entering(tcp))
			tprints(", ");
		else
			printnum_ptr(tcp, tcp->u_arg[1]);
		break;

	case PR_GET_TSC:
		if (entering(tcp))
			tprints(", ");
		else if (!umove_or_printaddr(tcp, tcp->u_arg[1], &i)) {
			tprints("[");
			printxval(pr_tsc, i, "PR_TSC_???");
			tprints("]");
		}
		break;

	case PR_GET_UNALIGN:
		if (entering(tcp))
			tprints(", ");
		else if (!umove_or_printaddr(tcp, tcp->u_arg[1], &i)) {
			tprints("[");
			printflags(pr_unalign_flags, i, "PR_UNALIGN_???");
			tprints("]");
		}
		break;

	/* PR_TASK_PERF_EVENTS_* take no arguments. */
	case PR_TASK_PERF_EVENTS_DISABLE:
	case PR_TASK_PERF_EVENTS_ENABLE:
		return RVAL_DECODED;

	case PR_SET_CHILD_SUBREAPER:
	case PR_SET_DUMPABLE:
	case PR_SET_ENDIAN:
	case PR_SET_FPEMU:
	case PR_SET_FPEXC:
	case PR_SET_KEEPCAPS:
	case PR_SET_TIMING:
		tprintf(", %lu", tcp->u_arg[1]);
		return RVAL_DECODED;

	case PR_CAPBSET_DROP:
	case PR_CAPBSET_READ:
		tprints(", ");
		printxval_long(cap, tcp->u_arg[1], "CAP_???");
		return RVAL_DECODED;

	case PR_CAP_AMBIENT:
		tprints(", ");
		printxval_long(pr_cap_ambient, tcp->u_arg[1],
			       "PR_CAP_AMBIENT_???");
		switch (tcp->u_arg[1]) {
		case PR_CAP_AMBIENT_RAISE:
		case PR_CAP_AMBIENT_LOWER:
		case PR_CAP_AMBIENT_IS_SET:
			tprints(", ");
			printxval_long(cap, tcp->u_arg[2], "CAP_???");
			print_prctl_args(tcp, 3);
			break;
		default:
			print_prctl_args(tcp, 2);
			break;
		}
		return RVAL_DECODED;

	case PR_MCE_KILL:
		tprints(", ");
		printxval_long(pr_mce_kill, tcp->u_arg[1], "PR_MCE_KILL_???");
		tprints(", ");
		if (PR_MCE_KILL_SET == tcp->u_arg[1])
			printxval_long(pr_mce_kill_policy, tcp->u_arg[2],
				   "PR_MCE_KILL_???");
		else
			tprintf("%#lx", tcp->u_arg[2]);
		print_prctl_args(tcp, 3);
		return RVAL_DECODED;

	case PR_SET_NAME:
		tprints(", ");
		printstr(tcp, tcp->u_arg[1], TASK_COMM_LEN);
		return RVAL_DECODED;

#ifdef __ANDROID__
# ifndef PR_SET_VMA_ANON_NAME
#  define PR_SET_VMA_ANON_NAME    0
# endif
	case PR_SET_VMA:
		if (tcp->u_arg[1] == PR_SET_VMA_ANON_NAME) {
			tprintf(", PR_SET_VMA_ANON_NAME, %#lx", tcp->u_arg[2]);
			tprintf(", %lu, ", tcp->u_arg[3]);
			printstr(tcp, tcp->u_arg[4], -1);
		} else {
			/* There are no other sub-options now, but there
			 * might be in future... */
			print_prctl_args(tcp, 1);
		}
		return RVAL_DECODED;
#endif

	case PR_SET_MM:
		tprints(", ");
		printxval(pr_set_mm, tcp->u_arg[1], "PR_SET_MM_???");
		print_prctl_args(tcp, 2);
		return RVAL_DECODED;

	case PR_SET_PDEATHSIG:
		tprints(", ");
		if ((unsigned long) tcp->u_arg[1] > 128)
			tprintf("%lu", tcp->u_arg[1]);
		else
			tprints(signame(tcp->u_arg[1]));
		return RVAL_DECODED;

	case PR_SET_PTRACER:
		tprints(", ");
		if (tcp->u_arg[1] == -1)
			tprints("PR_SET_PTRACER_ANY");
		else
			tprintf("%lu", tcp->u_arg[1]);
		return RVAL_DECODED;

	case PR_SET_SECCOMP:
		tprints(", ");
		printxval_long(seccomp_mode, tcp->u_arg[1],
			  "SECCOMP_MODE_???");
		if (SECCOMP_MODE_STRICT == tcp->u_arg[1])
			return RVAL_DECODED;
		if (SECCOMP_MODE_FILTER == tcp->u_arg[1]) {
			tprints(", ");
			print_seccomp_filter(tcp, tcp->u_arg[2]);
			return RVAL_DECODED;
		}
		print_prctl_args(tcp, 2);
		return RVAL_DECODED;

	case PR_SET_SECUREBITS:
		tprints(", ");
		printflags_long(secbits, tcp->u_arg[1], "SECBIT_???");
		return RVAL_DECODED;

	case PR_SET_TIMERSLACK:
		tprintf(", %ld", tcp->u_arg[1]);
		return RVAL_DECODED;

	case PR_SET_TSC:
		tprints(", ");
		printxval(pr_tsc, tcp->u_arg[1], "PR_TSC_???");
		return RVAL_DECODED;

	case PR_SET_UNALIGN:
		tprints(", ");
		printflags(pr_unalign_flags, tcp->u_arg[1], "PR_UNALIGN_???");
		return RVAL_DECODED;

	case PR_SET_NO_NEW_PRIVS:
	case PR_SET_THP_DISABLE:
		tprintf(", %lu", tcp->u_arg[1]);
		print_prctl_args(tcp, 2);
		return RVAL_DECODED;

	case PR_MCE_KILL_GET:
		if (entering(tcp)) {
			print_prctl_args(tcp, 1);
			return 0;
		}
		if (syserror(tcp))
			return 0;
		tcp->auxstr = xlookup(pr_mce_kill_policy,
				      (unsigned long) tcp->u_rval);
		return tcp->auxstr ? RVAL_STR : RVAL_UDECIMAL;

	case PR_GET_NO_NEW_PRIVS:
	case PR_GET_THP_DISABLE:
	case PR_MPX_DISABLE_MANAGEMENT:
	case PR_MPX_ENABLE_MANAGEMENT:
	default:
		print_prctl_args(tcp, 1);
		return RVAL_DECODED;
	}
	return 0;
}

#if defined X86_64 || defined X32
# include <asm/prctl.h>
# include "xlat/archvals.h"

SYS_FUNC(arch_prctl)
{
	const unsigned int option = tcp->u_arg[0];

	if (entering(tcp))
		printxval(archvals, option, "ARCH_???");

	switch (option) {
	case ARCH_GET_GS:
	case ARCH_GET_FS:
		if (entering(tcp))
			tprints(", ");
		else
			printnum_ptr(tcp, tcp->u_arg[1]);
		return 0;
	}

	tprintf(", %#lx", tcp->u_arg[1]);
	return RVAL_DECODED;
}
#endif /* X86_64 || X32 */
