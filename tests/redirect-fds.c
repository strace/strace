/*
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
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
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>

#define N_FDS 3

/*
 * Do not print any messages, indicate errors with return codes.
 */
static int
check_fd(int fd, const char *fname)
{
	const int should_be_closed = (fname[0] == '\0');

	struct stat st_fd, st_fn;

	if (fstat(fd, &st_fd)) {
		if (!should_be_closed)
			return 10 + fd;
	} else {
		if (should_be_closed)
			return 20 + fd;

		if (stat(fname, &st_fn))
			return 30 + fd;

		if (st_fd.st_dev != st_fn.st_dev
		    || st_fd.st_ino != st_fn.st_ino)
			return 40 + fd;
	}

	return 0;
}

int
main(int ac, char **av)
{
	assert(ac == 1 + N_FDS);

	int rc = 0, fd;
	for (fd = 1; fd < 1 + N_FDS; ++fd)
		if ((rc = check_fd(fd - 1, av[fd])))
			break;

	return rc;
}
