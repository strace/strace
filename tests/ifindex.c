/*
 * Proxy wrappers for if_nametoindex.
 *
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#ifdef HAVE_IF_INDEXTONAME

# include <net/if.h>

unsigned int
ifindex_lo(void)
{
	static unsigned int index;

	if (!index)
		index = if_nametoindex("lo");

	return index;
}

#else /* !HAVE_IF_INDEXTONAME */

unsigned int
ifindex_lo(void)
{
	return 1;
}

#endif
