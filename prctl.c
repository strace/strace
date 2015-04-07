#include "defs.h"

#include <sys/prctl.h>

#include "xlat/prctl_options.h"
#include "xlat/pr_unalign_flags.h"
#include "xlat/pr_mce_kill.h"
#include "xlat/pr_mce_kill_policy.h"
#include "xlat/pr_set_mm.h"
#include "xlat/pr_tsc.h"

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

static int
prctl_enter(struct tcb *tcp)
{
	unsigned int i;

	printxval(prctl_options, tcp->u_arg[0], "PR_???");

	switch (tcp->u_arg[0]) {
	/* PR_GET_* are decoded on exit. */
	case PR_GET_CHILD_SUBREAPER:
	case PR_GET_DUMPABLE:
	case PR_GET_ENDIAN:
	case PR_GET_FPEMU:
	case PR_GET_FPEXC:
	case PR_GET_KEEPCAPS:
	case PR_GET_NAME:
	case PR_GET_PDEATHSIG:
	case PR_GET_SECCOMP:
	case PR_GET_SECUREBITS:
	case PR_GET_TID_ADDRESS:
	case PR_GET_TIMERSLACK:
	case PR_GET_TIMING:
	case PR_GET_TSC:
	case PR_GET_UNALIGN:
	/* PR_TASK_PERF_EVENTS_* have nothing to decode on enter. */
	case PR_TASK_PERF_EVENTS_DISABLE:
	case PR_TASK_PERF_EVENTS_ENABLE:
		break;

	case PR_SET_CHILD_SUBREAPER:
	case PR_SET_DUMPABLE:
	case PR_SET_ENDIAN:
	case PR_SET_FPEMU:
	case PR_SET_FPEXC:
	case PR_SET_KEEPCAPS:
	case PR_SET_TIMING:
		tprintf(", %lu", tcp->u_arg[1]);
		break;

	case PR_CAPBSET_DROP:
	case PR_CAPBSET_READ:
		tprints(", ");
		printxval(cap, tcp->u_arg[1], "CAP_???");
		break;

	case PR_MCE_KILL:
		tprints(", ");
		printxval(pr_mce_kill, tcp->u_arg[1], "PR_MCE_KILL_???");
		tprints(", ");
		if (PR_MCE_KILL_SET == tcp->u_arg[1])
			printxval(pr_mce_kill_policy, tcp->u_arg[2],
				   "PR_MCE_KILL_???");
		else
			tprintf("%#lx", tcp->u_arg[2]);
		for (i = 3; i < tcp->s_ent->nargs; i++)
			tprintf(", %#lx", tcp->u_arg[i]);
		break;

	case PR_SET_NAME:
		tprints(", ");
		printstr(tcp, tcp->u_arg[1], TASK_COMM_LEN);
		break;

	case PR_SET_MM:
		tprints(", ");
		printxval(pr_set_mm, tcp->u_arg[1], "PR_SET_MM_???");
		for (i = 2; i < tcp->s_ent->nargs; i++)
			tprintf(", %#lx", tcp->u_arg[i]);
		break;

	case PR_SET_PDEATHSIG:
		tprints(", ");
		if ((unsigned long) tcp->u_arg[1] > 128)
			tprintf("%lu", tcp->u_arg[1]);
		else
			tprints(signame(tcp->u_arg[1]));
		break;

	case PR_SET_PTRACER:
		tprints(", ");
		if (tcp->u_arg[1] == -1)
			tprints("PR_SET_PTRACER_ANY");
		else
			tprintf("%lu", tcp->u_arg[1]);
		break;

	case PR_SET_SECCOMP:
		tprints(", ");
		printxval(seccomp_mode, tcp->u_arg[1],
			  "SECCOMP_MODE_???");
		if (SECCOMP_MODE_STRICT == tcp->u_arg[1])
			break;
		if (SECCOMP_MODE_FILTER == tcp->u_arg[1]) {
			tprints(", ");
			print_seccomp_filter(tcp, tcp->u_arg[2]);
			break;
		}
		for (i = 2; i < tcp->s_ent->nargs; i++)
			tprintf(", %#lx", tcp->u_arg[i]);
		break;

	case PR_SET_SECUREBITS:
		tprints(", ");
		printflags(secbits, tcp->u_arg[1], "SECBIT_???");
		break;

	case PR_SET_TIMERSLACK:
		tprintf(", %ld", tcp->u_arg[1]);
		break;

	case PR_SET_TSC:
		tprints(", ");
		printxval(pr_tsc, tcp->u_arg[1], "PR_TSC_???");
		break;

	case PR_SET_UNALIGN:
		tprints(", ");
		printflags(pr_unalign_flags, tcp->u_arg[1], "PR_UNALIGN_???");
		break;

	case PR_SET_NO_NEW_PRIVS:
	case PR_SET_THP_DISABLE:
		tprintf(", %lu", tcp->u_arg[1]);
		for (i = 2; i < tcp->s_ent->nargs; i++)
			tprintf(", %#lx", tcp->u_arg[i]);
		break;

	case PR_GET_NO_NEW_PRIVS:
	case PR_GET_THP_DISABLE:
	case PR_MCE_KILL_GET:
	/* Return code of "GET" commands will be decoded on exit */
	case PR_MPX_DISABLE_MANAGEMENT:
	case PR_MPX_ENABLE_MANAGEMENT:
	default:
		for (i = 1; i < tcp->s_ent->nargs; i++)
			tprintf(", %#lx", tcp->u_arg[i]);
		break;
	}
	return 0;
}

