#include "defs.h"

int
sys_process_vm_readv(struct tcb *tcp)
{
	if (entering(tcp)) {
		/* arg 1: pid */
		tprintf("%ld, ", tcp->u_arg[0]);
	} else {
		/* arg 2: local iov */
		if (syserror(tcp)) {
			tprintf("%#lx", tcp->u_arg[1]);
		} else {
			tprint_iov(tcp, tcp->u_arg[2], tcp->u_arg[1], 1);
		}
		/* arg 3: local iovcnt */
		tprintf(", %lu, ", tcp->u_arg[2]);
		/* arg 4: remote iov */
		if (syserror(tcp)) {
			tprintf("%#lx", tcp->u_arg[3]);
		} else {
			tprint_iov(tcp, tcp->u_arg[4], tcp->u_arg[3], 0);
		}
		/* arg 5: remote iovcnt */
		/* arg 6: flags */
		tprintf(", %lu, %lu", tcp->u_arg[4], tcp->u_arg[5]);
	}
	return 0;
}

int
sys_process_vm_writev(struct tcb *tcp)
{
	if (entering(tcp)) {
		/* arg 1: pid */
		tprintf("%ld, ", tcp->u_arg[0]);
		/* arg 2: local iov */
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else
			tprint_iov(tcp, tcp->u_arg[2], tcp->u_arg[1], 1);
		/* arg 3: local iovcnt */
		tprintf(", %lu, ", tcp->u_arg[2]);
		/* arg 4: remote iov */
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[3]);
		else
			tprint_iov(tcp, tcp->u_arg[4], tcp->u_arg[3], 0);
		/* arg 5: remote iovcnt */
		/* arg 6: flags */
		tprintf(", %lu, %lu", tcp->u_arg[4], tcp->u_arg[5]);
	}
	return 0;
}
