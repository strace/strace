/*
 * Copyright (c) 2015 Gleb Fotengauer-Malinovskiy <glebfm@altlinux.org>
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

#ifdef __NR_readlinkat

# include <stdio.h>
# include <unistd.h>

# define PREFIX "test.readlinkat"
# define TARGET (PREFIX ".target")
# define LINKPATH (PREFIX ".link")

int
main(void)
{
	const char * const fname = tail_memdup(LINKPATH, sizeof(LINKPATH));
	const char * const hex_fname =
		hexquote_strndup(fname, sizeof(LINKPATH) - 1);

	const unsigned int size = sizeof(TARGET) - 1;
	char * const buf = tail_alloc(size);

	(void) unlink(fname);

	long rc = syscall(__NR_readlinkat, -100, fname, buf, size);
	printf("readlinkat(AT_FDCWD, \"%s\", %p, %u) = -1 ENOENT (%m)\n",
	       hex_fname, buf, size);

	if (symlink(TARGET, fname))
		perror_msg_and_fail("symlink");

	rc = syscall(__NR_readlinkat, -100, fname, buf, size);
	if (rc < 0) {
		perror("readlinkat");
		(void) unlink(fname);
		return 77;
	}
	const char * const hex_buf = hexquote_strndup(buf, size);
	printf("readlinkat(AT_FDCWD, \"%s\", \"%s\", %u) = %u\n",
	       hex_fname, hex_buf, size, size);

	if (unlink(fname))
		perror_msg_and_fail("unlink");

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_readlink")

#endif
