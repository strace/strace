/*
 * Check decoding of kexec_file_load syscall.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_kexec_file_load

# include <inttypes.h>
# include <stdio.h>
# include <unistd.h>

struct strval {
	kernel_ulong_t val;
	const char *str64;
	const char *str32;
	const char *str;
};

# define CMDLINE_STR "deadcodebaddatadefaced"

int
main(void)
{
	static const kernel_ulong_t bogus_kernel_fd =
		(kernel_ulong_t) 0xdeadca57badda7a1ULL;
	static const kernel_ulong_t bogus_initrd_fd =
		(kernel_ulong_t) 0xdec0ded1defaced2ULL;
	static const char cmdline_str[] = CMDLINE_STR;
	static const char cmdline_short_str[] = "abcdef";

	static const kernel_ulong_t cmdline_lens[] = {
		0,
		(kernel_ulong_t) 0xcaffeeeddeadbeefULL,
		sizeof(cmdline_str),
		sizeof(cmdline_str) - 1,
		sizeof(cmdline_short_str),
		sizeof(cmdline_short_str) - 1,
		sizeof(cmdline_short_str) + 1,
	};
	static const struct strval flags[] = {
		{ (kernel_ulong_t) 0xbadc0dedda7a1050ULL,
			"0xbadc0ded", "0x",
			"da7a1050 /* KEXEC_FILE_??? */" },
		{ 0, "", "", "0" },
		{ 0xdeadbeef, "", "",
			"KEXEC_FILE_UNLOAD|KEXEC_FILE_ON_CRASH|"
			"KEXEC_FILE_NO_INITRAMFS|KEXEC_FILE_DEBUG|0xdeadbee0" },
	};


	long rc;
	char *cmdline = tail_memdup(cmdline_str, sizeof(cmdline_str));
	char *cmdline_short =
		tail_memdup(cmdline_short_str, sizeof(cmdline_short_str));
	char cmdline_ptr[sizeof("0x") + sizeof(void *) * 2];
	char cmdline_short_ptr[sizeof("0x") + sizeof(void *) * 2];

	struct strval cmdlines[] = {
		{ (uintptr_t) NULL, "", "", "NULL" },
		{ (uintptr_t) (cmdline + sizeof(cmdline_str)), "", "",
			cmdline_ptr },
		{ (uintptr_t) cmdline, "", "", "\"deadcodeb\"..." },
		{ (uintptr_t) cmdline, "", "", "\"deadcodeb\"..." },
		{ (uintptr_t) cmdline_short, "", "", "\"abcdef\\0\"" },
		{ (uintptr_t) cmdline_short, "", "", "\"abcdef\"" },
		{ (uintptr_t) cmdline_short, "", "", cmdline_short_ptr },
	};


	snprintf(cmdline_ptr, sizeof(cmdline_ptr), "%p",
		cmdline + sizeof(cmdline_str));
	snprintf(cmdline_short_ptr, sizeof(cmdline_short_ptr), "%p",
		cmdline_short);

	for (unsigned int i = 0; i < ARRAY_SIZE(flags); ++i) {
		for (unsigned int j = 0; j < ARRAY_SIZE(cmdlines); ++j) {
			rc = syscall(__NR_kexec_file_load, bogus_kernel_fd,
				     bogus_initrd_fd, cmdline_lens[j],
				     cmdlines[j].val, flags[i].val);
			printf("kexec_file_load(%d, %d, %llu, %s, %s%s) = %s\n",
			       (int) bogus_kernel_fd, (int) bogus_initrd_fd,
			       (unsigned long long) cmdline_lens[j],
			       cmdlines[j].str,
			       sizeof(kernel_ulong_t) == 8 ? flags[i].str64 :
			       flags[i].str32, flags[i].str, sprintrc(rc));
		}
	}

	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_kexec_file_load");

#endif
