#include "defs.h"

#include <asm/posix_types.h>

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

int
sys_getresuid(struct tcb *tcp)
{
	if (exiting(tcp)) {
		__kernel_uid_t uid;
		if (syserror(tcp))
			tprintf("%#lx, %#lx, %#lx", tcp->u_arg[0],
				tcp->u_arg[1], tcp->u_arg[2]);
		else {
			if (umove(tcp, tcp->u_arg[0], &uid) < 0)
				tprintf("%#lx, ", tcp->u_arg[0]);
			else
				tprintf("[%lu], ", (unsigned long) uid);
			if (umove(tcp, tcp->u_arg[1], &uid) < 0)
				tprintf("%#lx, ", tcp->u_arg[1]);
			else
				tprintf("[%lu], ", (unsigned long) uid);
			if (umove(tcp, tcp->u_arg[2], &uid) < 0)
				tprintf("%#lx", tcp->u_arg[2]);
			else
				tprintf("[%lu]", (unsigned long) uid);
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
	if ((unsigned int) -1 == uid)
		tprintf("%s-1", text);
	else
		tprintf("%s%u", text, uid);
}
