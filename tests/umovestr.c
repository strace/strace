/*
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <string.h>
#include <unistd.h>

int
main(void)
{
	const size_t tail_len = 257;
	char *addr = tail_alloc(tail_len);
	memset(addr, '/', tail_len - 1);
	addr[tail_len - 1] = '\0';
	if (chdir(addr))
		perror_msg_and_skip("chdir");
	return 0;
}
