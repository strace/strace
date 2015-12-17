/*
 * Copyright (c) 2015 Gleb Fotengauer-Malinovskiy <glebfm@altlinux.org>
 * Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

int
main(void)
{
#ifdef __NR_readlinkat
	static const char fname[] = "readlinkat.link";
	unsigned char buf[31];
	long rc;
	unsigned int i;

	rc = syscall(__NR_readlinkat, -100, fname, buf, sizeof(buf));
	if (rc < 0)
		return 77;

	printf("readlinkat(AT_FDCWD, \"");
	for (i = 0; fname[i]; ++i)
		printf("\\x%02x", (int) (unsigned char) fname[i]);
	printf("\", \"");
	for (i = 0; i < 3; ++i)
		printf("\\x%02x", (int) buf[i]);
	printf("\"..., %zu) = %ld\n", sizeof(buf), rc);

	puts("+++ exited with 0 +++");
	return 0;
#else
	return 77;
#endif
}
