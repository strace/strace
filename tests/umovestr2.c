/*
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int
main(void)
{
	const size_t page_len = get_page_size();
	const size_t tail_len = page_len * 2 - 1;
	const size_t str_len = tail_len - 1;

	char *addr = tail_alloc(tail_len);
	memset(addr, '0', str_len);
	addr[str_len] = '\0';

	char *argv[] = { NULL };
	char *envp[] = { addr, NULL };
	execve("", argv, envp);

	printf("execve(\"\", [], [\"%0*u\"]) = -1 ENOENT (%m)\n",
	       (int) str_len, 0);
	puts("+++ exited with 0 +++");

	return 0;
}
