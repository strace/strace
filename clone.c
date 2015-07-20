#include "defs.h"

#include <sched.h>

#ifndef CSIGNAL
# define CSIGNAL 0x000000ff
#endif

#include "xlat/clone_flags.h"

#if defined IA64
# define ARG_FLAGS	0
# define ARG_STACK	1
# define ARG_STACKSIZE	(tcp->scno == SYS_clone2 ? 2 : -1)
# define ARG_PTID	(tcp->scno == SYS_clone2 ? 3 : 2)
# define ARG_CTID	(tcp->scno == SYS_clone2 ? 4 : 3)
# define ARG_TLS	(tcp->scno == SYS_clone2 ? 5 : 4)
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
#elif defined ALPHA || defined TILE || defined OR1K
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

#if defined I386 || defined X86_64 || defined X32
extern void print_user_desc(struct tcb *, long);
#endif /* I386 || X86_64 || X32 */

SYS_FUNC(clone)
{
	if (exiting(tcp)) {
		const char *sep = "|";
		unsigned long flags = tcp->u_arg[ARG_FLAGS];
		tprintf("child_stack=%#lx, ", tcp->u_arg[ARG_STACK]);
#ifdef ARG_STACKSIZE
		if (ARG_STACKSIZE != -1)
			tprintf("stack_size=%#lx, ",
				tcp->u_arg[ARG_STACKSIZE]);
#endif
		tprints("flags=");
		if (!printflags(clone_flags, flags &~ CSIGNAL, NULL))
			sep = "";
		if ((flags & CSIGNAL) != 0)
			tprintf("%s%s", sep, signame(flags & CSIGNAL));
		if ((flags & (CLONE_PARENT_SETTID|CLONE_CHILD_SETTID
			      |CLONE_CHILD_CLEARTID|CLONE_SETTLS)) == 0)
			return 0;
		if (flags & CLONE_PARENT_SETTID)
			tprintf(", parent_tidptr=%#lx", tcp->u_arg[ARG_PTID]);
		if (flags & CLONE_SETTLS) {
#if defined I386 || defined X86_64 || defined X32
# ifndef I386
			if (current_personality == 1)
# endif
			{
				tprints(", tls=");
				print_user_desc(tcp, tcp->u_arg[ARG_TLS]);
			}
# ifndef I386
			else
# endif
#endif /* I386 || X86_64 || X32 */
				tprintf(", tls=%#lx", tcp->u_arg[ARG_TLS]);
		}
		if (flags & (CLONE_CHILD_SETTID|CLONE_CHILD_CLEARTID))
			tprintf(", child_tidptr=%#lx", tcp->u_arg[ARG_CTID]);
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
	printflags(clone_flags, tcp->u_arg[1], "CLONE_???");

	return RVAL_DECODED;
}

SYS_FUNC(unshare)
{
	printflags(clone_flags, tcp->u_arg[0], "CLONE_???");
	return RVAL_DECODED;
}

SYS_FUNC(fork)
{
	return RVAL_DECODED | RVAL_UDECIMAL;
}
