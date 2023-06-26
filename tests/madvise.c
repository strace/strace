/*
 * Copyright (c) 2017-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include "scno.h"

static const char *errstr;

static long
k_madvise(const kernel_ulong_t addr,
	  const kernel_ulong_t length,
	  const kernel_ulong_t advice)
{
	long rc = syscall(__NR_madvise, addr, length, advice);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	const unsigned long length = get_page_size();
	void *const addr = tail_alloc(length);
	long rc;

	rc = madvise(addr, length, MADV_NORMAL);
	printf("madvise(%p, %lu, " XLAT_FMT ") = %s\n",
	       addr, length, XLAT_ARGS(MADV_NORMAL), sprintrc(rc));

	static const kernel_ulong_t advice =
		(kernel_ulong_t) 0xfacefeed00000000ULL | MADV_RANDOM;
	rc = k_madvise((uintptr_t) addr, length, advice);
	printf("madvise(%p, %lu, " XLAT_FMT ") = %s\n",
	       addr, length, XLAT_ARGS(MADV_RANDOM), sprintrc(rc));

	static const kernel_ulong_t bogus_length =
		(kernel_ulong_t) 0xfffffffffffffaceULL;
	rc = k_madvise(0, bogus_length, MADV_SEQUENTIAL);
	printf("madvise(NULL, %llu, " XLAT_FMT ") = %s\n",
	       (unsigned long long) bogus_length,
	       XLAT_ARGS(MADV_SEQUENTIAL), sprintrc(rc));

	if (F8ILL_KULONG_SUPPORTED) {
		rc = k_madvise(f8ill_ptr_to_kulong(addr), length,
			       MADV_WILLNEED);
		printf("madvise(%#llx, %lu, " XLAT_FMT ") = %s\n",
		       (unsigned long long) f8ill_ptr_to_kulong(addr),
		       length, XLAT_ARGS(MADV_WILLNEED), sprintrc(rc));
	}

	static const struct strval32 advices[] = {
#ifdef __alpha__
		{ 4, "0x4" NRAW(" /* MADV_??? */") },
		{ ARG_XLAT_KNOWN(0x6, "MADV_DONTNEED") },
#else
		{ ARG_XLAT_KNOWN(0x4, "MADV_DONTNEED") },
		{ 6, "0x6" NRAW(" /* MADV_??? */") },
#endif
		{ 5, "0x5" NRAW(" /* MADV_??? */") },
		{ 7, "0x7" NRAW(" /* MADV_??? */") },
		{ ARG_XLAT_KNOWN(0x8, "MADV_FREE") },
		{ ARG_XLAT_KNOWN(0x9, "MADV_REMOVE") },
		{ ARG_XLAT_KNOWN(0xa, "MADV_DONTFORK") },
		{ ARG_XLAT_KNOWN(0xb, "MADV_DOFORK") },
#ifdef __hppa__
		{ 12, "0xc" NRAW(" /* generic MADV_MERGEABLE */") },
		{ 13, "0xd" NRAW(" /* generic MADV_UNMERGEABLE */") },
		{ 14, "0xe" NRAW(" /* generic MADV_HUGEPAGE */") },
		{ 15, "0xf" NRAW(" /* generic MADV_NOHUGEPAGE */") },
		{ 16, "0x10" NRAW(" /* generic MADV_DONTDUMP */") },
		{ 17, "0x11" NRAW(" /* generic MADV_DODUMP */") },
		{ 18, "0x12" NRAW(" /* generic MADV_WIPEONFORK */") },
		{ 19, "0x13" NRAW(" /* generic MADV_KEEPONFORK */") },
#else
		{ ARG_XLAT_KNOWN(0xc, "MADV_MERGEABLE") },
		{ ARG_XLAT_KNOWN(0xd, "MADV_UNMERGEABLE") },
		{ ARG_XLAT_KNOWN(0xe, "MADV_HUGEPAGE") },
		{ ARG_XLAT_KNOWN(0xf, "MADV_NOHUGEPAGE") },
		{ ARG_XLAT_KNOWN(0x10, "MADV_DONTDUMP") },
		{ ARG_XLAT_KNOWN(0x11, "MADV_DODUMP") },
		{ ARG_XLAT_KNOWN(0x12, "MADV_WIPEONFORK") },
		{ ARG_XLAT_KNOWN(0x13, "MADV_KEEPONFORK") },
