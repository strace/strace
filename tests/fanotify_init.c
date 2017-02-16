/*
 * Check decoding of fanotify_init syscall.
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

#if defined __NR_fanotify_init

# include <limits.h>
# include <stdio.h>
# include <unistd.h>

/* Performs fanotify_init call via the syscall interface. */
static void
do_call(kernel_ulong_t flags, const char *flags_str,
	kernel_ulong_t event_f_flags, const char *event_f_flags_str)
{
	long rc;

	rc = syscall(__NR_fanotify_init, flags, event_f_flags);

	printf("fanotify_init(%s, %s) = %s\n",
	       flags_str, event_f_flags_str, sprintrc(rc));
}

struct strval {
	kernel_ulong_t val;
	const char *str;
};


int
main(void)
{
	static const struct strval flags[] = {
		{ F8ILL_KULONG_MASK, "FAN_CLASS_NOTIF" },
		{ (kernel_ulong_t) 0xffffffff0000000cULL,
			"0xc /* FAN_CLASS_??? */" },
		{ (kernel_ulong_t) 0xdec0deddefaced04ULL,
			"FAN_CLASS_CONTENT|0xefaced00 /* FAN_??? */" },
		{ (kernel_ulong_t) 0xffffffffffffffffULL,
			"0xc /* FAN_CLASS_??? */|FAN_CLOEXEC|FAN_NONBLOCK|"
			"FAN_UNLIMITED_QUEUE|FAN_UNLIMITED_MARKS|0xffffffc0" },
	};
	static const struct strval event_f_flags[] = {
		{ F8ILL_KULONG_MASK, "O_RDONLY" },
		{ (kernel_ulong_t) 0xdeadbeef80000001ULL,
			"O_WRONLY|0x80000000" }
	};

	unsigned int i;
	unsigned int j;


	for (i = 0; i < ARRAY_SIZE(flags); i++)
		for (j = 0; j < ARRAY_SIZE(event_f_flags); j++)
			do_call(flags[i].val, flags[i].str,
				event_f_flags[j].val, event_f_flags[j].str);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_fanotify_init")

#endif
