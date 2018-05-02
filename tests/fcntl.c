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

#ifdef __NR_fcntl

# define TEST_SYSCALL_NR __NR_fcntl
# define TEST_SYSCALL_STR "fcntl"
# include "struct_flock.c"

# define TEST_FLOCK64_EINVAL(cmd) test_flock64_einval(cmd, #cmd)

static void
test_flock64_einval(const int cmd, const char *name)
{
	struct_kernel_flock64 fl = {
		.l_type = F_RDLCK,
		.l_start = 0xdefaced1facefeedULL,
		.l_len = 0xdefaced2cafef00dULL
	};
	long rc = invoke_test_syscall(cmd, &fl);
	printf("%s(0, %s, %p) = %s\n",
	       TEST_SYSCALL_STR, name, &fl, sprintrc(rc));
}

static void
test_flock64(void)
{
/*
 * F_[GS]ETOWN_EX had conflicting values with F_[GS]ETLK64
 * in kernel revisions v2.6.32-rc1~96..v2.6.32-rc7~23.
 */
#if !defined(F_GETOWN_EX) || F_GETOWN_EX != F_SETLK64
	TEST_FLOCK64_EINVAL(F_SETLK64);
#endif
/* F_GETLK and F_SETLKW64 have conflicting values on mips64 */
#if !defined(__mips64) || F_GETLK != F_SETLKW64
	TEST_FLOCK64_EINVAL(F_SETLKW64);
#endif
#if !defined(F_SETOWN_EX) || F_SETOWN_EX != F_GETLK64
	TEST_FLOCK64_EINVAL(F_GETLK64);
#endif
}

/*
 * F_[GS]ETOWN_EX had conflicting values with F_[SG]ETLK64
 * in kernel revisions v2.6.32-rc1~96..v2.6.32-rc7~23.
 */
#undef TEST_F_OWNER_EX
#if defined F_GETOWN_EX && defined F_SETOWN_EX \
 && (F_GETOWN_EX != F_SETLK64) && (F_SETOWN_EX != F_GETLK64)
# define TEST_F_OWNER_EX
#endif

#ifdef TEST_F_OWNER_EX
# include "f_owner_ex.h"

static long
test_f_owner_ex_type_pid(const int cmd, const char *const cmd_name,
			 const int type, const char *const type_name,
			 pid_t pid)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(struct_kernel_f_owner_ex, fo);

	fo->type = type;
	fo->pid = pid;
	long rc = invoke_test_syscall(cmd, fo);
	printf("%s(0, %s, {type=%s, pid=%d}) = %s\n",
	       TEST_SYSCALL_STR, cmd_name, type_name, fo->pid, sprintrc(rc));

	void *bad_addr = (void *) fo + 1;
	long rc_efault = invoke_test_syscall(cmd, bad_addr);
	printf("%s(0, %s, %p) = %s\n",
	       TEST_SYSCALL_STR, cmd_name, bad_addr, sprintrc(rc_efault));

	return rc;
}

static void
test_f_owner_ex_umove_or_printaddr(const int type, const char *const type_name,
				   pid_t pid)
{
	long rc = test_f_owner_ex_type_pid(ARG_STR(F_SETOWN_EX),
					   type, type_name, pid);
	if (!rc)
		test_f_owner_ex_type_pid(ARG_STR(F_GETOWN_EX),
					 type, type_name, pid);
}

static void
test_f_owner_ex(void)
{
	static const struct {
		int type;
		const char *type_name;
		pid_t pid[2];
	} a[] = {
		{ ARG_STR(F_OWNER_TID), { 1234567890, 20 } },
		{ ARG_STR(F_OWNER_PID), { 1298126790, 30 } },
		{ ARG_STR(F_OWNER_PGRP), { 1294567890, 40 } }
	};

	for (unsigned int i = 0; i < ARRAY_SIZE(a); i++) {
		for (unsigned int j = 0; j < ARRAY_SIZE(a[0].pid); j++) {
			test_f_owner_ex_umove_or_printaddr(a[i].type,
							   a[i].type_name,
							   a[i].pid[j]);
		}
	}
}
#endif /* TEST_F_OWNER_EX */

int
main(void)
{
	create_sample();
	test_flock();
	test_flock64();
#ifdef TEST_F_OWNER_EX
	test_f_owner_ex();
#endif

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_fcntl")

#endif
