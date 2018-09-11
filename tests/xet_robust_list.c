/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2017 The strace developers.
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
# include <string.h>
# include <unistd.h>

# include <linux/futex.h>

static void
print_rlh(struct robust_list_head *ptr, size_t len)
{
	printf("{list={next=%p}, futex_offset=%ld, list_op_pending=%p",
	       ptr->list.next, ptr->futex_offset, ptr->list_op_pending);
	if (len > sizeof(*ptr))
		printf(", /* ??? */");
	printf("} /* %p */", ptr);
}

int
main(void)
{
	static const kernel_ulong_t bogus_len =
		(kernel_ulong_t) 0xbadc0deddeadfaceULL;
	const pid_t pid = getpid();
	const long long_pid = (unsigned long) (0xdeadbeef00000000LL | pid);
	TAIL_ALLOC_OBJECT_CONST_PTR(void *, p_head);
	TAIL_ALLOC_OBJECT_CONST_PTR(size_t, p_len);
	long rc;
	const char *errstr;

	if (syscall(__NR_get_robust_list, long_pid, p_head, p_len))
		perror_msg_and_skip("get_robust_list");
	printf("get_robust_list(%d, [", (int) pid);
	if (*p_head)
		print_rlh(*p_head, *p_len);
	else
		printf("NULL");
	printf("], [%lu]) = 0\n", (unsigned long) *p_len);

	rc = syscall(__NR_get_robust_list, long_pid, NULL, NULL);
	printf("get_robust_list(%d, NULL, NULL) = %s\n",
	       (int) pid, sprintrc(rc));

	rc = syscall(__NR_get_robust_list, long_pid, p_head + 1, p_len + 1);
	printf("get_robust_list(%d, %p, %p) = %s\n",
	       (int) pid, p_head + 1, p_len + 1, sprintrc(rc));

	rc = syscall(__NR_get_robust_list, long_pid, p_head + 1, p_len);
	printf("get_robust_list(%d, %p, %p) = %s\n",
	       (int) pid, p_head + 1, p_len, sprintrc(rc));

	rc = syscall(__NR_get_robust_list, long_pid, p_head, p_len + 1);
	printf("get_robust_list(%d, %p, %p) = %s\n",
	       (int) pid, p_head, p_len + 1, sprintrc(rc));

	void *head = tail_alloc(*p_len);
	if (syscall(__NR_set_robust_list, head, *p_len))
		perror_msg_and_skip("set_robust_list");
	printf("set_robust_list(");
	print_rlh(head, *p_len);
	printf(", %lu) = 0\n", (unsigned long) *p_len);

	rc = syscall(__NR_set_robust_list, NULL, 0);
	printf("set_robust_list(NULL, 0) = %s\n", sprintrc(rc));

	rc = syscall(__NR_set_robust_list, p_head + 1, bogus_len);
	printf("set_robust_list(%p, %lu) = %s\n",
	       p_head + 1, (unsigned long) bogus_len, sprintrc(rc));

	rc = syscall(__NR_set_robust_list, head, *p_len - 1);
	printf("set_robust_list(%p, %lu) = %s\n",
	       head, (unsigned long) (*p_len - 1), sprintrc(rc));

	rc = syscall(__NR_set_robust_list, head, *p_len + 1);
	errstr = sprintrc(rc);
	printf("set_robust_list(");
	print_rlh(head, *p_len + 1);
	printf(", %lu) = %s\n", (unsigned long) (*p_len + 1), errstr);

	memset(head, 0, *p_len);
	if (syscall(__NR_get_robust_list, long_pid, p_head, p_len))
		perror_msg_and_skip("get_robust_list");
	printf("get_robust_list(%d, [{list={next=NULL}, futex_offset=0"
	       ", list_op_pending=NULL} /* %p */], [%lu]) = 0\n",
	       (int) pid, head, (unsigned long) *p_len);

	fill_memory(head, *p_len);
	if (syscall(__NR_get_robust_list, long_pid, p_head, p_len))
		perror_msg_and_skip("get_robust_list");
	printf("get_robust_list(%d, [", (int) pid);
	print_rlh(head, *p_len);
	printf("], [%lu]) = 0\n", (unsigned long) *p_len);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_get_robust_list && __NR_set_robust_list")

#endif
