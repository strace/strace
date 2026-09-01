/*
 * Hyperlink support.
 *
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "hyperlink.h"

#include <string.h>

bool hyperlink_is_enabled = false;

/*
 * Build a manual page URL on man7.org for a given system call name.
 * Returns dynamically allocated string, or NULL if not applicable.
 */
char *
make_hyperlink_url_for_syscall(const char *name)
{
	if (!name || *name == '?' || !strcmp(name, "system call"))
		return NULL;

	/*
	 * Strip architecture/ABI prefixes (such as "n32:", "n64:", "o32:"
	 * on MIPS) when constructing the manual page URL.
	 */
	const char *colon = strchr(name, ':');
	const char *base_name = colon ? colon + 1 : name;

	/*
	 * Strip variant suffixes (such as "#64" on x32) when constructing
	 * the manual page URL.
	 */
	const char *hash = strchr(base_name, '#');
	size_t len = hash ? (size_t) (hash - base_name) : strlen(base_name);

	return xasprintf("https://man7.org/linux/man-pages/man2/%.*s.2.html",
			 (int) len, base_name);
}
