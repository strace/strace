/*
 * Check decoding of prctl PR_GET_SECUREBITS/PR_SET_SECUREBITS operations.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
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
#include <linux/prctl.h>

#if defined __NR_prctl && defined PR_GET_SECUREBITS && defined PR_SET_SECUREBITS

# include <stdio.h>
# include <unistd.h>

# include "xlat.h"
# include "xlat/secbits.h"

static const char *errstr;

static long
prctl(kernel_ulong_t arg1, kernel_ulong_t arg2)
{
	static const kernel_ulong_t bogus_arg =
		(kernel_ulong_t) 0xdeadbeefbadc0dedULL;
	long rc = syscall(__NR_prctl, arg1, arg2, bogus_arg);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	static const kernel_ulong_t bits1 =
		(kernel_ulong_t) 0xdeadc0defacebeefULL;
	static const kernel_ulong_t bits2 =
		(kernel_ulong_t) 0xbadc0ded00000000ULL;
	static const kernel_ulong_t bits3 =
		(kernel_ulong_t) 0xffULL;

	prctl(PR_SET_SECUREBITS, 0);
	printf("prctl(PR_SET_SECUREBITS, 0) = %s\n", errstr);

	prctl(PR_SET_SECUREBITS, bits1);
	printf("prctl(PR_SET_SECUREBITS, SECBIT_NOROOT|SECBIT_NOROOT_LOCKED|"
	       "SECBIT_NO_SETUID_FIXUP|SECBIT_NO_SETUID_FIXUP_LOCKED|"
	       "SECBIT_KEEP_CAPS_LOCKED|SECBIT_NO_CAP_AMBIENT_RAISE|"
	       "SECBIT_NO_CAP_AMBIENT_RAISE_LOCKED|%#llx) = %s\n",
	       (unsigned long long) bits1 & ~0xffULL, errstr);

	if (bits2) {
		prctl(PR_SET_SECUREBITS, bits2);
		printf("prctl(PR_SET_SECUREBITS, %#llx /* SECBIT_??? */)"
		       " = %s\n", (unsigned long long) bits2, errstr);
	}

	prctl(PR_SET_SECUREBITS, bits3);
	printf("prctl(PR_SET_SECUREBITS, SECBIT_NOROOT|SECBIT_NOROOT_LOCKED|"
	       "SECBIT_NO_SETUID_FIXUP|SECBIT_NO_SETUID_FIXUP_LOCKED|"
	       "SECBIT_KEEP_CAPS|SECBIT_KEEP_CAPS_LOCKED|"
	       "SECBIT_NO_CAP_AMBIENT_RAISE|SECBIT_NO_CAP_AMBIENT_RAISE_LOCKED)"
	       " = %s\n", errstr);

	long rc = prctl(PR_GET_SECUREBITS, bits1);
	printf("prctl(PR_GET_SECUREBITS) = %s", errstr);
	if (rc > 0) {
		printf(" (");
		printflags(secbits, rc, NULL);
		printf(")");
	}

	puts("");

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_prctl && PR_GET_SECUREBITS && PR_SET_SECUREBITS")

#endif
