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

#ifdef __NR_getdents

# include <assert.h>
# include <dirent.h>
# include <fcntl.h>
# include <stddef.h>
# include <stdio.h>
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

static char buf[8192];

static const char *
str_d_type(const unsigned char d_type)
{
	switch (d_type) {
		case DT_DIR:
			return "DT_DIR";
		case DT_REG:
			return "DT_REG";
		default:
			return "DT_UNKNOWN";
	}
}
static void
print_dirent(const kernel_dirent *d)
{
	const unsigned int d_name_offset = offsetof(kernel_dirent, d_name);
	int d_name_len = d->d_reclen - d_name_offset - 1;
	assert(d_name_len > 0);

	printf("{d_ino=%llu, d_off=%llu, d_reclen=%u, d_name=",
	       (unsigned long long) d->d_ino,
	       (unsigned long long) d->d_off, d->d_reclen);

	if (d->d_name[0] == '.')
		printf("\"%.*s\"", d_name_len, d->d_name);
	else
		printf("\"%s\"", qname);

	printf(", d_type=%s}",
	       str_d_type(*((const char *) d + d->d_reclen - 1)));
}

int
main(void)
{
	static const char dname[] = "getdents.test.tmp.dir";

	assert(!mkdir(dname, 0700));
	assert(!chdir(dname));
	(void) close(0);
	assert(!creat(fname, 0600));
	assert(!close(0));
	assert(!open(".", O_RDONLY | O_DIRECTORY));

	unsigned long count = (unsigned long) 0xfacefeeddeadbeefULL;
	long rc = syscall(__NR_getdents, (long) 0xdefacedffffffffULL, NULL,
			  count);
	printf("getdents(-1, NULL, %u) = %ld %s (%m)\n",
	       (unsigned) count, rc, errno2name());

	count = (unsigned long) 0xfacefeed00000000ULL | sizeof(buf);
	while ((rc = syscall(__NR_getdents, 0, buf, count))) {
		kernel_dirent *d;
		long i;

		if (rc < 0)
			perror_msg_and_skip("getdents");
		printf("getdents(0, [");
		for (i = 0; i < rc; i += d->d_reclen) {
			d = (kernel_dirent *) &buf[i];
			if (i)
				printf(", ");
			print_dirent(d);
		}
		printf("], %u) = %ld\n", (unsigned) count, rc);
	}
	printf("getdents(0, [], %u) = 0\n", (unsigned) count);
	puts("+++ exited with 0 +++");
	assert(!unlink(fname));
	assert(!chdir(".."));
	assert(!rmdir(dname));

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_getdents")

#endif
