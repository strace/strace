/*
 * Check decoding and dumping of read and write syscalls.
 *
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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <asm/unistd.h>

static void
dump_str(const char *str, const unsigned int len)
{
	static const char dots[16] = "................";
	unsigned int i;

	for (i = 0; i < len; i += 16) {
		unsigned int n = len - i > 16 ? 16 : len - i;
		const char *dump = hexdump_memdup(str + i, n);

		tprintf(" | %05x %-49s  %-16.*s |\n",
			i, dump, n, dots);

		free((void *) dump);
	}
}

static void
print_hex(const char *str, const unsigned int len)
{
	const unsigned char *ustr = (const unsigned char *) str;
	unsigned int i;

	for (i = 0; i < len; ++i) {
		unsigned int c = ustr[i];

		switch (c) {
		case '\t':
			tprintf("\\t"); break;
		case '\n':
			tprintf("\\n"); break;
		case '\v':
			tprintf("\\v"); break;
		case '\f':
			tprintf("\\f"); break;
		case '\r':
			tprintf("\\r"); break;
		default:
			tprintf("\\%o", ustr[i]);
		}
	}
}

static long
k_read(unsigned int fd, void *buf, size_t count)
{
	kernel_ulong_t kfd = (kernel_ulong_t) 0xfacefeed00000000ULL | fd;
	return syscall(__NR_read, kfd, buf, count);
}

static long
k_write(unsigned int fd, const void *buf, size_t count)
{
	kernel_ulong_t kfd = (kernel_ulong_t) 0xfacefeed00000000ULL | fd;
	return syscall(__NR_write, kfd, buf, count);
}

static void
test_dump(const unsigned int len)
{
	static char *buf;

	if (buf) {
		size_t ps1 = get_page_size() - 1;
		buf = (void *) (((size_t) buf + ps1) & ~ps1) - len;
	} else {
		buf = tail_alloc(len);
	}

	long rc = k_read(0, buf, len);
	if (rc != (int) len)
		perror_msg_and_fail("read: expected %d, returned %ld",
				    len, rc);

	tprintf("%s(%d, \"", "read", 0);
	print_hex(buf, len);
	tprintf("\", %d) = %ld\n", len, rc);
	dump_str(buf, len);

	unsigned int i;
	for (i = 0; i < len; ++i)
		buf[i] = i;

	rc = k_write(1, buf, len);
	if (rc != (int) len)
		perror_msg_and_fail("write: expected %d, returned %ld",
				    len, rc);

	tprintf("%s(%d, \"", "write", 1);
	print_hex(buf, len);
	tprintf("\", %d) = %ld\n", len, rc);
	dump_str(buf, len);

	if (!len)
		buf = 0;
}

int
main(void)
{
	tprintf("%s", "");

	skip_if_unavailable("/proc/self/fd/");

	static const char tmp[] = "read-write-tmpfile";
	if (open(tmp, O_CREAT|O_RDONLY|O_TRUNC, 0600) != 0)
		perror_msg_and_fail("creat: %s", tmp);
	if (open(tmp, O_WRONLY) != 1)
		perror_msg_and_fail("open: %s", tmp);

	static const char w_c[] = "0123456789abcde";
	const unsigned int w_len = LENGTH_OF(w_c);
	const char *w_d = hexdump_strdup(w_c);
	const void *w = tail_memdup(w_c, w_len);

	static const char r0_c[] = "01234567";
	const char *r0_d = hexdump_strdup(r0_c);
	const unsigned int r0_len = (w_len + 1) / 2;
	void *r0 = tail_alloc(r0_len);

	static const char r1_c[] = "89abcde";
	const char *r1_d = hexdump_strdup(r1_c);
	const unsigned int r1_len = w_len - r0_len;
	void *r1 = tail_alloc(w_len);

	void *efault = r1 - get_page_size();

	long rc;

	rc = k_write(1, w, 0);
	if (rc)
		perror_msg_and_fail("write: expected 0, returned %ld", rc);
	tprintf("write(1, \"\", 0) = 0\n");

	rc = k_write(1, efault, 1);
	if (rc != -1)
		perror_msg_and_fail("write: expected -1 EFAULT"
				    ", returned %ld", rc);
	tprintf("write(1, %p, 1) = -1 EFAULT (%m)\n", efault);

	rc = k_write(1, w, w_len);
	if (rc != (int) w_len)
		perror_msg_and_fail("write: expected %u, returned %ld",
				    w_len, rc);
	tprintf("write(1, \"%s\", %u) = %ld\n"
		" | 00000 %-49s  %-16s |\n",
		w_c, w_len, rc, w_d, w_c);
	close(1);

	rc = k_read(0, r0, 0);
	if (rc)
		perror_msg_and_fail("read: expected 0, returned %ld", rc);
	tprintf("read(0, \"\", 0) = 0\n");

	rc = k_read(0, efault, 1);
	if (rc != -1)
		perror_msg_and_fail("read: expected -1, returned %ld", rc);
	tprintf("read(0, %p, 1) = -1 EFAULT (%m)\n", efault);

	rc = k_read(0, r0, r0_len);
	if (rc != (int) r0_len)
		perror_msg_and_fail("read: expected %u, returned %ld",
				    r0_len, rc);
	tprintf("read(0, \"%s\", %u) = %ld\n"
		" | 00000 %-49s  %-16s |\n",
		r0_c, r0_len, rc, r0_d, r0_c);

	rc = k_read(0, r1, w_len);
	if (rc != (int) r1_len)
		perror_msg_and_fail("read: expected %u, returned %ld",
				    r1_len, rc);
	tprintf("read(0, \"%s\", %u) = %ld\n"
		" | 00000 %-49s  %-16s |\n",
		r1_c, w_len, rc, r1_d, r1_c);
	close(0);

	if (open("/dev/zero", O_RDONLY))
		perror_msg_and_fail("open");

	if (open("/dev/null", O_WRONLY) != 1)
		perror_msg_and_fail("open");

	unsigned int i;
	for (i = 0; i <= 32; ++i)
		test_dump(i);

	tprintf("+++ exited with 0 +++\n");
	return 0;
}
