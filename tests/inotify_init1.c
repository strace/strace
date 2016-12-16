/*
 * Check decoding of inotify_init1 syscall.
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

#if defined(__NR_inotify_init1)

# include <fcntl.h>
# include <stdio.h>
# include <unistd.h>

# ifdef O_CLOEXEC
#  define cloexec_flag O_CLOEXEC
# else
#  define cloexec_flag 0
# endif
# define all_flags (O_NONBLOCK | cloexec_flag)

int
main(void)
{
	static const kernel_ulong_t bogus_flags1 =
		(kernel_ulong_t) 0xfacefeeddeadbeefULL | O_NONBLOCK;
	static const kernel_ulong_t bogus_flags2 =
		(kernel_ulong_t) 0x55555550ff96b77bULL & ~all_flags;

	long rc;

	rc = syscall(__NR_inotify_init1, bogus_flags1);
	printf("inotify_init1(IN_NONBLOCK|%s%#x) = %s\n",
	       bogus_flags1 & cloexec_flag  ? "IN_CLOEXEC|" : "",
	       (unsigned int) (bogus_flags1 & ~all_flags),
	       sprintrc(rc));

	rc = syscall(__NR_inotify_init1, bogus_flags2);
	printf("inotify_init1(%#x /* IN_??? */) = %s\n",
	       (unsigned int) bogus_flags2, sprintrc(rc));

	rc = syscall(__NR_inotify_init1, all_flags);
	printf("inotify_init1(IN_NONBLOCK%s) = %s\n",
	       all_flags & cloexec_flag ? "|IN_CLOEXEC" : "", sprintrc(rc));

	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_inotify_init1");

#endif