static int
prctl_exit(struct tcb *tcp)
{
	unsigned long addr;
	unsigned int i;

	switch (tcp->u_arg[0]) {
	case PR_CAPBSET_READ:
	case PR_GET_DUMPABLE:
	case PR_GET_KEEPCAPS:
	case PR_GET_NO_NEW_PRIVS:
	case PR_GET_SECCOMP:
	case PR_GET_THP_DISABLE:
	case PR_GET_TIMERSLACK:
	case PR_GET_TIMING:
		return syserror(tcp) ? 0 : RVAL_UDECIMAL;

	case PR_GET_CHILD_SUBREAPER:
	case PR_GET_ENDIAN:
	case PR_GET_FPEMU:
	case PR_GET_FPEXC:
		tprints(", ");
		/* cannot use printnum_int() because of syserror() */
		if (!tcp->u_arg[1])
			tprints("NULL");
		else if (syserror(tcp) || umove(tcp, tcp->u_arg[1], &i) < 0)
			tprintf("%#lx", tcp->u_arg[1]);
		else
			tprintf("[%u]", i);
		break;

	case PR_GET_NAME:
		tprints(", ");
		if (!tcp->u_arg[1])
			tprints("NULL");
		else if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else
			printstr(tcp, tcp->u_arg[1], -1);
		break;

	case PR_GET_PDEATHSIG:
		tprints(", ");
		if (!tcp->u_arg[1])
			tprints("NULL");
		else if (syserror(tcp) || umove(tcp, tcp->u_arg[1], &i) < 0)
			tprintf("%#lx", tcp->u_arg[1]);
		else {
			tprints("[");
			tprints(signame(i));
			tprints("]");
		}
		break;

	case PR_GET_SECUREBITS:
		if (syserror(tcp) || tcp->u_rval == 0)
			return 0;
		tcp->auxstr = sprintflags("", secbits, tcp->u_rval);
		return RVAL_STR;

	case PR_GET_TID_ADDRESS:
		tprints(", ");
		/* cannot use printnum_long() because of syserror() */
		if (!tcp->u_arg[1])
			tprints("NULL");
		else if (syserror(tcp) || umove(tcp, tcp->u_arg[1], &addr) < 0)
			tprintf("%#lx", tcp->u_arg[1]);
		else
			tprintf("[%#lx]", addr);
		break;

	case PR_GET_TSC:
		tprints(", ");
		if (!tcp->u_arg[1])
			tprints("NULL");
		else if (syserror(tcp) || umove(tcp, tcp->u_arg[1], &i) < 0)
			tprintf("%#lx", tcp->u_arg[1]);
		else {
			tprints("[");
			printxval(pr_tsc, i, "PR_TSC_???");
			tprints("]");
		}
		break;

	case PR_GET_UNALIGN:
		tprints(", ");
		if (!tcp->u_arg[1])
			tprints("NULL");
		else if (syserror(tcp) || umove(tcp, tcp->u_arg[1], &i) < 0)
			tprintf("%#lx", tcp->u_arg[1]);
		else {
			tprints("[");
			printflags(pr_unalign_flags, i, "PR_UNALIGN_???");
			tprints("]");
		}
		break;

	case PR_MCE_KILL_GET:
		if (syserror(tcp))
			return 0;
		tcp->auxstr = xlookup(pr_mce_kill_policy, tcp->u_rval);
		return tcp->auxstr ? RVAL_STR : RVAL_UDECIMAL;

	default:
		break;
	}
	return 0;
}

SYS_FUNC(prctl)
{
	return entering(tcp) ? prctl_enter(tcp) : prctl_exit(tcp);
}

#if defined X86_64 || defined X32
# include <asm/prctl.h>
# include "xlat/archvals.h"

SYS_FUNC(arch_prctl)
{
	if (entering(tcp))
		printxval(archvals, tcp->u_arg[0], "ARCH_???");

	switch (tcp->u_arg[0]) {
	case ARCH_GET_GS:
	case ARCH_GET_FS:
		if (exiting(tcp)) {
			if (syserror(tcp))
				break;
			tprints(", ");
			printnum_long(tcp, tcp->u_arg[1], "%#lx");
		}
		return 0;
	default:
		if (exiting(tcp))
			return 0;
	}

	tprintf(", %#lx", tcp->u_arg[1]);
	return 0;
}
#endif /* X86_64 || X32 */
