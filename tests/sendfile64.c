/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>

#ifdef __NR_sendfile64

int
main(int ac, const char **av)
{
	assert(ac == 2);

	(void) close(0);
	if (open("/dev/zero", O_RDONLY) != 0)
		return 77;

	int sv[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv))
		return 77;

	int reg_in = open(av[1], O_RDONLY);
	if (reg_in < 0)
		return 77;

	struct stat stb;
	if (fstat(reg_in, &stb))
		return 77;
	const size_t blen = stb.st_size / 3;
	const size_t alen = stb.st_size - blen;
	assert(S_ISREG(stb.st_mode) && blen > 0);

	const size_t page_len = sysconf(_SC_PAGESIZE);
	if (!syscall(__NR_sendfile64, 0, 1, NULL, page_len) ||
	    EBADF != errno)
		return 77;
	printf("sendfile64(0, 1, NULL, %lu) = -1 EBADF (Bad file descriptor)\n",
	       (unsigned long) page_len);

	void *p = mmap(NULL, page_len * 2, PROT_READ | PROT_WRITE,
		       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (MAP_FAILED == p || munmap(p + page_len, page_len))
		return 77;

	if (!syscall(__NR_sendfile64, 0, 1, p + page_len, page_len))
		return 77;
	printf("sendfile64(0, 1, %#lx, %lu) = -1 EFAULT (Bad address)\n",
	       (unsigned long) p + page_len, (unsigned long) page_len);

	if (syscall(__NR_sendfile64, sv[1], reg_in, NULL, alen) != (long) alen)
		return 77;
	printf("sendfile64(%d, %d, NULL, %lu) = %lu\n",
	       sv[1], reg_in, (unsigned long) alen,
	       (unsigned long) alen);

	uint64_t *p_off = p + page_len - sizeof(uint64_t);
	if (syscall(__NR_sendfile64, sv[1], reg_in, p_off, alen) != (long) alen)
		return 77;
	printf("sendfile64(%d, %d, [0] => [%lu], %lu) = %lu\n",
	       sv[1], reg_in, (unsigned long) alen,
	       (unsigned long) alen, (unsigned long) alen);

	if (syscall(__NR_sendfile64, sv[1], reg_in, p_off, stb.st_size + 1)
	    != (long) blen)
		return 77;
	printf("sendfile64(%d, %d, [%lu] => [%lu], %lu) = %lu\n",
	       sv[1], reg_in, (unsigned long) alen,
	       (unsigned long) stb.st_size,
	       (unsigned long) stb.st_size + 1,
	       (unsigned long) blen);

	*p_off = 0xcafef00dfacefeed;
	if (!syscall(__NR_sendfile64, sv[1], reg_in, p_off, 1))
		return 77;
	printf("sendfile64(%d, %d, [14627392582579060461], 1)"
		" = -1 EINVAL (Invalid argument)\n",
	       sv[1], reg_in);

	*p_off = 0xfacefeed;
	if (syscall(__NR_sendfile64, sv[1], reg_in, p_off, 1))
		return 77;
	printf("sendfile64(%d, %d, [4207869677], 1) = 0\n",
	       sv[1], reg_in);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

int
main(void)
{
	return 77;
}

#endif
