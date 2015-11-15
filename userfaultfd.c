#include "defs.h"
#include <fcntl.h>

#include "xlat/uffd_flags.h"

SYS_FUNC(userfaultfd)
{
	printflags(uffd_flags, tcp->u_arg[0], "UFFD_???");

	return RVAL_DECODED | RVAL_FD;
}
