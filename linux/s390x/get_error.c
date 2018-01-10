#include "negated_errno.h"

#define get_error s390_get_error
#define ARCH_REGSET s390_regset
#include "../s390/get_error.c"
#undef ARCH_REGSET
#undef get_error

#define get_error s390x_get_error
#define ARCH_REGSET s390x_regset
#include "../s390/get_error.c"
#undef ARCH_REGSET
#undef get_error

static void
get_error(struct tcb *tcp, const bool check_errno)
{
	if (tcp->currpers == 1)
		s390_get_error(tcp, check_errno);
	else
		s390x_get_error(tcp, check_errno);
}
