/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
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
# include <unistd.h>
# include <sys/socket.h>
# include <sys/stat.h>

int
main(int ac, const char **av)
{
	assert(ac == 1);

	(void) close(0);
	if (open("/dev/zero", O_RDONLY) != 0)
		perror_msg_and_skip("open: %s", "/dev/zero");

	int sv[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv))
		perror_msg_and_skip("socketpair");

	int reg_in = open(av[0], O_RDONLY);
	if (reg_in < 0)
		perror_msg_and_fail("open: %s", av[0]);

	struct stat stb;
	assert(fstat(reg_in, &stb) == 0);
	const size_t blen = stb.st_size / 3;
	const size_t alen = stb.st_size - blen;
	assert(S_ISREG(stb.st_mode) && blen > 0);

	const size_t page_len = get_page_size();
	assert(syscall(__NR_sendfile, 0, 1, NULL, page_len) == -1);
	if (EBADF != errno)
		perror_msg_and_skip("sendfile");
	printf("sendfile(0, 1, NULL, %lu) = -1 EBADF (%m)\n",
	       (unsigned long) page_len);

	TAIL_ALLOC_OBJECT_VAR_PTR(uint32_t, p_off);
	void *p = p_off + 1;
	*p_off = 0;

	assert(syscall(__NR_sendfile, 0, 1, p, page_len) == -1);
	printf("sendfile(0, 1, %#lx, %lu) = -1 EFAULT (%m)\n",
	       (unsigned long) p, (unsigned long) page_len);

	assert(syscall(__NR_sendfile, sv[1], reg_in, NULL, alen)
	       == (long) alen);
	printf("sendfile(%d, %d, NULL, %lu) = %lu\n",
	       sv[1], reg_in, (unsigned long) alen,
	       (unsigned long) alen);

	p = p_off;
	if (syscall(__NR_sendfile, sv[1], reg_in, p_off, alen) != (long) alen) {
		printf("sendfile(%d, %d, %#lx, %lu) = -1 EFAULT (%m)\n",
		       sv[1], reg_in, (unsigned long) p_off,
		       (unsigned long) alen);
		--p_off;
		*p_off = 0;
		assert(syscall(__NR_sendfile, sv[1], reg_in, p_off, alen)
		       == (long) alen);
	}
	printf("sendfile(%d, %d, [0] => [%lu], %lu) = %lu\n",
	       sv[1], reg_in, (unsigned long) alen,
	       (unsigned long) alen, (unsigned long) alen);

	assert(syscall(__NR_sendfile, sv[1], reg_in, p_off, stb.st_size + 1)
	       == (long) blen);
	printf("sendfile(%d, %d, [%lu] => [%lu], %lu) = %lu\n",
	       sv[1], reg_in, (unsigned long) alen,
	       (unsigned long) stb.st_size,
	       (unsigned long) stb.st_size + 1,
	       (unsigned long) blen);

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
