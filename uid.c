#ifdef STRACE_UID_SIZE
# if STRACE_UID_SIZE != 16
#  error invalid STRACE_UID_SIZE
# endif

# define SIZEIFY(x)		SIZEIFY_(x,STRACE_UID_SIZE)
# define SIZEIFY_(x,size)	SIZEIFY__(x,size)
# define SIZEIFY__(x,size)	x ## size

# define sys_getuid	SIZEIFY(sys_getuid)
# define sys_setfsuid	SIZEIFY(sys_setfsuid)
# define sys_setuid	SIZEIFY(sys_setuid)
# define sys_getresuid	SIZEIFY(sys_getresuid)
# define sys_setreuid	SIZEIFY(sys_setreuid)
# define sys_setresuid	SIZEIFY(sys_setresuid)
# define sys_chown	SIZEIFY(sys_chown)
# define sys_fchown	SIZEIFY(sys_fchown)
# define printuid	SIZEIFY(printuid)
#endif /* STRACE_UID_SIZE */

#include "defs.h"

#ifdef STRACE_UID_SIZE
# if !NEED_UID16_PARSERS
#  undef STRACE_UID_SIZE
# endif
#else
# define STRACE_UID_SIZE 32
#endif

#ifdef STRACE_UID_SIZE

# undef uid_t
# define uid_t		uid_t_(STRACE_UID_SIZE)
# define uid_t_(size)	uid_t__(size)
# define uid_t__(size)	uint ## size ## _t

int
sys_getuid(struct tcb *tcp)
{
	if (exiting(tcp))
		tcp->u_rval = (uid_t) tcp->u_rval;
	return RVAL_UDECIMAL;
}

int
sys_setfsuid(struct tcb *tcp)
{
	if (entering(tcp))
		tprintf("%u", (uid_t) tcp->u_arg[0]);
	else
		tcp->u_rval = (uid_t) tcp->u_rval;
	return RVAL_UDECIMAL;
}

int
sys_setuid(struct tcb *tcp)
{
	if (entering(tcp)) {
		tprintf("%u", (uid_t) tcp->u_arg[0]);
	}
	return 0;
}

static void
get_print_uid(struct tcb *tcp, const char *prefix, const long addr)
{
	uid_t uid;

	if (umove(tcp, addr, &uid) < 0)
		tprintf("%s%#lx", prefix, addr);
	else
		tprintf("%s[%u]", prefix, uid);
}

int
sys_getresuid(struct tcb *tcp)
{
	if (exiting(tcp)) {
		if (syserror(tcp)) {
			tprintf("%#lx, %#lx, %#lx", tcp->u_arg[0],
				tcp->u_arg[1], tcp->u_arg[2]);
		} else {
			get_print_uid(tcp, "", tcp->u_arg[0]);
			get_print_uid(tcp, ", ", tcp->u_arg[1]);
			get_print_uid(tcp, ", ", tcp->u_arg[2]);
		}
	}
	return 0;
}

int
sys_setreuid(struct tcb *tcp)
{
	if (entering(tcp)) {
		printuid("", tcp->u_arg[0]);
		printuid(", ", tcp->u_arg[1]);
	}
	return 0;
}

int
sys_setresuid(struct tcb *tcp)
{
	if (entering(tcp)) {
		printuid("", tcp->u_arg[0]);
		printuid(", ", tcp->u_arg[1]);
		printuid(", ", tcp->u_arg[2]);
	}
	return 0;
}

int
sys_chown(struct tcb *tcp)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		printuid(", ", tcp->u_arg[1]);
		printuid(", ", tcp->u_arg[2]);
	}
	return 0;
}

int
sys_fchown(struct tcb *tcp)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		printuid(", ", tcp->u_arg[1]);
		printuid(", ", tcp->u_arg[2]);
	}
	return 0;
}

void
printuid(const char *text, const unsigned int uid)
{
	if ((unsigned int) -1 == uid || (uid_t) -1 == uid)
		tprintf("%s-1", text);
	else
		tprintf("%s%u", text, uid);
}

#endif /* STRACE_UID_SIZE */
