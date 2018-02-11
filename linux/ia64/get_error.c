#include "negated_errno.h"

static void
get_error(struct tcb *tcp, const bool check_errno)
{
	if (ia64_regs.gr[10]) {
		tcp->u_rval = -1;
		tcp->u_error = ia64_regs.gr[8];
	} else {
		tcp->u_rval = ia64_regs.gr[8];
	}
}
