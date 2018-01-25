/*
 * Check printing of file name in strace -y mode.
 *
 * Copyright (c) 2018 The strace developers.
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

#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>

int
main(void)
{
	char dir[PATH_MAX + 1];

	const struct {
		const char *path;
		const char *cstr;
		const char *fdstr;
	} checks[] = {
		{ ARG_STR("\1\0020\v\0047\f\58\t\79\n\10\0171\r\0167\218\37 \\\'\"<<0::0>>1~\177\200\377"),
			"\\1\\0020\\v\\0047\\f\\58\\t\\79\\n\\10\\0171\\r\\0167"
			"\\218\\37 \\\\\'\\\"\\74\\0740::0\\76\\0761~\\177\\200\\377" },
	};

	if (!getcwd(dir, sizeof(dir)))
		perror_msg_and_fail("getcwd");

	for (unsigned int i = 0; i < ARRAY_SIZE(checks); i++) {
		long fd = open(checks[i].path, O_RDONLY|O_CREAT, 0600);
		if (fd < 0)
			perror_msg_and_fail("open(%s)", checks[i].cstr);

		int rc = fsync(fd);

		printf("fsync(%ld<", fd);
		print_quoted_string_ex(dir, false, ">:");
		printf("/%s>) = %s\n", checks[i].fdstr, sprintrc(rc));

		close(fd);
	}

	puts("+++ exited with 0 +++");
	return 0;
}
