/*
 * Copyright (c) 1994-1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-2000 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2005-2007 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2008-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2014-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "xstring.h"

#include <linux/prctl.h>

#include "xlat/prctl_options.h"
#include "xlat/pr_cap_ambient.h"
#include "xlat/pr_dumpable.h"
#include "xlat/pr_fp_mode.h"
#include "xlat/pr_mce_kill.h"
#include "xlat/pr_mce_kill_policy.h"
#include "xlat/pr_mdwe_flags.h"
#include "xlat/pr_pac_enabled_keys.h"
#include "xlat/pr_pac_keys.h"
#include "xlat/pr_sched_core_cmds.h"
#include "xlat/pr_sched_core_pidtypes.h"
#include "xlat/pr_set_mm.h"
#include "xlat/pr_set_vma.h"
#include "xlat/pr_sme_vl_flags.h"
#include "xlat/pr_spec_cmds.h"
#include "xlat/pr_spec_get_store_bypass_flags.h"
#include "xlat/pr_spec_set_store_bypass_flags.h"
#include "xlat/pr_sud_cmds.h"
#include "xlat/pr_sve_vl_flags.h"
#include "xlat/pr_tagged_addr_enable.h"
#include "xlat/pr_tagged_addr_mte_tcf.h"
#include "xlat/pr_tsc.h"
#include "xlat/pr_unalign_flags.h"

#ifndef TASK_COMM_LEN
# define TASK_COMM_LEN 16
#endif

#include <linux/seccomp.h>
#include "xlat/seccomp_mode.h"

#include <linux/securebits.h>
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
	for (unsigned int i = first; i < n_args(tcp); ++i) {
		tprint_arg_next();
		PRINT_VAL_X(tcp->u_arg[i]);
	}
}

static char *
sprint_sve_val(kernel_ulong_t arg, bool aux)
{
	static char out[sizeof("0x /* PR_SVE_SET_VL_ONEXEC|PR_SVE_VL_INHERIT"
			       "|0x|0x */") + sizeof(kernel_ulong_t) * 2 * 3];

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW) {
		if (aux)
			return NULL;

		xsprintf(out, "%#" PRI_klx, arg);

		return out;
	}

	kernel_ulong_t vl = arg & PR_SVE_VL_LEN_MASK;
	kernel_ulong_t flags = arg & ~PR_SVE_VL_LEN_MASK;

	if (!flags && aux)
		return NULL;

	const char *flags_str = sprintflags_ex("", pr_sve_vl_flags, flags, '\0',
					       XLAT_STYLE_ABBREV);

	if (!aux && flags && xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
	{
		xsprintf(out, "%#" PRI_klx " /* %s%s%#" PRI_klx " */",
			 arg, flags_str ?: "", flags_str ? "|" : "", vl);
	} else {
		xsprintf(out, "%s%s%#" PRI_klx,
			 flags_str ?: "", flags_str ? "|" : "", vl);
	}

	return out;
}

static char *
sprint_sme_val(kernel_ulong_t arg, bool aux)
{
	static char out[sizeof("0x /* PR_SME_SET_VL_ONEXEC|PR_SME_VL_INHERIT"
			       "|0x|0x */") + sizeof(kernel_ulong_t) * 2 * 3];

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW) {
		if (aux)
			return NULL;

		xsprintf(out, "%#" PRI_klx, arg);

		return out;
	}

	kernel_ulong_t vl = arg & PR_SME_VL_LEN_MASK;
	kernel_ulong_t flags = arg & ~PR_SME_VL_LEN_MASK;

	if (!flags && aux)
		return NULL;

	const char *flags_str = sprintflags_ex("", pr_sme_vl_flags, flags, '\0',
					       XLAT_STYLE_ABBREV);

	if (!aux && flags && xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
	{
		xsprintf(out, "%#" PRI_klx " /* %s%s%#" PRI_klx " */",
			 arg, flags_str ?: "", flags_str ? "|" : "", vl);
	} else {
		xsprintf(out, "%s%s%#" PRI_klx,
			 flags_str ?: "", flags_str ? "|" : "", vl);
	}

	return out;
}

