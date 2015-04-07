#include "defs.h"

#include <fcntl.h>

#include "xlat/sync_file_range_flags.h"

SYS_FUNC(sync_file_range)
{
	if (entering(tcp)) {
		int argn;
		printfd(tcp, tcp->u_arg[0]);
		argn = printllval(tcp, ", %lld, ", 1);
		argn = printllval(tcp, "%lld, ", argn);
		printflags(sync_file_range_flags, tcp->u_arg[argn],
		           "SYNC_FILE_RANGE_???");
	}
	return 0;
}

SYS_FUNC(sync_file_range2)
{
	if (entering(tcp)) {
		int argn;
		printfd(tcp, tcp->u_arg[0]);
		printflags(sync_file_range_flags, 1,
		           "SYNC_FILE_RANGE_???");
		argn = printllval(tcp, ", %lld, ", 2);
		argn = printllval(tcp, "%lld, ", argn);
	}
	return 0;
}
