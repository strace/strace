#include "defs.h"

#include <fcntl.h>

#include "xlat/advise.h"

SYS_FUNC(fadvise64)
{
	if (entering(tcp)) {
		int argn;
		printfd(tcp, tcp->u_arg[0]);
		argn = printllval(tcp, ", %lld", 1);
		tprintf(", %ld, ", tcp->u_arg[argn++]);
		printxval(advise, tcp->u_arg[argn], "POSIX_FADV_???");
	}
	return 0;
}

SYS_FUNC(fadvise64_64)
{
	if (entering(tcp)) {
		int argn;
		printfd(tcp, tcp->u_arg[0]);
		argn = printllval(tcp, ", %lld, ", 1);
		argn = printllval(tcp, "%lld, ", argn);
#if defined __ARM_EABI__ || defined AARCH64 || defined POWERPC || defined XTENSA
		printxval(advise, tcp->u_arg[1], "POSIX_FADV_???");
#else
		printxval(advise, tcp->u_arg[argn], "POSIX_FADV_???");
#endif
	}
	return 0;
}
