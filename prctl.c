#include "defs.h"

#ifdef HAVE_PRCTL
#include <sys/prctl.h>

#include "xlat/prctl_options.h"

static const char *
unalignctl_string(unsigned int ctl)
{
	static char buf[sizeof(int)*2 + 2];

	switch (ctl) {
#ifdef PR_UNALIGN_NOPRINT
		case PR_UNALIGN_NOPRINT:
			return "NOPRINT";
#endif
#ifdef PR_UNALIGN_SIGBUS
		case PR_UNALIGN_SIGBUS:
			return "SIGBUS";
#endif
		default:
			break;
	}
	sprintf(buf, "%x", ctl);
	return buf;
}

int
sys_prctl(struct tcb *tcp)
{
	unsigned int i;

	if (entering(tcp)) {
		printxval(prctl_options, tcp->u_arg[0], "PR_???");
		switch (tcp->u_arg[0]) {
#ifdef PR_GETNSHARE
		case PR_GETNSHARE:
			break;
#endif
#ifdef PR_SET_PDEATHSIG
		case PR_SET_PDEATHSIG:
			tprintf(", %lu", tcp->u_arg[1]);
			break;
#endif
#ifdef PR_GET_PDEATHSIG
		case PR_GET_PDEATHSIG:
			break;
#endif
#ifdef PR_SET_DUMPABLE
		case PR_SET_DUMPABLE:
			tprintf(", %lu", tcp->u_arg[1]);
			break;
#endif
#ifdef PR_GET_DUMPABLE
		case PR_GET_DUMPABLE:
			break;
#endif
#ifdef PR_SET_UNALIGN
		case PR_SET_UNALIGN:
			tprintf(", %s", unalignctl_string(tcp->u_arg[1]));
			break;
#endif
#ifdef PR_GET_UNALIGN
		case PR_GET_UNALIGN:
			tprintf(", %#lx", tcp->u_arg[1]);
			break;
#endif
#ifdef PR_SET_KEEPCAPS
		case PR_SET_KEEPCAPS:
			tprintf(", %lu", tcp->u_arg[1]);
			break;
#endif
#ifdef PR_GET_KEEPCAPS
		case PR_GET_KEEPCAPS:
			break;
#endif
		default:
			for (i = 1; i < tcp->s_ent->nargs; i++)
				tprintf(", %#lx", tcp->u_arg[i]);
			break;
		}
	} else {
		switch (tcp->u_arg[0]) {
#ifdef PR_GET_PDEATHSIG
		case PR_GET_PDEATHSIG:
			if (umove(tcp, tcp->u_arg[1], &i) < 0)
				tprintf(", %#lx", tcp->u_arg[1]);
			else
				tprintf(", {%u}", i);
			break;
#endif
#ifdef PR_GET_DUMPABLE
		case PR_GET_DUMPABLE:
			return RVAL_UDECIMAL;
#endif
#ifdef PR_GET_UNALIGN
		case PR_GET_UNALIGN:
			if (syserror(tcp) || umove(tcp, tcp->u_arg[1], &i) < 0)
				break;
			tcp->auxstr = unalignctl_string(i);
			return RVAL_STR;
#endif
#ifdef PR_GET_KEEPCAPS
		case PR_GET_KEEPCAPS:
			return RVAL_UDECIMAL;
#endif
		default:
			break;
		}
	}
	return 0;
}
#endif /* HAVE_PRCTL */

#if defined X86_64 || defined X32
# include <asm/prctl.h>
# include "xlat/archvals.h"

int
sys_arch_prctl(struct tcb *tcp)
{
	if (entering(tcp)) {
		printxval(archvals, tcp->u_arg[0], "ARCH_???");
		if (tcp->u_arg[0] == ARCH_SET_GS
		 || tcp->u_arg[0] == ARCH_SET_FS
		) {
			tprintf(", %#lx", tcp->u_arg[1]);
		}
	} else {
		if (tcp->u_arg[0] == ARCH_GET_GS
		 || tcp->u_arg[0] == ARCH_GET_FS
		) {
			long int v;
			if (!syserror(tcp) && umove(tcp, tcp->u_arg[1], &v) != -1)
				tprintf(", [%#lx]", v);
			else
				tprintf(", %#lx", tcp->u_arg[1]);
		}
	}
	return 0;
}
#endif /* X86_64 || X32 */
