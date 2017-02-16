/*
 * Helper header containing common code for finit_module, init_module,
 * and delete_module tests.
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

#ifndef STRACE_TESTS_INIT_DELETE_MODULE_H
#define STRACE_TESTS_INIT_DELETE_MODULE_H

# include <stdbool.h>
# include <stdio.h>

enum {
	PARAM1_LEN = 33,
	PARAM2_LEN = 8,
	PARAM1_BASE = 0x30,
	PARAM2_BASE = 0x80,
	MAX_STRLEN = 32,
};

static void
print_str(unsigned int base, unsigned int len, bool escape)
{
	unsigned int i;

	if (!escape) {
		for (i = base; i < (base + len); i++)
			putc(i, stdout);

		return;
	}

	for (i = base; i < (base + len); i++)
		printf("\\%u%u%u", (i >> 6) & 0x3, (i >> 3) & 0x7, i & 0x7);
}

#endif /* !STRACE_TESTS_INIT_DELETE_MODULE_H */
