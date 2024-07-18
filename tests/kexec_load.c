/*
 * Check decoding of kexec_load syscall.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <stdio.h>
#include <unistd.h>

struct strval {
	kernel_ulong_t val;
	const char *str64;
	const char *str32;
	const char *str;
};

struct segm {
	void *buf;
	size_t bufsz;
	void *mem;
	size_t memsz;
};

int
main(void)
{
	enum {
		NUM_SEGMS = 17,
		NUM_SEGMS_UNCUT = 5,
		NUM_SEGMS_UNCUT_MAX = 9,
		NUM_SEGMS_CUT = 12,
		SEGMS_ARRAY_SIZE = sizeof(struct segm) * NUM_SEGMS,
	};

	static const kernel_ulong_t bogus_zero =
		sizeof(long) < sizeof(kernel_long_t) ? F8ILL_KULONG_MASK : 0;
	static const kernel_ulong_t bogus_entry =
		(kernel_ulong_t) 0xdeadca57badda7a1ULL;
	static const kernel_ulong_t bogus_nsegs =
		(kernel_ulong_t) 0xdec0ded1defaced2ULL;

	static const struct strval flags[] = {
		{ (kernel_ulong_t) 0xbadc0dedda7a1050ULL,
			"0xda7a0000 /* KEXEC_ARCH_??? */|0xbadc0ded0000",
			"0xda7a0000 /* KEXEC_ARCH_??? */|0x",
			"1050 /* KEXEC_??? */" },
		{ 0, "", "", "KEXEC_ARCH_DEFAULT" },
		{ 0x2a000f, "", "",
			"KEXEC_ARCH_SH|KEXEC_ON_CRASH|KEXEC_PRESERVE_CONTEXT"
			"|KEXEC_UPDATE_ELFCOREHDR|KEXEC_CRASH_HOTPLUG_SUPPORT" },
		{ 0xdead0000, "", "", "0xdead0000 /* KEXEC_ARCH_??? */" },
	};

	const char *errstr;
	long rc;
	struct segm *segms = tail_alloc(SEGMS_ARRAY_SIZE);

	fill_memory(segms, SEGMS_ARRAY_SIZE);
	segms[0].buf = segms[0].mem = NULL;

	rc = syscall(__NR_kexec_load, bogus_zero, bogus_zero, bogus_zero,
		flags[0].val);
	printf("kexec_load(NULL, 0, NULL, %s%s) = %s\n",
	       sizeof(long) == 8 ? flags[0].str64 : flags[0].str32,
	       flags[0].str, sprintrc(rc));

	rc = syscall(__NR_kexec_load, bogus_entry, bogus_nsegs,
		     segms + SEGMS_ARRAY_SIZE, flags[1].val);
	printf("kexec_load(%#lx, %lu, %p, %s) = %s\n",
	       (unsigned long) bogus_entry, (unsigned long) bogus_nsegs,
	       segms + SEGMS_ARRAY_SIZE, flags[1].str, sprintrc(rc));

	rc = syscall(__NR_kexec_load, bogus_entry, NUM_SEGMS,
		     segms, flags[2].val);
	printf("kexec_load(%#lx, %lu, %p, %s) = %s\n",
	       (unsigned long) bogus_entry, (unsigned long) NUM_SEGMS,
	       segms, flags[2].str, sprintrc(rc));

	rc = syscall(__NR_kexec_load, bogus_entry, NUM_SEGMS_CUT,
		     segms, flags[3].val);
	errstr = sprintrc(rc);
	printf("kexec_load(%#lx, %lu, [{buf=NULL, bufsz=%zu, mem=NULL, "
	       "memsz=%zu}, ",
	       (unsigned long) bogus_entry, (unsigned long) NUM_SEGMS_CUT,
	       segms[0].bufsz, segms[0].memsz);
	for (unsigned int i = 1; i < NUM_SEGMS_UNCUT_MAX; ++i)
		printf("{buf=%p, bufsz=%zu, mem=%p, memsz=%zu}, ",
		       segms[i].buf, segms[i].bufsz,
		       segms[i].mem, segms[i].memsz);
	printf("...], %s) = %s\n", flags[3].str, errstr);

	rc = syscall(__NR_kexec_load, bogus_entry, NUM_SEGMS_CUT,
		     segms + (NUM_SEGMS - NUM_SEGMS_UNCUT_MAX),
		     flags[0].val);
	errstr = sprintrc(rc);
	printf("kexec_load(%#lx, %lu, [",
	       (unsigned long) bogus_entry, (unsigned long) NUM_SEGMS_CUT);
	for (unsigned int i = NUM_SEGMS - NUM_SEGMS_UNCUT_MAX;
	     i < NUM_SEGMS; ++i)
		printf("{buf=%p, bufsz=%zu, mem=%p, memsz=%zu}, ",
		       segms[i].buf, segms[i].bufsz,
		       segms[i].mem, segms[i].memsz);
	printf("... /* %p */], %s%s) = %s\n",
	       segms + NUM_SEGMS,
	       sizeof(long) == 8 ? flags[0].str64 : flags[0].str32,
	       flags[0].str, errstr);

	rc = syscall(__NR_kexec_load, bogus_entry, NUM_SEGMS_UNCUT,
		     segms + (NUM_SEGMS - NUM_SEGMS_UNCUT),
		     flags[1].val);
	errstr = sprintrc(rc);
	printf("kexec_load(%#lx, %lu, [",
	       (unsigned long) bogus_entry, (unsigned long) NUM_SEGMS_UNCUT);
	for (unsigned int i = NUM_SEGMS - NUM_SEGMS_UNCUT; i < NUM_SEGMS; ++i)
		printf("{buf=%p, bufsz=%zu, mem=%p, memsz=%zu}%s",
		       segms[i].buf, segms[i].bufsz,
		       segms[i].mem, segms[i].memsz,
		       (i == NUM_SEGMS - 1) ? "" : ", ");
	printf("], %s) = %s\n", flags[1].str, errstr);

	rc = syscall(__NR_kexec_load, bogus_entry, NUM_SEGMS_CUT,
		     segms + 1, flags[2].val);
	errstr = sprintrc(rc);
	printf("kexec_load(%#lx, %lu, [",
	       (unsigned long) bogus_entry, (unsigned long) NUM_SEGMS_CUT);
	for (unsigned int i = 1; i < NUM_SEGMS_UNCUT_MAX + 1; ++i)
		printf("{buf=%p, bufsz=%zu, mem=%p, memsz=%zu}, ",
		       segms[i].buf, segms[i].bufsz,
		       segms[i].mem, segms[i].memsz);
	printf("...], %s) = %s\n", flags[2].str, errstr);

	puts("+++ exited with 0 +++");

	return 0;
}
