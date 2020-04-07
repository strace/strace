/*
 * Copyright (c) 1994-1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-2000 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2005-2007 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2008-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2014-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <linux/prctl.h>

#include "xstring.h"

#include "xlat/prctl_options.h"
#include "xlat/pr_cap_ambient.h"
#include "xlat/pr_dumpable.h"
#include "xlat/pr_fp_mode.h"
#include "xlat/pr_mce_kill.h"
#include "xlat/pr_mce_kill_policy.h"
#include "xlat/pr_set_mm.h"
#include "xlat/pr_spec_cmds.h"
#include "xlat/pr_spec_get_store_bypass_flags.h"
#include "xlat/pr_spec_set_store_bypass_flags.h"
#include "xlat/pr_sve_vl_flags.h"
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

#ifndef PR_SVE_VL_LEN_MASK
# define PR_SVE_VL_LEN_MASK 0xffff
#endif


static void
print_prctl_args(struct tcb *tcp, const unsigned int first)
{
	unsigned int i;

	for (i = first; i < n_args(tcp); ++i)
		tprintf(", %#" PRI_klx, tcp->u_arg[i]);
}

static char *
sprint_sve_val(kernel_ulong_t arg)
{
	static char out[sizeof("PR_SVE_SET_VL_ONEXEC|PR_SVE_VL_INHERIT|0x") +
			sizeof(kernel_ulong_t) * 2];

	kernel_ulong_t vl = arg & PR_SVE_VL_LEN_MASK;
	kernel_ulong_t flags = arg & ~PR_SVE_VL_LEN_MASK;
	const char *flags_str = sprintflags("", pr_sve_vl_flags, flags);

	xsprintf(out, "%s%s%#" PRI_klx,
		 flags_str ?: "", flags_str ? "|" : "", vl);

	return out;
}

SYS_FUNC(prctl)
{
	const unsigned int option = tcp->u_arg[0];
	const kernel_ulong_t arg2 = tcp->u_arg[1];
	const kernel_ulong_t arg3 = tcp->u_arg[2];
	/*
	 * PR_SET_VMA is the only command which actually uses these arguments
	 * currently, and it is available only on Android for now.
	 */
#ifdef __ANDROID__
	const kernel_ulong_t arg4 = tcp->u_arg[3];
	const kernel_ulong_t arg5 = tcp->u_arg[4];
