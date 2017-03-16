/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
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

#if defined __NR_get_robust_list && defined __NR_set_robust_list

# include <stdio.h>
# include <unistd.h>

static const char *
sprintaddr(void *addr)
{
	static char buf[sizeof(addr) * 2 + sizeof("0x")];

	if (!addr)
		return "NULL";
	else
		snprintf(buf, sizeof(buf), "%p", addr);

	return buf;
}

int
main(void)
{
	const pid_t pid = getpid();
	const long long_pid = (unsigned long) (0xdeadbeef00000000LL | pid);
	TAIL_ALLOC_OBJECT_CONST_PTR(void *, p_head);
	TAIL_ALLOC_OBJECT_CONST_PTR(size_t, p_len);

	if (syscall(__NR_get_robust_list, long_pid, p_head, p_len))
		perror_msg_and_skip("get_robust_list");
	printf("get_robust_list(%d, [%s], [%lu]) = 0\n",
	       (int) pid, sprintaddr(*p_head), (unsigned long) *p_len);

	void *head = tail_alloc(*p_len);
	if (syscall(__NR_set_robust_list, head, *p_len))
		perror_msg_and_skip("set_robust_list");
	printf("set_robust_list(%p, %lu) = 0\n",
	       head, (unsigned long) *p_len);

	if (syscall(__NR_get_robust_list, long_pid, p_head, p_len))
		perror_msg_and_skip("get_robust_list");
	printf("get_robust_list(%d, [%s], [%lu]) = 0\n",
	       (int) pid, sprintaddr(*p_head), (unsigned long) *p_len);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_get_robust_list && __NR_set_robust_list")

#endif