#endif
		{ ARG_XLAT_KNOWN(0x14, "MADV_COLD") },
		{ ARG_XLAT_KNOWN(0x15, "MADV_PAGEOUT") },
		{ ARG_XLAT_KNOWN(0x16, "MADV_POPULATE_READ") },
		{ ARG_XLAT_KNOWN(0x17, "MADV_POPULATE_WRITE") },
		{ ARG_XLAT_KNOWN(0x18, "MADV_DONTNEED_LOCKED") },
#ifdef __hppa__
		{ 25, "0x19" NRAW(" /* generic MADV_COLLAPSE */") },
#else
		{ ARG_XLAT_KNOWN(0x19, "MADV_COLLAPSE") },
#endif
		{ ARG_XLAT_UNKNOWN(0x1a, "MADV_???") },

		{ ARG_XLAT_UNKNOWN(0x40, "MADV_???") },
#ifdef __hppa__
		{ 65, "0x41" NRAW(" /* old MADV_MERGEABLE */") },
		{ 66, "0x42" NRAW(" /* old MADV_UNMERGEABLE */") },
		{ 67, "0x43" NRAW(" /* old MADV_HUGEPAGE */") },
		{ 68, "0x44" NRAW(" /* old MADV_NOHUGEPAGE */") },
		{ 69, "0x45" NRAW(" /* old MADV_DONTDUMP */") },
		{ 70, "0x46" NRAW(" /* old MADV_DODUMP */") },
		{ 71, "0x47" NRAW(" /* old MADV_WIPEONFORK */") },
		{ 72, "0x48" NRAW(" /* old MADV_KEEPONFORK */") },
		{ 73, "0x49" NRAW(" /* old MADV_COLLAPSE */") },
#else
		{ ARG_XLAT_UNKNOWN(0x41, "MADV_???") },
		{ ARG_XLAT_UNKNOWN(0x42, "MADV_???") },
		{ ARG_XLAT_UNKNOWN(0x43, "MADV_???") },
		{ ARG_XLAT_UNKNOWN(0x44, "MADV_???") },
		{ ARG_XLAT_UNKNOWN(0x45, "MADV_???") },
		{ ARG_XLAT_UNKNOWN(0x46, "MADV_???") },
		{ ARG_XLAT_UNKNOWN(0x47, "MADV_???") },
		{ ARG_XLAT_UNKNOWN(0x48, "MADV_???") },
		{ ARG_XLAT_UNKNOWN(0x49, "MADV_???") },
#endif
		{ ARG_XLAT_UNKNOWN(0x4a, "MADV_???") },

		{ ARG_XLAT_UNKNOWN(0x63, "MADV_???") },
		{ ARG_XLAT_KNOWN(0x64, "MADV_HWPOISON") },
		{ ARG_XLAT_KNOWN(0x65, "MADV_SOFT_OFFLINE") },
		{ ARG_XLAT_UNKNOWN(0x66, "MADV_???") },

		{ ARG_XLAT_UNKNOWN(0x80, "MADV_???") },
		{ ARG_XLAT_UNKNOWN(0x81, "MADV_???") },
		{ ARG_XLAT_UNKNOWN(0x100, "MADV_???") },
		{ ARG_XLAT_UNKNOWN(0x101, "MADV_???") },
		{ ARG_XLAT_UNKNOWN(0x1000, "MADV_???") },
		{ ARG_XLAT_UNKNOWN(0x1001, "MADV_???") },
		{ ARG_XLAT_UNKNOWN(0xbadc0ded, "MADV_???") },
	};

	for (unsigned int i = 0; i < ARRAY_SIZE(advices); i++) {
		rc = madvise(NULL, length, advices[i].val);
		printf("madvise(NULL, %lu, %s) = %s\n",
		       length, advices[i].str, sprintrc(rc));
	}

	puts("+++ exited with 0 +++");
	return 0;
}
