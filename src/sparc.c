/*
 * SPARC-specific syscall decoders.
 *
 * Copyright (c) 2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#if defined SPARC || defined SPARC64

# include "xlat/sparc_kern_features.h"

SYS_FUNC(kern_features)
{
	if (entering(tcp) || syserror(tcp))
		return 0;

	tcp->auxstr = sprintflags("", sparc_kern_features,
				  (kernel_ulong_t) tcp->u_rval);
	return RVAL_HEX | RVAL_STR;
}

#endif /* SPARC || SPARC64 */
