/*
 * This file is part of caps strace test.
 *
 * Copyright (c) 2014-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <errno.h>
#include <string.h>

extern int capget(int *, int *);
extern int capset(int *, const int *);

int
main(void)
{
	const int data[] = { 2, 4, 0, 8, 16, 0 };
	const int v1 = 0x19980330;
	const int v2 = 0x20071026;
	const int v3 = 0x20080522;

	int * const head = tail_alloc(sizeof(int) * 2);
	head[0] = v3;
	head[1] = 0;
	int * const tail_data = tail_alloc(sizeof(data));

	capget(NULL, NULL);
	capget(head + 2, tail_data);
	capget(head, tail_data + ARRAY_SIZE(data));

	if (capget(head, tail_data))
		perror_msg_and_skip("capget");
	if (head[0] != v3)
		error_msg_and_skip("capget: v3 expected");

	memcpy(tail_data, data, sizeof(data));

	capset(NULL, NULL);
	capset(head + 2, tail_data);

	head[0] = 0xbadc0ded;
	head[1] = 2718281828U;
	capset(head, tail_data + ARRAY_SIZE(data) - 2);

	head[0] = v2;
	head[1] = 0;
	capset(head, tail_data + ARRAY_SIZE(data) - 5);

	memcpy(tail_data, data, sizeof(data));
	head[0] = v3;
	if (capset(head, tail_data) == 0 || errno != EPERM)
		perror_msg_and_skip("capset");

	memset(tail_data, 0, sizeof(data) / 2);
	if (capset(head, tail_data) == 0 || errno != EPERM)
		perror_msg_and_skip("capset");

	memcpy(tail_data + ARRAY_SIZE(data) / 2, data, sizeof(data) / 2);
	head[0] = v1;
	if (capset(head, tail_data + ARRAY_SIZE(data) / 2) == 0 ||
	    errno != EPERM)
		perror_msg_and_skip("capset");

	return 0;
}