static char *
sprint_tagged_addr_val(const kernel_ulong_t arg, bool rval)
{
	static char out[sizeof("0x /* !PR_TAGGED_ADDR_ENABLE|PR_MTE_TCF_ASYNC"
			"|0xffff<<PR_MTE_TAG_SHIFT|0x */") +
			sizeof(kernel_ulong_t) * 2 * 2];

	const kernel_ulong_t enabled = arg & PR_TAGGED_ADDR_ENABLE;
	const kernel_ulong_t mte_tcf = arg & PR_MTE_TCF_MASK;
	const kernel_ulong_t mte_tag = arg & PR_MTE_TAG_MASK;
	const kernel_ulong_t rest = arg &
		~((kernel_ulong_t) PR_TAGGED_ADDR_ENABLE | PR_MTE_TCF_MASK
		  | PR_MTE_TAG_MASK);
	char *pos = out;

	if (!rval && (xlat_verbose(xlat_verbosity) != XLAT_STYLE_ABBREV))
		pos = xappendstr(out, pos, "%#" PRI_klx, arg);
	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW)
		return rval ? NULL : out;
	if (!rval && (xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE))
		pos = xappendstr(out, pos, " /* ");

	pos = xappendstr(out, pos, "%s|",
			 sprintflags_ex("", pr_tagged_addr_enable, enabled,
					'\0', XLAT_STYLE_ABBREV));
	pos += sprintxval_ex(pos, sizeof(out) - (pos - out),
			     pr_tagged_addr_mte_tcf, mte_tcf, NULL,
			     XLAT_STYLE_ABBREV);
	if (mte_tag) {
		pos = xappendstr(out, pos, "|%#" PRI_klx "<<PR_MTE_TAG_SHIFT",
				 mte_tag >> PR_MTE_TAG_SHIFT);
	}
	if (rest)
		pos = xappendstr(out, pos, "|%#" PRI_klx, rest);

	if (!rval && (xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE))
		pos = xappendstr(out, pos, " */");

	return out;
}

