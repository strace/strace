#include "negated_errno.h"

#ifndef ARCH_REGSET
# define ARCH_REGSET s390_regset
#endif

static void
get_error(struct tcb *tcp, const bool check_errno)
{
	if (check_errno && is_negated_errno(ARCH_REGSET.gprs[2])) {
		tcp->u_rval = -1;
		tcp->u_error = -ARCH_REGSET.gprs[2];
	} else {
		tcp->u_rval = ARCH_REGSET.gprs[2];
	}
}
