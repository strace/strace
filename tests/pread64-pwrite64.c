/*
 * Check decoding of pread64 and pwrite64 syscalls.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void
dump_str(const char *str, const unsigned int len)
{
	static const char dots[16] = "................";

	for (unsigned int i = 0; i < len; i += 16) {
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

	for (unsigned int i = 0; i < len; ++i) {
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

	const off_t offset = 0xdefaceddeadbeefLL + len;
	long rc = pread(0, buf, len, offset);
	if (rc != (int) len)
		perror_msg_and_fail("pread64: expected %d, returned %ld",
				    len, rc);

	tprintf("%s(%d, \"", "pread64", 0);
	print_hex(buf, len);
	tprintf("\", %d, %lld) = %ld\n", len, (long long) offset, rc);
	dump_str(buf, len);

	for (unsigned int i = 0; i < len; ++i)
		buf[i] = i;

	rc = pwrite(1, buf, len, offset);
	if (rc != (int) len)
		perror_msg_and_fail("pwrite64: expected %d, returned %ld",
				    len, rc);

	tprintf("%s(%d, \"", "pwrite64", 1);
	print_hex(buf, len);
	tprintf("\", %d, %lld) = %ld\n", len, (long long) offset, rc);
	dump_str(buf, len);

	if (!len)
		buf = 0;
}

int
main(void)
{
	tprintf("%s", "");

	skip_if_unavailable("/proc/self/fd/");

	static const char tmp[] = "pread64-pwrite64-tmpfile";
	if (open(tmp, O_CREAT|O_RDONLY|O_TRUNC, 0600) != 0)
		perror_msg_and_fail("creat: %s", tmp);
	if (open(tmp, O_WRONLY) != 1)
		perror_msg_and_fail("open: %s", tmp);

	char *nil = tail_alloc(1);
	*nil = '\0';

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

	rc = pwrite(1, w, 0, 0);
	if (rc)
		perror_msg_and_fail("pwrite64: expected 0, returned %ld", rc);
	tprintf("pwrite64(1, \"\", 0, 0) = 0\n");

	rc = pwrite(1, efault, 1, 0);
	if (rc != -1)
		perror_msg_and_fail("pwrite64: expected -1 EFAULT"
				    ", returned %ld", rc);
	tprintf("pwrite64(1, %p, 1, 0) = -1 EFAULT (%m)\n", efault);

	rc = pwrite(1, nil, 1, -3);
	if (rc != -1)
		perror_msg_and_fail("pwrite64: expected -1, returned %ld", rc);
	tprintf("pwrite64(1, \"\\0\", 1, -3) = -1 EINVAL (%m)\n");
	dump_str(nil, 1);

	rc = pwrite(1, w, w_len, 0);
	if (rc != (int) w_len)
		perror_msg_and_fail("pwrite64: expected %u, returned %ld",
				    w_len, rc);
	tprintf("pwrite64(1, \"%s\", %u, 0) = %ld\n"
		" | 00000 %-49s  %-16s |\n",
		w_c, w_len, rc, w_d, w_c);
	close(1);

	rc = pread(0, r0, 0, 0);
	if (rc)
		perror_msg_and_fail("pread64: expected 0, returned %ld", rc);
	tprintf("pread64(0, \"\", 0, 0) = 0\n");

	rc = pread(0, efault, 1, 0);
	if (rc != -1)
		perror_msg_and_fail("pread64: expected -1, returned %ld", rc);
	tprintf("pread64(0, %p, 1, 0) = -1 EFAULT (%m)\n", efault);

	rc = pread(0, efault, 2, -7);
	if (rc != -1)
		perror_msg_and_fail("pread64: expected -1, returned %ld", rc);
	tprintf("pread64(0, %p, 2, -7) = -1 EINVAL (%m)\n", efault);

	rc = pread(0, r0, r0_len, 0);
	if (rc != (int) r0_len)
		perror_msg_and_fail("pread64: expected %u, returned %ld",
				    r0_len, rc);
	tprintf("pread64(0, \"%s\", %u, 0) = %ld\n"
		" | 00000 %-49s  %-16s |\n",
		r0_c, r0_len, rc, r0_d, r0_c);

	rc = pread(0, r1, w_len, r0_len);
	if (rc != (int) r1_len)
		perror_msg_and_fail("pread64: expected %u, returned %ld",
				    r1_len, rc);
	tprintf("pread64(0, \"%s\", %u, %u) = %ld\n"
		" | 00000 %-49s  %-16s |\n",
		r1_c, w_len, r0_len, rc, r1_d, r1_c);
	close(0);

	if (open("/dev/zero", O_RDONLY))
		perror_msg_and_fail("open");

	if (open("/dev/null", O_WRONLY) != 1)
		perror_msg_and_fail("open");

	for (unsigned int i = 0; i <= 32; ++i)
		test_dump(i);

	tprintf("+++ exited with 0 +++\n");
	return 0;
}