SYS_FUNC(prctl)
{
	const unsigned int option = tcp->u_arg[0];
	const kernel_ulong_t arg2 = tcp->u_arg[1];
	const kernel_ulong_t arg3 = tcp->u_arg[2];
	const kernel_ulong_t arg4 = tcp->u_arg[3];
	const kernel_ulong_t arg5 = tcp->u_arg[4];
	unsigned int i;

	if (entering(tcp))
		printxval(prctl_options, option, "PR_???");

	switch (option) {
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
			tprint_arg_next();
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
			tprint_arg_next();
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
			tprint_arg_next();
		} else if (!umove_or_printaddr(tcp, arg2, &i)) {
			tprint_indirect_begin();
			printsignal(i);
			tprint_indirect_end();
		}
		break;

	case PR_GET_SECUREBITS:
		if (entering(tcp))
			break;
		if (syserror(tcp) || tcp->u_rval == 0)
			return 0;
		tcp->auxstr = sprintflags_ex("", secbits,
				(kernel_ulong_t) tcp->u_rval, '\0',
				XLAT_STYLE_DEFAULT | SPFF_AUXSTR_MODE);
		return RVAL_HEX | RVAL_STR;

	case PR_GET_TID_ADDRESS:
		if (entering(tcp))
			tprint_arg_next();
		else
			printnum_kptr(tcp, arg2);
		break;

	case PR_GET_TSC:
		if (entering(tcp)) {
			tprint_arg_next();
		} else if (!umove_or_printaddr(tcp, arg2, &i)) {
			tprint_indirect_begin();
			printxval(pr_tsc, i, "PR_TSC_???");
			tprint_indirect_end();
		}
		break;

	case PR_GET_UNALIGN:
		if (entering(tcp)) {
			tprint_arg_next();
		} else if (!umove_or_printaddr(tcp, arg2, &i)) {
			tprint_indirect_begin();
			printflags(pr_unalign_flags, i, "PR_UNALIGN_???");
			tprint_indirect_end();
		}
		break;

	case PR_GET_FP_MODE:
		if (entering(tcp))
			break;
		if (syserror(tcp) || tcp->u_rval == 0)
			return 0;
		tcp->auxstr = sprintflags_ex("", pr_fp_mode,
				(kernel_ulong_t) tcp->u_rval, '\0',
				XLAT_STYLE_DEFAULT | SPFF_AUXSTR_MODE);
		return RVAL_HEX | RVAL_STR;

	case PR_SVE_SET_VL:
		if (entering(tcp)) {
			tprint_arg_next();
			tprints_string(sprint_sve_val(arg2, false));
			return 0;
		}
		ATTRIBUTE_FALLTHROUGH;

	case PR_SVE_GET_VL:
		if (entering(tcp))
			break;
		if (syserror(tcp) || tcp->u_rval == 0)
			return 0;

		tcp->auxstr = sprint_sve_val(tcp->u_rval, true);

		return RVAL_HEX | RVAL_STR;

	case PR_GET_SPECULATION_CTRL:
		if (entering(tcp)) {
			tprint_arg_next();
			printxval64(pr_spec_cmds, arg2, "PR_SPEC_???");

			break;
		}

		if (syserror(tcp))
			return 0;

		switch (arg2) {
		case PR_SPEC_STORE_BYPASS:
		case PR_SPEC_INDIRECT_BRANCH:
		case PR_SPEC_L1D_FLUSH:
			tcp->auxstr = sprintflags_ex("",
					pr_spec_get_store_bypass_flags,
					(kernel_ulong_t) tcp->u_rval, '\0',
					XLAT_STYLE_DEFAULT | SPFF_AUXSTR_MODE);
			return RVAL_HEX | RVAL_STR;
		}

		return RVAL_STR;

	case PR_SET_TAGGED_ADDR_CTRL:
		tprint_arg_next();
		tprints_string(sprint_tagged_addr_val(arg2, false));
		print_prctl_args(tcp, 2);
		return RVAL_DECODED;

	case PR_GET_TAGGED_ADDR_CTRL:
		if (entering(tcp)) {
			print_prctl_args(tcp, 1);
			break;
		}
		if (syserror(tcp))
			return 0;
		tcp->auxstr = sprint_tagged_addr_val(tcp->u_rval, true);

		return RVAL_HEX | RVAL_STR;

	case PR_SME_SET_VL:
		if (entering(tcp)) {
			tprint_arg_next();
			tprints_string(sprint_sme_val(arg2, false));
			return 0;
		}
		ATTRIBUTE_FALLTHROUGH;

	case PR_SME_GET_VL:
		if (entering(tcp))
			break;
		if (syserror(tcp) || tcp->u_rval == 0)
			return 0;

		tcp->auxstr = sprint_sme_val(tcp->u_rval, true);

		return RVAL_HEX | RVAL_STR;

	case PR_GET_MDWE:
		if (entering(tcp)) {
			print_prctl_args(tcp, 1);
			break;
		}
		if (syserror(tcp))
			return 0;
		tcp->auxstr = sprintflags_ex("", pr_mdwe_flags,
				(kernel_ulong_t) tcp->u_rval, '\0',
				XLAT_STYLE_DEFAULT | SPFF_AUXSTR_MODE);
		return RVAL_HEX | RVAL_STR;

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
		tprint_arg_next();
		PRINT_VAL_U(arg2);
		return RVAL_DECODED;

	case PR_SET_DUMPABLE:
		tprint_arg_next();
		printxval64(pr_dumpable, arg2, "SUID_DUMP_???");
		return RVAL_DECODED;

	case PR_CAPBSET_DROP:
	case PR_CAPBSET_READ:
		tprint_arg_next();
		printxval64(cap, arg2, "CAP_???");
		return RVAL_DECODED;

	case PR_CAP_AMBIENT:
		tprint_arg_next();
		printxval64(pr_cap_ambient, arg2,
			       "PR_CAP_AMBIENT_???");
		switch (arg2) {
		case PR_CAP_AMBIENT_RAISE:
		case PR_CAP_AMBIENT_LOWER:
		case PR_CAP_AMBIENT_IS_SET:
			tprint_arg_next();
			printxval64(cap, arg3, "CAP_???");
			print_prctl_args(tcp, 3);
			break;
		default:
			print_prctl_args(tcp, 2);
			break;
		}
		return RVAL_DECODED;

	case PR_MCE_KILL:
		tprint_arg_next();
		printxval64(pr_mce_kill, arg2, "PR_MCE_KILL_???");
		tprint_arg_next();
		if (PR_MCE_KILL_SET == arg2)
			printxval64(pr_mce_kill_policy, arg3,
				    "PR_MCE_KILL_???");
		else
			PRINT_VAL_X(arg3);
		print_prctl_args(tcp, 3);
		return RVAL_DECODED;

	case PR_SET_NAME:
		tprint_arg_next();
		printstr_ex(tcp, arg2, TASK_COMM_LEN - 1,
			    QUOTE_0_TERMINATED);
		return RVAL_DECODED;

	case PR_SET_VMA:
		tprint_arg_next();
		printxval64(pr_set_vma, arg2, "PR_SET_VMA_???");
		if (arg2 == PR_SET_VMA_ANON_NAME) {
			tprint_arg_next();
			printaddr(arg3);
			tprint_arg_next();
			PRINT_VAL_U(arg4);
			tprint_arg_next();
			printstr(tcp, arg5);
		} else {
			/* There are no other sub-options now, but there
			 * might be in future... */
			print_prctl_args(tcp, 2);
		}
		return RVAL_DECODED;

	case PR_SET_MM:
		tprint_arg_next();
		printxval(pr_set_mm, arg2, "PR_SET_MM_???");
		print_prctl_args(tcp, 2);
		return RVAL_DECODED;

	case PR_SET_PDEATHSIG:
		tprint_arg_next();
		if (arg2 > 128)
			PRINT_VAL_U(arg2);
		else
			printsignal(arg2);
		return RVAL_DECODED;

	case PR_SET_PTRACER:
		tprint_arg_next();
		if ((int) arg2 == -1) {
			print_xlat_ex((int) arg2, "PR_SET_PTRACER_ANY",
				      XLAT_STYLE_FMT_D);
		} else {
			printpid(tcp, arg2, PT_TGID);
		}
		return RVAL_DECODED;

	case PR_SET_SECCOMP:
		tprint_arg_next();
		printxval64(seccomp_mode, arg2,
			    "SECCOMP_MODE_???");
		if (SECCOMP_MODE_STRICT == arg2)
			return RVAL_DECODED;
		if (SECCOMP_MODE_FILTER == arg2) {
			tprint_arg_next();
			decode_seccomp_fprog(tcp, arg3);
			return RVAL_DECODED;
		}
		print_prctl_args(tcp, 2);
		return RVAL_DECODED;

	case PR_SET_SECUREBITS:
		tprint_arg_next();
		printflags64(secbits, arg2, "SECBIT_???");
		return RVAL_DECODED;

	case PR_SET_TIMERSLACK:
		tprint_arg_next();
		PRINT_VAL_D(arg2);
		return RVAL_DECODED;

	case PR_SET_TSC:
		tprint_arg_next();
		printxval(pr_tsc, arg2, "PR_TSC_???");
		return RVAL_DECODED;

	case PR_SET_UNALIGN:
		tprint_arg_next();
		printflags(pr_unalign_flags, arg2, "PR_UNALIGN_???");
		return RVAL_DECODED;

	case PR_SET_MDWE:
		tprint_arg_next();
		printflags(pr_mdwe_flags, arg2, "PR_MDWE_???");
		print_prctl_args(tcp, 2);
		return RVAL_DECODED;

	case PR_SET_NO_NEW_PRIVS:
	case PR_SET_THP_DISABLE:
	case PR_SET_IO_FLUSHER:
		tprint_arg_next();
		PRINT_VAL_U(arg2);
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
		tprint_arg_next();
		printflags(pr_fp_mode, arg2, "PR_FP_MODE_???");
		return RVAL_DECODED;

	case PR_SET_SPECULATION_CTRL:
		tprint_arg_next();
		printxval64(pr_spec_cmds, arg2, "PR_SPEC_???");
		tprint_arg_next();

		switch (arg2) {
		case PR_SPEC_STORE_BYPASS:
		case PR_SPEC_INDIRECT_BRANCH:
		case PR_SPEC_L1D_FLUSH:
			printxval64(pr_spec_set_store_bypass_flags, arg3,
				    "PR_SPEC_???");
			break;

		default:
			PRINT_VAL_X(arg3);
		}

		return RVAL_DECODED;

	case PR_PAC_RESET_KEYS:
		tprint_arg_next();
		printflags_ex(arg2, "PR_PAC_???", XLAT_STYLE_DEFAULT,
			      pr_pac_enabled_keys, pr_pac_keys, NULL);
		print_prctl_args(tcp, 2);

		return RVAL_DECODED;

	case PR_PAC_SET_ENABLED_KEYS:
		tprint_arg_next();
		printflags64(pr_pac_enabled_keys, arg2, "PR_PAC_???");
		tprint_arg_next();
		printflags64(pr_pac_enabled_keys, arg3, "PR_PAC_???");
		print_prctl_args(tcp, 3);

		return RVAL_DECODED;

	case PR_PAC_GET_ENABLED_KEYS:
		if (entering(tcp)) {
			print_prctl_args(tcp, 1);
			return 0;
		}
		if (syserror(tcp))
			return 0;
		tcp->auxstr = sprintflags_ex("", pr_pac_enabled_keys,
					     (kernel_ulong_t) tcp->u_rval, '\0',
					     XLAT_STYLE_DEFAULT | SPFF_AUXSTR_MODE);
		return RVAL_HEX | RVAL_STR;

	case PR_SET_SYSCALL_USER_DISPATCH:
		tprint_arg_next();
		printxval64(pr_sud_cmds, arg2, "PR_SYS_DISPATCH_???");
		tprint_arg_next();
		PRINT_VAL_X(arg3);
		tprint_arg_next();
		PRINT_VAL_X(arg4);
		tprint_arg_next();
		printaddr(arg5);

		return RVAL_DECODED;

	case PR_SCHED_CORE:
		if (entering(tcp)) {
			tprint_arg_next();
			printxval(pr_sched_core_cmds, arg2, "PR_SCHED_CORE_???");

			tprint_arg_next();
			enum pid_type pt;
			switch ((unsigned int) arg4) {
			case PIDTYPE_PID:  pt = PT_TID;  break;
			case PIDTYPE_TGID: pt = PT_TGID; break;
			case PIDTYPE_PGID: pt = PT_PGID; break;
			case PIDTYPE_SID:  pt = PT_SID;  break;
			default:           pt = PT_NONE;
			}
			printpid(tcp, arg3, pt);

			tprint_arg_next();
			printxval_ex(pr_sched_core_pidtypes,
				     (unsigned int) arg4, "PIDTYPE_???",
				     (xlat_verbose(xlat_verbosity)
							== XLAT_STYLE_RAW)
					? XLAT_STYLE_DEFAULT
					: XLAT_STYLE_VERBOSE);

			tprint_arg_next();
			switch ((unsigned int) arg2) {
			case PR_SCHED_CORE_GET:
				/* arg5 is to be decoded on exiting */
				return 0;
			default:
				printaddr(arg5);
			}
		} else {
			/* PR_SCHED_CORE_GET */
			if (syserror(tcp))
				printaddr(arg5);
			else
				printnum_int64(tcp, arg5, "%#" PRIx64);
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
# include "xlat/x86_xfeature_bits.h"
# include "xlat/x86_xfeatures.h"

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
			tprint_arg_next();
		else
			printnum_kptr(tcp, addr);
		return 0;

	case ARCH_GET_CPUID: /* has no arguments */
		return RVAL_DECODED;

	case ARCH_GET_XCOMP_SUPP:
	case ARCH_GET_XCOMP_PERM:
	case ARCH_GET_XCOMP_GUEST_PERM:
		if (entering(tcp)) {
			tprint_arg_next();
		} else {
			uint64_t val;

			if (umove_or_printaddr(tcp, addr, &val))
				return 0;

			/* XFEATURE_MASK_* macros are not publicly exposed */
			tprint_indirect_begin();
			printflags_ex(val, "XFEATURE_MASK_???",
				xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW
				      ? XLAT_STYLE_RAW : XLAT_STYLE_VERBOSE,
				x86_xfeatures, NULL);
			tprint_indirect_end();
		}

		return 0;

	case ARCH_REQ_XCOMP_PERM:
	case ARCH_REQ_XCOMP_GUEST_PERM:
		if (entering(tcp)) {
			/* XFEATURE_* enum is not publicly exposed */
			tprint_arg_next();
			printxvals_ex(addr, "XFEATURE_???",
				xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW
				      ? XLAT_STYLE_RAW : XLAT_STYLE_VERBOSE,
				x86_xfeature_bits, NULL);
		} else {
			if (tcp->u_rval <= 0)
				return 0;

			tcp->auxstr = sprintflags_ex("", x86_xfeatures,
					(kernel_ulong_t) tcp->u_rval, '\0',
					XLAT_STYLE_DEFAULT | SPFF_AUXSTR_MODE);

			return RVAL_HEX | RVAL_STR;
		}

		return 0;

	/* default handling: print arg2 in hexadecimal on entering */
	case ARCH_SET_GS:
	case ARCH_SET_FS:
	case ARCH_SET_CPUID:
	case ARCH_MAP_VDSO_X32:
	case ARCH_MAP_VDSO_32:
	case ARCH_MAP_VDSO_64:
	default:
		break;
	}

	tprint_arg_next();
	PRINT_VAL_X(addr);

	return RVAL_DECODED;
}
#endif /* X86_64 || X32 || I386 */
