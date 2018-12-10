/*
 * Copyright (c) 2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include "error_prints.h"
#include "xstring.h"

#ifndef MAX_ADDR_LEN
# define MAX_ADDR_LEN 32
#endif

const char *
sprint_mac_addr(const uint8_t addr[], size_t size)
{
	static char res[MAX_ADDR_LEN * 3];

	if (size > MAX_ADDR_LEN) {
		error_func_msg("Address size (%zu) is more than maximum "
			       "supported (%u)", size, MAX_ADDR_LEN);

		return NULL;
	}

	char *ptr = res;

	for (size_t i = 0; i < size; i++)
		ptr = xappendstr(res, ptr, "%s%02x", i ? ":" : "", addr[i]);

	return res;
}
