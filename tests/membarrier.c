/*
 * Copyright (c) 2015-2017 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2018 The strace developers.
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

#ifdef __NR_membarrier

# include <assert.h>
# include <errno.h>
# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	assert(syscall(__NR_membarrier, 3, 255) == -1);
	int saved_errno = errno;
	printf("membarrier(0x3 /* MEMBARRIER_CMD_??? */, 255) = %s\n",
	       sprintrc(-1));
	if (saved_errno != ENOSYS) {
		const char *text_global;
		const char *text;
		int rc = syscall(__NR_membarrier, 0, 0);

		assert(rc >= 0);

		text_global = rc & 1 ? "MEMBARRIER_CMD_GLOBAL" : "";

		switch (rc & ~1) {
		case 0:
			text = "";
			break;
		case 8:
			text = "MEMBARRIER_CMD_PRIVATE_EXPEDITED";
			break;
		case 8|16:
			text = "MEMBARRIER_CMD_PRIVATE_EXPEDITED|"
			       "MEMBARRIER_CMD_REGISTER_PRIVATE_EXPEDITED";
			break;

		case 2|4|8|16:
			text = "MEMBARRIER_CMD_GLOBAL_EXPEDITED|"
			       "MEMBARRIER_CMD_REGISTER_GLOBAL_EXPEDITED|"
			       "MEMBARRIER_CMD_PRIVATE_EXPEDITED|"
			       "MEMBARRIER_CMD_REGISTER_PRIVATE_EXPEDITED";
			break;

		case 2|4|8|16|32|64:
			text = "MEMBARRIER_CMD_GLOBAL_EXPEDITED|"
			       "MEMBARRIER_CMD_REGISTER_GLOBAL_EXPEDITED|"
			       "MEMBARRIER_CMD_PRIVATE_EXPEDITED|"
			       "MEMBARRIER_CMD_REGISTER_PRIVATE_EXPEDITED|"
			       "MEMBARRIER_CMD_PRIVATE_EXPEDITED_SYNC_CORE|"
			       "MEMBARRIER_CMD_REGISTER_PRIVATE_EXPEDITED_SYNC_CORE";
			break;

		default:
			error_msg_and_fail("membarrier returned %#x, does"
					   " the test have to be updated?", rc);
		}
		printf("membarrier(MEMBARRIER_CMD_QUERY, 0) = %#x (%s%s%s)\n",
		       rc, text_global, text[0] && text_global[0] ? "|" : "",
		       text);
	}
	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_membarrier")

#endif
