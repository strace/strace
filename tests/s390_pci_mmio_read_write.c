/*
 * Check decoding of s390_pci_mmio_read and s390_pci_mmio_write syscalls.
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
#include <asm/unistd.h>

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

	unsigned int i, j, k, l;
	unsigned int ctr = 0;

	for (i = 0; i < ARRAY_SIZE(addrs); i++) {
		for (j = 0; j < ARRAY_SIZE(bufs); j++) {
			for (k = 0; k < ARRAY_SIZE(sizes); k++) {
				for (l = 0; l < ARRAY_SIZE(bools); l++) {
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
