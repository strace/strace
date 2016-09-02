/*
 * Copyright (c) 2016 Eugene Syromiatnikov <evgsyr@gmail.com>
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
#include <stdio.h>

/**
 * Provides pointer to static string buffer with printed return code in format
 * used by strace - with errno and error message.
 *
 * @param rc Return code.
 * @return   Pointer to (statically allocated) buffer containing decimal
 *           representation of return code and errno/error message in case @rc
 *           is equal to -1.
 */
const char *
sprintrc(long rc)
{
	static char buf[4096];

	if (rc == 0)
		return "0";

	int ret = (rc == -1)
		? snprintf(buf, sizeof(buf), "-1 %s (%m)", errno2name())
		: snprintf(buf, sizeof(buf), "%ld", rc);

	if (ret < 0)
		perror_msg_and_fail("snprintf");
	if ((size_t) ret >= sizeof(buf))
		error_msg_and_fail("snprintf overflow: got %d, expected"
				   " no more than %zu", ret, sizeof(buf));

	return buf;
}
