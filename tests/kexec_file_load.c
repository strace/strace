/*
 * Check decoding of kexec_file_load syscall.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
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

#define CMDLINE_STR "deadcodebaddatadefaced";

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
		{ (kernel_ulong_t) 0xbadc0dedda7a1058ULL,
			"0xbadc0ded", "0x",
			"da7a1058 /* KEXEC_FILE_??? */" },
		{ 0, "", "", "0" },
		{ 0xdeadbeef, "", "", "KEXEC_FILE_UNLOAD|KEXEC_FILE_ON_CRASH|"
			"KEXEC_FILE_NO_INITRAMFS|0xdeadbee8" },
	};


	long rc;
	char *cmdline = tail_memdup(cmdline_str, sizeof(cmdline_str));
	char *cmdline_short =
		tail_memdup(cmdline_short_str, sizeof(cmdline_short_str));
	char cmdline_ptr[sizeof("0x") + sizeof(void *) * 2];
	char cmdline_short_ptr[sizeof("0x") + sizeof(void *) * 2];
	unsigned int i;
	unsigned int j;

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

	for (i = 0; i < ARRAY_SIZE(flags); i++) {
		for (j = 0; j < ARRAY_SIZE(cmdlines); j++) {
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
