/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2017 The strace developers.
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
#include <asm/unistd.h>

#ifdef __NR_sendfile

# include <assert.h>
# include <errno.h>
# include <fcntl.h>
# include <stdio.h>
# include <stdint.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/socket.h>

int
main(void)
{
	(void) close(0);
	if (open("/dev/zero", O_RDONLY) != 0)
		perror_msg_and_skip("open: %s", "/dev/zero");

	int sv[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv))
		perror_msg_and_skip("socketpair");

	const unsigned int page_size = get_page_size();
	assert(syscall(__NR_sendfile, 0, 1, NULL, page_size) == -1);
	if (EBADF != errno)
		perror_msg_and_skip("sendfile");
	printf("sendfile(0, 1, NULL, %u) = -1 EBADF (%m)\n", page_size);

	unsigned int file_size = 0;
	socklen_t optlen = sizeof(file_size);
	if (getsockopt(sv[1], SOL_SOCKET, SO_SNDBUF, &file_size, &optlen))
		perror_msg_and_fail("getsockopt");
	if (file_size < 1024)
		error_msg_and_skip("SO_SNDBUF too small: %u", file_size);

	file_size /= 4;
	if (file_size / 16 > page_size)
		file_size = page_size * 16;
	const unsigned int blen = file_size / 3;
	const unsigned int alen = file_size - blen;

	static const char fname[] = "sendfile-tmpfile";
	int reg_in = open(fname, O_RDWR | O_CREAT | O_TRUNC, 0600);
	if (reg_in < 0)
		perror_msg_and_fail("open: %s", fname);
	if (unlink(fname))
		perror_msg_and_fail("unlink: %s", fname);
	if (ftruncate(reg_in, file_size))
		perror_msg_and_fail("ftruncate: %s", fname);

	TAIL_ALLOC_OBJECT_VAR_PTR(uint32_t, p_off);
	void *p = p_off + 1;
	*p_off = 0;

	assert(syscall(__NR_sendfile, 0, 1, p, page_size) == -1);
	printf("sendfile(0, 1, %p, %u) = -1 EFAULT (%m)\n", p, page_size);

	assert(syscall(__NR_sendfile, sv[1], reg_in, NULL, alen)
	       == (long) alen);
	printf("sendfile(%d, %d, NULL, %u) = %u\n",
	       sv[1], reg_in, alen, alen);

	p = p_off;
	if (syscall(__NR_sendfile, sv[1], reg_in, p_off, alen) != (long) alen) {
		printf("sendfile(%d, %d, %#lx, %u) = -1 EFAULT (%m)\n",
		       sv[1], reg_in, (unsigned long) p_off, alen);
		--p_off;
		*p_off = 0;
		assert(syscall(__NR_sendfile, sv[1], reg_in, p_off, alen)
		       == (long) alen);
	}
	printf("sendfile(%d, %d, [0] => [%u], %u) = %u\n",
	       sv[1], reg_in, alen, alen, alen);

	assert(syscall(__NR_sendfile, sv[1], reg_in, p_off, file_size + 1)
	       == (long) blen);
	printf("sendfile(%d, %d, [%u] => [%u], %u) = %u\n",
	       sv[1], reg_in, alen, file_size, file_size + 1, blen);

	if (p_off != p) {
		uint64_t *p_off64 = (uint64_t *) p_off;
		*p_off64 = 0xcafef00dfacefeedULL;
		assert(syscall(__NR_sendfile, sv[1], reg_in, p_off64, 1) == -1);
		printf("sendfile(%d, %d, [14627392582579060461], 1)"
		       " = -1 EINVAL (%m)\n", sv[1], reg_in);
		*p_off64 = 0xdefaced;
	} else {
		*p_off = 0xdefaced;
	}
	assert(syscall(__NR_sendfile, sv[1], reg_in, p_off, 1) == 0);
	printf("sendfile(%d, %d, [233811181], 1) = 0\n",
	       sv[1], reg_in);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_sendfile")

#endif
