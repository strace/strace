#include "defs.h"

/* defines copied from linux/sched.h since we can't include that
 * ourselves (it conflicts with *lots* of libc includes)
 */
#define CSIGNAL         0x000000ff      /* signal mask to be sent at exit */
#define CLONE_VM        0x00000100      /* set if VM shared between processes */
#define CLONE_FS        0x00000200      /* set if fs info shared between processes */
#define CLONE_FILES     0x00000400      /* set if open files shared between processes */
#define CLONE_SIGHAND   0x00000800      /* set if signal handlers shared */
#define CLONE_IDLETASK  0x00001000      /* kernel-only flag */
#define CLONE_PTRACE    0x00002000      /* set if we want to let tracing continue on the child too */
#define CLONE_VFORK     0x00004000      /* set if the parent wants the child to wake it up on mm_release */
#define CLONE_PARENT    0x00008000      /* set if we want to have the same parent as the cloner */
#define CLONE_THREAD	0x00010000	/* Same thread group? */
#define CLONE_NEWNS	0x00020000	/* New namespace group? */
#define CLONE_SYSVSEM	0x00040000	/* share system V SEM_UNDO semantics */
#define CLONE_SETTLS	0x00080000	/* create a new TLS for the child */
#define CLONE_PARENT_SETTID	0x00100000	/* set the TID in the parent */
#define CLONE_CHILD_CLEARTID	0x00200000	/* clear the TID in the child */
#define CLONE_UNTRACED		0x00800000	/* set if the tracing process can't force CLONE_PTRACE on this clone */
#define CLONE_CHILD_SETTID	0x01000000	/* set the TID in the child */
#define CLONE_STOPPED		0x02000000	/* Start in stopped state */
#define CLONE_NEWUTS		0x04000000	/* New utsname group? */
#define CLONE_NEWIPC		0x08000000	/* New ipcs */
#define CLONE_NEWUSER		0x10000000	/* New user namespace */
#define CLONE_NEWPID		0x20000000	/* New pid namespace */
#define CLONE_NEWNET		0x40000000	/* New network namespace */
#define CLONE_IO		0x80000000	/* Clone io context */

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
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		printflags(clone_flags, tcp->u_arg[1], "CLONE_???");
	}
	return 0;
}

SYS_FUNC(unshare)
{
	if (entering(tcp))
		printflags(clone_flags, tcp->u_arg[0], "CLONE_???");
	return 0;
}

SYS_FUNC(fork)
{
	if (exiting(tcp))
		return RVAL_UDECIMAL;
	return 0;
}
