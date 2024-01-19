/*
 * Check decoding and dumping of read and write syscalls.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "scno.h"

static void
dump_str_ex(const char *str, const unsigned int len, const int idx_w)
{
	static const char chars[256] =
		"................................"
		" !\"#$%&'()*+,-./0123456789:;<=>?"
		"@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
		"`abcdefghijklmnopqrstuvwxyz{|}~."
		"................................"
		"................................"
		"................................"
		"................................";

	for (unsigned int i = 0; i < len; i += 16) {
		unsigned int n = len - i > 16 ? 16 : len - i;
		const char *dump = hexdump_memdup(str + i, n);

		tprintf(" | %0*x %-49s  %-16.*s |\n",
			idx_w, i, dump, n, chars + i % 0x100);

		free((void *) dump);
	}
}

static void
dump_str(const char *str, const unsigned int len)
{
	dump_str_ex(str, len, 5);
}

static void
print_hex(const char *str, const unsigned int len)
{
	const unsigned char *ustr = (const unsigned char *) str;

	tprintf("\"");

	for (unsigned int i = 0; i < len; ++i) {
		unsigned int c = ustr[i];

		if (i >= DEFAULT_STRLEN) {
			tprintf("\"...");
			return;
		}

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

	tprintf("\"");
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
test_dump(const unsigned int len, bool err_desc)
{
	static char *buf;
	const char *rc_str;
	int in_fd = err_desc ? 5 : 0;
	int out_fd = err_desc ? 4 : 1;

	if (buf) {
		size_t ps1 = get_page_size() - 1;
		buf = (void *) (((size_t) buf + ps1) & ~ps1) - len;
	} else {
		buf = tail_alloc(len);
	}

	long rc = k_read(in_fd, buf, len);
	rc_str = sprintrc(rc);
	if (err_desc ^ (rc != (int) len))
		perror_msg_and_fail("read: expected %d, returned %ld",
				    err_desc ? -1 : (int) len, rc);

	tprintf("%s(%d, ", "read", in_fd);
	if (!err_desc)
		print_hex(buf, len);
	else
		tprintf("%p", buf);
	tprintf(", %d) = %s\n", len, rc_str);
	if (!err_desc)
		dump_str(buf, len);

	for (unsigned int i = 0; i < len; ++i)
		buf[i] = i;

	rc = k_write(out_fd, buf, len);
	rc_str = sprintrc(rc);
	if (err_desc ^ (rc != (int) len))
		perror_msg_and_fail("write: expected %d, returned %ld",
				    err_desc ? -1 : (int) len, rc);

	tprintf("%s(%d, ", "write", out_fd);
	print_hex(buf, len);
	tprintf(", %d) = %s\n", len, rc_str);
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
	bool need_cleanup = true;
	long rc;

	rc = open(tmp, O_RDONLY, 0600);
	if (rc < 0) {
		rc = open(tmp, O_CREAT|O_EXCL|O_RDONLY, 0600);
		need_cleanup = false;
	}
	if (rc != 0)
		perror_msg_and_fail("creat: %s", tmp);
	if (open(tmp, O_TRUNC|O_WRONLY) != 1)
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

	rc = k_write(1, w, 0);
	if (rc)
		perror_msg_and_fail("write: expected 0, returned %ld", rc);
	tprintf("write(1, \"\", 0) = 0\n");

	rc = k_write(1, efault, 1);
	if (rc != -1)
		perror_msg_and_fail("write: expected -1 EFAULT"
				    ", returned %ld", rc);
	tprintf("write(1, %p, 1)" RVAL_EFAULT, efault);

	rc = k_write(1, w, w_len);
	if (rc != (int) w_len)
		perror_msg_and_fail("write: expected %u, returned %ld",
				    w_len, rc);
	tprintf("write(1, \"%s\", %u) = %ld\n"
		" | 00000 %-49s  %-16s |\n",
		w_c, w_len, rc, w_d, w_c);

	rc = k_read(0, r0, 0);
	if (rc)
		perror_msg_and_fail("read: expected 0, returned %ld", rc);
	tprintf("read(0, \"\", 0) = 0\n");

	rc = k_read(0, efault, 1);
	if (rc != -1)
		perror_msg_and_fail("read: expected -1, returned %ld", rc);
	tprintf("read(0, %p, 1)" RVAL_EFAULT, efault);

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

	/*
	 * Check partial dump; relies on dumpstr() implementation details
	 * (maximum size of chunk to be copied at once).
	 */
	static const size_t six_wide_size = 1 << 20;
	static const size_t fetch_size = 1 << 16;
	static const char big_buf_str[] =
		"\\0\\1\\2\\3\\4\\5\\6\\7"
		"\\10\\t\\n\\v\\f\\r\\16\\17"
		"\\20\\21\\22\\23\\24\\25\\26\\27"
		"\\30\\31\\32\\33\\34\\35\\36\\37";
	const size_t buf_size = six_wide_size + fetch_size;
	const size_t sizes[] = {
		six_wide_size,
		six_wide_size + 1,
		buf_size,
		buf_size + 1,
		buf_size + 2,
	};
	char *big_buf = tail_alloc(buf_size);

	fill_memory_ex(big_buf, buf_size, 0, 0x100);

	for (size_t i = 0; i < ARRAY_SIZE(sizes); i++) {
		rc = k_write(1, big_buf, sizes[i]);
		tprintf("write(1, \"%s\"..., %zu) = %s\n",
			big_buf_str, sizes[i], sprintrc(rc));
		dump_str_ex(big_buf, MIN(sizes[i], buf_size),
			    sizes[i] > six_wide_size ? 6 : 5);

		if (sizes[i] == buf_size + 1)
			tprintf(" | <Cannot fetch 1 byte from pid %d @%p>\n",
				getpid(), big_buf + buf_size);

		if (sizes[i] == buf_size + 2)
			tprintf(" | <Cannot fetch 2 bytes from pid %d @%p>\n",
				getpid(), big_buf + buf_size);
	}

	close(1);

	if (open("/dev/zero", O_RDONLY))
		perror_msg_and_fail("open");

	if (open("/dev/null", O_WRONLY) != 1)
		perror_msg_and_fail("open");

	(void) close(4);
	if (open("/dev/zero", O_RDONLY) != 4)
		perror_msg_and_fail("open");

	(void) close(5);
	if (open("/dev/null", O_WRONLY) != 5)
		perror_msg_and_fail("open");

	for (unsigned int i = 0; i <= DEFAULT_STRLEN; ++i)
		test_dump(i, false);

	test_dump(256, true);

	if (need_cleanup && unlink(tmp))
		perror_msg_and_fail("unlink: %s", tmp);

	tprintf("+++ exited with 0 +++\n");
	return 0;
}
