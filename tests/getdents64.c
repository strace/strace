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

#ifdef __NR_getdents64

# include <assert.h>
# include <dirent.h>
# include <fcntl.h>
# include <inttypes.h>
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

typedef struct {
		uint64_t d_ino;
		uint64_t d_off;
		unsigned short d_reclen;
		unsigned char d_type;
		char d_name[256];
} kernel_dirent64;

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
print_dirent(const kernel_dirent64 *d)
{
	const unsigned int d_name_offset = offsetof(kernel_dirent64, d_name);
	int d_name_len = d->d_reclen - d_name_offset;
	assert(d_name_len > 0);

	printf("{d_ino=%" PRIu64 ", d_off=%" PRId64
	       ", d_reclen=%u, d_type=%s, d_name=",
	       d->d_ino, d->d_off, d->d_reclen, str_d_type(d->d_type));

	if (d->d_name[0] == '.')
		printf("\"%.*s\"}", d_name_len, d->d_name);
	else
		printf("\"%s\"}", qname);
}

int
main(void)
{
	static const char dname[] = "getdents64.test.tmp.dir";

	assert(!mkdir(dname, 0700));
	assert(!chdir(dname));
	(void) close(0);
	assert(!creat(fname, 0600));
	assert(!close(0));
	assert(!open(".", O_RDONLY | O_DIRECTORY));

	unsigned long count = (unsigned long) 0xfacefeeddeadbeefULL;
	long rc = syscall(__NR_getdents64, (long) 0xdefacedffffffffULL, NULL,
			  count);
	printf("getdents64(-1, NULL, %u) = %ld %s (%m)\n",
	       (unsigned) count, rc, errno2name());

	count = (unsigned long) 0xfacefeed00000000ULL | sizeof(buf);
	while ((rc = syscall(__NR_getdents64, 0, buf, count))) {
		kernel_dirent64 *d;
		long i;

		if (rc < 0)
			perror_msg_and_skip("getdents64");
		printf("getdents64(0, [");
		for (i = 0; i < rc; i += d->d_reclen) {
			d = (kernel_dirent64 *) &buf[i];
			if (i)
				printf(", ");
			print_dirent(d);
		}
		printf("], %u) = %ld\n", (unsigned) count, rc);
	}
	printf("getdents64(0, [], %u) = 0\n", (unsigned) count);
	puts("+++ exited with 0 +++");
	assert(!unlink(fname));
	assert(!chdir(".."));
	assert(!rmdir(dname));

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_getdents64")

#endif
