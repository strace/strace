#include "defs.h"
#include <fcntl.h>
#ifdef HAVE_SYS_SIGNALFD_H
# include <sys/signalfd.h>
#endif

#include "xlat/sfd_flags.h"

static int
do_signalfd(struct tcb *tcp, int flags_arg)
{
	/* NB: kernel requires arg[2] == NSIG / 8 */
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	print_sigset_addr_len(tcp, tcp->u_arg[1], tcp->u_arg[2]);
	tprintf(", %lu", tcp->u_arg[2]);
	if (flags_arg >= 0) {
		tprints(", ");
		printflags(sfd_flags, tcp->u_arg[flags_arg], "SFD_???");
	}

	return RVAL_DECODED | RVAL_FD;
}

SYS_FUNC(signalfd)
{
	return do_signalfd(tcp, -1);
}

SYS_FUNC(signalfd4)
{
	return do_signalfd(tcp, 3);
}
