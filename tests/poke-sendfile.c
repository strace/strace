/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>

int
main(void)
{
	int sv[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv))
		perror_msg_and_skip("socketpair");

	unsigned int file_size = 0;
	socklen_t optlen = sizeof(file_size);
	if (getsockopt(sv[1], SOL_SOCKET, SO_SNDBUF, &file_size, &optlen))
		perror_msg_and_fail("getsockopt");
	file_size /= 4;

	int reg_in = create_tmpfile(O_RDWR);
	if (ftruncate(reg_in, file_size))
		perror_msg_and_fail("ftruncate(%d, %u)", reg_in, file_size);

	TAIL_ALLOC_OBJECT_CONST_PTR(uint64_t, p_off);
	*p_off = 0xcafef00dfacefeedULL;

#ifdef __NR_sendfile64
	syscall(__NR_sendfile64, sv[1], reg_in, p_off, file_size);
	static const char name[] = "sendfile64";
	static const unsigned long long expected = -1ULL;
#else
	syscall(__NR_sendfile, sv[1], reg_in, p_off, file_size);
	static const char name[] = "sendfile";
	static const unsigned long long expected = (kernel_ulong_t) -1ULL;
#endif

	printf("%s(%d, %d, [0] => [%llu], %u) = %u (INJECTED: args)\n",
	       name, sv[1], reg_in, expected, file_size, file_size);

	puts("+++ exited with 0 +++");
	return 0;
}
