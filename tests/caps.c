/*
 * This file is part of caps strace test.
 *
 * Copyright (c) 2014-2016 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
