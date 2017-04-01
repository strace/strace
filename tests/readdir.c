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

#ifdef __NR_readdir

# include <assert.h>
# include <dirent.h>
# include <fcntl.h>
# include <stdio.h>
# include <string.h>
# include <sys/stat.h>
# include <unistd.h>

static const char fname[] =
	"A\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\n"
	"A\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\n"
	"A\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\n"
	"A\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\n"
	"A\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\n"
	"A\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\n"
	"A\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\n"
	"A\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nZ";
static const char qname[] =
	"A\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\n"
	"A\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\n"
	"A\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\n"
	"A\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\n"
	"A\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\n"
	"A\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\n"
	"A\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\n"
	"A\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nA\\nZ";

int
main(void)
{
	static const char dname[] = "readdir.test.tmp.dir";
	struct {
		unsigned long   d_ino;
		unsigned long   d_off;
		unsigned short  d_reclen;
		char d_name[1024];
	} e;
	int rc;

	assert(!mkdir(dname, 0700));
	assert(!chdir(dname));
	(void) close(0);
	assert(!creat(fname, 0600));
	assert(!close(0));
	assert(!open(".", O_RDONLY | O_DIRECTORY));
	while ((rc = syscall(__NR_readdir, 0, &e, 1))) {
		if (rc < 0)
			perror_msg_and_skip("readdir");
		e.d_name[e.d_reclen] = '\0';
		printf("readdir(0, {d_ino=%lu, d_off=%lu, d_reclen=%u"
		       ", d_name=\"%s\"}) = %d\n",
		       e.d_ino, e.d_off, e.d_reclen,
		       e.d_name[0] == '.' ? e.d_name : qname, rc);
	}
	printf("readdir(0, %p) = 0\n", &e);
	puts("+++ exited with 0 +++");
	assert(!unlink(fname));
	assert(!chdir(".."));
	assert(!rmdir(dname));

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_readdir")

#endif