#endif
	unsigned int i;

	if (entering(tcp))
		printxval(prctl_options, option, "PR_???");

	switch (option) {
	case PR_GET_KEEPCAPS:
	case PR_GET_SECCOMP:
	case PR_GET_TIMERSLACK:
	case PR_GET_TIMING:
	case PR_GET_TAGGED_ADDR_CTRL:
		return RVAL_DECODED;

	case PR_GET_CHILD_SUBREAPER:
	case PR_GET_ENDIAN:
	case PR_GET_FPEMU:
	case PR_GET_FPEXC:
		if (entering(tcp))
			tprints(", ");
		else
			printnum_int(tcp, arg2, "%u");
		break;

	case PR_GET_DUMPABLE:
		if (entering(tcp))
			break;
		if (syserror(tcp))
			return 0;
		tcp->auxstr = xlookup(pr_dumpable, (kernel_ulong_t) tcp->u_rval);
		return RVAL_STR;

	case PR_GET_NAME:
		if (entering(tcp)) {
			tprints(", ");
		} else {
			if (syserror(tcp))
				printaddr(arg2);
			else
				printstr_ex(tcp, arg2, TASK_COMM_LEN,
					    QUOTE_0_TERMINATED);
		}
		break;

	case PR_GET_PDEATHSIG:
		if (entering(tcp)) {
			tprints(", ");
		} else if (!umove_or_printaddr(tcp, arg2, &i)) {
			tprints("[");
			printsignal(i);
			tprints("]");
		}
		break;

	case PR_GET_SECUREBITS:
		if (entering(tcp))
			break;
		if (syserror(tcp) || tcp->u_rval == 0)
			return 0;
		tcp->auxstr = sprintflags("", secbits,
					  (kernel_ulong_t) tcp->u_rval);
		return RVAL_STR;

	case PR_GET_TID_ADDRESS:
		if (entering(tcp))
			tprints(", ");
		else
			printnum_kptr(tcp, arg2);
		break;

	case PR_GET_TSC:
		if (entering(tcp)) {
			tprints(", ");
		} else if (!umove_or_printaddr(tcp, arg2, &i)) {
			tprints("[");
			printxval(pr_tsc, i, "PR_TSC_???");
			tprints("]");
		}
		break;

	case PR_GET_UNALIGN:
		if (entering(tcp)) {
			tprints(", ");
		} else if (!umove_or_printaddr(tcp, arg2, &i)) {
			tprints("[");
			printflags(pr_unalign_flags, i, "PR_UNALIGN_???");
			tprints("]");
		}
		break;

	case PR_GET_FP_MODE:
		if (entering(tcp))
			break;
		if (syserror(tcp) || tcp->u_rval == 0)
			return 0;
		tcp->auxstr = sprintflags("", pr_fp_mode,
					  (kernel_ulong_t) tcp->u_rval);
		return RVAL_STR;

	case PR_SVE_SET_VL:
		if (entering(tcp)) {
			tprintf(", %s", sprint_sve_val(arg2));
			return 0;
		}
		ATTRIBUTE_FALLTHROUGH;

	case PR_SVE_GET_VL:
		if (entering(tcp))
			break;
		if (syserror(tcp) || tcp->u_rval == 0)
			return 0;

		tcp->auxstr = sprint_sve_val(tcp->u_rval);

		return RVAL_STR;

	case PR_GET_SPECULATION_CTRL:
		if (entering(tcp)) {
			tprints(", ");
			printxval64(pr_spec_cmds, arg2, "PR_SPEC_???");

			break;
		}

		if (syserror(tcp))
			return 0;

		switch (arg2) {
		case PR_SPEC_STORE_BYPASS:
		case PR_SPEC_INDIRECT_BRANCH:
			tcp->auxstr = sprintflags("",
						  pr_spec_get_store_bypass_flags,
						  (kernel_ulong_t) tcp->u_rval);
			break;
		}

		return RVAL_STR;

	/* PR_TASK_PERF_EVENTS_* take no arguments. */
	case PR_TASK_PERF_EVENTS_DISABLE:
	case PR_TASK_PERF_EVENTS_ENABLE:
		return RVAL_DECODED;

	case PR_SET_CHILD_SUBREAPER:
	case PR_SET_ENDIAN:
	case PR_SET_FPEMU:
	case PR_SET_FPEXC:
	case PR_SET_KEEPCAPS:
	case PR_SET_TIMING:
	case PR_SET_TAGGED_ADDR_CTRL:
		tprintf(", %" PRI_klu, arg2);
		return RVAL_DECODED;

	case PR_SET_DUMPABLE:
		tprints(", ");
		printxval64(pr_dumpable, arg2, "SUID_DUMP_???");
		return RVAL_DECODED;

	case PR_CAPBSET_DROP:
	case PR_CAPBSET_READ:
		tprints(", ");
		printxval64(cap, arg2, "CAP_???");
		return RVAL_DECODED;

	case PR_CAP_AMBIENT:
		tprints(", ");
		printxval64(pr_cap_ambient, arg2,
			       "PR_CAP_AMBIENT_???");
		switch (arg2) {
		case PR_CAP_AMBIENT_RAISE:
		case PR_CAP_AMBIENT_LOWER:
		case PR_CAP_AMBIENT_IS_SET:
			tprints(", ");
			printxval64(cap, arg3, "CAP_???");
			print_prctl_args(tcp, 3);
			break;
		default:
			print_prctl_args(tcp, 2);
			break;
		}
		return RVAL_DECODED;

	case PR_MCE_KILL:
		tprints(", ");
		printxval64(pr_mce_kill, arg2, "PR_MCE_KILL_???");
		tprints(", ");
		if (PR_MCE_KILL_SET == arg2)
			printxval64(pr_mce_kill_policy, arg3,
				    "PR_MCE_KILL_???");
		else
			tprintf("%#" PRI_klx, arg3);
		print_prctl_args(tcp, 3);
		return RVAL_DECODED;

	case PR_SET_NAME:
		tprints(", ");
		printstr_ex(tcp, arg2, TASK_COMM_LEN - 1,
			    QUOTE_0_TERMINATED);
		return RVAL_DECODED;

#ifdef __ANDROID__
# ifndef PR_SET_VMA_ANON_NAME
#  define PR_SET_VMA_ANON_NAME    0
# endif
	case PR_SET_VMA:
		if (arg2 == PR_SET_VMA_ANON_NAME) {
			tprintf(", PR_SET_VMA_ANON_NAME, %#" PRI_klx, arg3);
			tprintf(", %" PRI_klu ", ", arg4);
			printstr(tcp, arg5);
		} else {
			/* There are no other sub-options now, but there
			 * might be in future... */
			print_prctl_args(tcp, 1);
		}
		return RVAL_DECODED;
#endif

	case PR_SET_MM:
		tprints(", ");
		printxval(pr_set_mm, arg2, "PR_SET_MM_???");
		print_prctl_args(tcp, 2);
		return RVAL_DECODED;

	case PR_SET_PDEATHSIG:
		tprints(", ");
		if (arg2 > 128)
			tprintf("%" PRI_klu, arg2);
		else
			printsignal(arg2);
		return RVAL_DECODED;

	case PR_SET_PTRACER:
		tprints(", ");
		if ((int) arg2 == -1) {
			print_xlat_ex(arg2, "PR_SET_PTRACER_ANY",
				      XLAT_STYLE_DEFAULT);
		} else {
			tprintf("%" PRI_klu, arg2);
		}
		return RVAL_DECODED;

	case PR_SET_SECCOMP:
		tprints(", ");
		printxval64(seccomp_mode, arg2,
			    "SECCOMP_MODE_???");
		if (SECCOMP_MODE_STRICT == arg2)
			return RVAL_DECODED;
		if (SECCOMP_MODE_FILTER == arg2) {
			tprints(", ");
			decode_seccomp_fprog(tcp, arg3);
			return RVAL_DECODED;
		}
		print_prctl_args(tcp, 2);
		return RVAL_DECODED;

	case PR_SET_SECUREBITS:
		tprints(", ");
		printflags64(secbits, arg2, "SECBIT_???");
		return RVAL_DECODED;

	case PR_SET_TIMERSLACK:
		tprintf(", %" PRI_kld, arg2);
		return RVAL_DECODED;

	case PR_SET_TSC:
		tprints(", ");
		printxval(pr_tsc, arg2, "PR_TSC_???");
		return RVAL_DECODED;

	case PR_SET_UNALIGN:
		tprints(", ");
		printflags(pr_unalign_flags, arg2, "PR_UNALIGN_???");
		return RVAL_DECODED;

	case PR_SET_NO_NEW_PRIVS:
	case PR_SET_THP_DISABLE:
	case PR_SET_IO_FLUSHER:
		tprintf(", %" PRI_klu, arg2);
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
				      (kernel_ulong_t) tcp->u_rval);
		return RVAL_STR;

	case PR_SET_FP_MODE:
		tprints(", ");
		printflags(pr_fp_mode, arg2, "PR_FP_MODE_???");
		return RVAL_DECODED;

	case PR_SET_SPECULATION_CTRL:
		tprints(", ");
		printxval64(pr_spec_cmds, arg2, "PR_SPEC_???");
		tprints(", ");

		switch (arg2) {
		case PR_SPEC_STORE_BYPASS:
		case PR_SPEC_INDIRECT_BRANCH:
			printxval64(pr_spec_set_store_bypass_flags, arg3,
				    "PR_SPEC_???");
			break;

		default:
			tprintf("%#" PRI_klx, arg3);
		}

		return RVAL_DECODED;

	case PR_GET_NO_NEW_PRIVS:
	case PR_GET_THP_DISABLE:
	case PR_MPX_DISABLE_MANAGEMENT:
	case PR_MPX_ENABLE_MANAGEMENT:
	case PR_GET_IO_FLUSHER:
	default:
		print_prctl_args(tcp, 1);
		return RVAL_DECODED;
	}
	return 0;
}

#if defined X86_64 || defined X32 || defined I386
# include "xlat/archvals.h"

SYS_FUNC(arch_prctl)
{
	const unsigned int option = tcp->u_arg[0];
	const kernel_ulong_t addr = tcp->u_arg[1];

	if (entering(tcp))
		printxval(archvals, option, "ARCH_???");

	switch (option) {
	case ARCH_GET_GS:
	case ARCH_GET_FS:
		if (entering(tcp))
			tprints(", ");
		else
			printnum_kptr(tcp, addr);
		return 0;

	case ARCH_GET_CPUID: /* has no arguments */
		return RVAL_DECODED;
	}

	tprintf(", %#" PRI_klx, addr);
	return RVAL_DECODED;
}
#endif /* X86_64 || X32 || I386 */
