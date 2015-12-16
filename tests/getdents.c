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

#include <sys/syscall.h>

#ifdef __NR_getdents

#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "kernel_types.h"

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
main(int ac, const char **av)
{
	char *dname;
	int rc;

	assert(ac == 1);
	assert(asprintf(&dname, "%s.test.tmp.dir", av[0]) > 0);
	assert(!mkdir(dname, 0700));
	assert(!chdir(dname));
	(void) close(0);
	assert(!creat(fname, 0600));
	assert(!close(0));
	assert(!open(".", O_RDONLY | O_DIRECTORY));
	while ((rc = syscall(__NR_getdents, 0, buf, sizeof(buf)))) {
		kernel_dirent *d;
		int i;

		if (rc < 0)
			return 77;
		printf("getdents(0, [");
		for (i = 0; i < rc; i += d->d_reclen) {
			d = (kernel_dirent *) &buf[i];
			if (i)
				printf(", ");
			print_dirent(d);
		}
		printf("], %zu) = %d\n", sizeof(buf), rc);
	}
	printf("getdents(0, [], %zu) = 0\n", sizeof(buf));
	puts("+++ exited with 0 +++");
	assert(!unlink(fname));
	assert(!chdir(".."));
	assert(!rmdir(dname));

	return 0;
}

#else

int
main(void)
{
	return 77;
}

#endif
