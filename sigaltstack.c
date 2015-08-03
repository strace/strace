#include "defs.h"

#include DEF_MPERS_TYPE(stack_t)

#include <signal.h>

#include MPERS_DEFS

#include "xlat/sigaltstack_flags.h"

static void
print_stack_t(struct tcb *tcp, unsigned long addr)
{
	stack_t ss;

	if (umove_or_printaddr(tcp, addr, &ss))
		return;

	tprints("{ss_sp=");
	printaddr((unsigned long) ss.ss_sp);
	tprints(", ss_flags=");
	printflags(sigaltstack_flags, ss.ss_flags, "SS_???");
	tprintf(", ss_size=%lu}", (unsigned long) ss.ss_size);
}

SYS_FUNC(sigaltstack)
{
	if (entering(tcp)) {
		print_stack_t(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		print_stack_t(tcp, tcp->u_arg[1]);
	}
	return 0;
}
