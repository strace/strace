/*
 * Check decoding of s390_pci_mmio_read and s390_pci_mmio_write syscalls.
 *
 * Copyright (c) 2018-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#if defined __NR_s390_pci_mmio_read && defined __NR_s390_pci_mmio_write

# include <errno.h>
# include <stdint.h>
# include <stdio.h>
# include <unistd.h>

static void
do_call(bool wr, kernel_ulong_t mmio_addr, kernel_ulong_t buf,
	kernel_ulong_t len, bool buf_valid, const char *buf_str)
{
	long saved_errno = 0;
	long rc = 0;

	printf("s390_pci_mmio_%s(%#llx, ", wr ? "write" : "read",
	       (unsigned long long) mmio_addr);

	if (!wr) {
		rc = syscall(__NR_s390_pci_mmio_read, mmio_addr, buf, len);
		saved_errno = errno;
	}

	if (buf_valid && !rc) {
		char *buf_ptr = (char *) (uintptr_t) buf;

		print_quoted_hex(buf_ptr,
				 len > DEFAULT_STRLEN ? DEFAULT_STRLEN : len);

		if (len > DEFAULT_STRLEN)
			printf("...");
	} else {
		if (buf_str)
			printf("%s", buf_str);
		else
			printf("%#llx", (unsigned long long) buf);
	}

	printf(", %llu) = ", (unsigned long long) len);

	if (wr)
		rc = syscall(__NR_s390_pci_mmio_write, mmio_addr, buf, len);
	else
		errno = saved_errno;

	puts(sprintrc(rc));
}

int
main(void)
{
	static const size_t buf_size = DEFAULT_STRLEN + 10;

	char *buf = tail_alloc(buf_size);

	bool bools[] = { true, false };

	kernel_ulong_t addrs[] = {
		0,
		(kernel_ulong_t) 0xdeafbeefdeadc0deULL,
	};

	struct {
		kernel_ulong_t buf;
		const char *str;
		size_t size;
	} bufs[] = {
		{ (kernel_ulong_t) ARG_STR(NULL),            0 },
		{ (kernel_ulong_t) (buf + buf_size), NULL,   0 },
		{ (kernel_ulong_t) (buf),            NULL,   buf_size },
		{ (kernel_ulong_t) (buf + 9),        NULL,   buf_size - 9 },
		{ (kernel_ulong_t) (buf + 10),       NULL,   buf_size - 10 },
		{ (kernel_ulong_t) (buf + 16),       NULL,   buf_size - 16 },
		{ (kernel_ulong_t) (buf + 26),       NULL,   buf_size - 26 },
		{ (kernel_ulong_t) (buf + 28),       NULL,   buf_size - 28 },
	};

	kernel_ulong_t sizes[] = {
		0,
		DEFAULT_STRLEN / 2,
		DEFAULT_STRLEN - 10,
		DEFAULT_STRLEN,
		DEFAULT_STRLEN + 1,
		buf_size,
		buf_size + 10,
		(kernel_ulong_t) 0xfacefeedac0ffeedULL,
	};

	unsigned int ctr = 0;

	for (unsigned int i = 0;
	     i < ARRAY_SIZE(addrs); ++i) {
		for (unsigned int j = 0;
		     j < ARRAY_SIZE(bufs); ++j) {
			for (unsigned int k = 0;
			     k < ARRAY_SIZE(sizes); ++k) {
				for (unsigned int l = 0;
				     l < ARRAY_SIZE(bools); ++l) {
					bool valid = bufs[j].buf &&
						bufs[j].size >=
						MIN(sizes[k],
						    DEFAULT_STRLEN + 1);

					if (bufs[j].size && bools[l])
						fill_memory_ex((char *) buf,
							       bufs[j].size,
							       0xC0 + ctr, 255);

					do_call(bools[l], addrs[i], bufs[j].buf,
						sizes[k], valid, bufs[j].str);

					ctr++;
				}
			}
		}
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_s390_pci_mmio_read && __NR_s390_pci_mmio_write");

#endif
