/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include "flock.h"

#define FILE_LEN 4096

#define TEST_FLOCK_EINVAL(cmd) test_flock_einval(cmd, #cmd)
#define TEST_FLOCK64_EINVAL(cmd) test_flock64_einval(cmd, #cmd)

#ifdef HAVE_TYPEOF
# define TYPEOF_FLOCK_OFF_T typeof(((struct_kernel_flock *) NULL)->l_len)
#else
# define TYPEOF_FLOCK_OFF_T off_t
#endif

static const char *errstr;

static long
invoke_test_syscall(const unsigned int fd, const unsigned int cmd, void *const p)
{
	const kernel_ulong_t kfd = F8ILL_KULONG_MASK | fd;
	const kernel_ulong_t op = F8ILL_KULONG_MASK | cmd;

	long rc = syscall(TEST_SYSCALL_NR, kfd, op, (uintptr_t) p);
	errstr = sprintrc(rc);
	return rc;
}

static void
test_flock_einval(const int cmd, const char *name)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(struct_kernel_flock, fl);
	memset(fl, 0, sizeof(*fl));
	fl->l_type = F_RDLCK;
	fl->l_start = (TYPEOF_FLOCK_OFF_T) 0xdefaced1facefeedULL;
	fl->l_len = (TYPEOF_FLOCK_OFF_T) 0xdefaced2cafef00dULL;

	invoke_test_syscall(0, cmd, fl);
	printf("%s(0, %s, {l_type=F_RDLCK, l_whence=SEEK_SET"
	       ", l_start=%jd, l_len=%jd}) = %s\n", TEST_SYSCALL_STR, name,
	       (intmax_t) fl->l_start, (intmax_t) fl->l_len, errstr);

	void *const bad_addr = (void *) fl + 1;
	invoke_test_syscall(0, cmd, bad_addr);
	printf("%s(0, %s, %p) = %s\n",
	       TEST_SYSCALL_STR, name, bad_addr, errstr);
}

/*
 * This function is not declared static to avoid potential
 * "defined but not used" warning when included by fcntl.c
 */
void
test_flock64_einval(const int cmd, const char *name)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(struct_kernel_flock64, fl);
	memset(fl, 0, sizeof(*fl));
	fl->l_type = F_RDLCK;
	fl->l_start = (TYPEOF_FLOCK_OFF_T) 0xdefaced1facefeedULL;
	fl->l_len = (TYPEOF_FLOCK_OFF_T) 0xdefaced2cafef00dULL;

	invoke_test_syscall(0, cmd, fl);
	printf("%s(0, %s, {l_type=F_RDLCK, l_whence=SEEK_SET"
	       ", l_start=%jd, l_len=%jd}) = %s\n", TEST_SYSCALL_STR, name,
	       (intmax_t) fl->l_start, (intmax_t) fl->l_len, errstr);

	void *const bad_addr = (void *) fl + 1;
	invoke_test_syscall(0, cmd, bad_addr);
	printf("%s(0, %s, %p) = %s\n",
	       TEST_SYSCALL_STR, name, bad_addr, errstr);
}

static void
test_flock(void)
{
	TEST_FLOCK_EINVAL(F_SETLK);
	TEST_FLOCK_EINVAL(F_SETLKW);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct_kernel_flock, fl);
	memset(fl, 0, sizeof(*fl));
	fl->l_type = F_RDLCK;
	fl->l_len = FILE_LEN;

	long rc = invoke_test_syscall(0, F_SETLK, fl);
	printf("%s(0, F_SETLK, {l_type=F_RDLCK, l_whence=SEEK_SET"
	       ", l_start=0, l_len=%d}) = %s\n",
	       TEST_SYSCALL_STR, FILE_LEN, errstr);
	if (rc)
		return;

	invoke_test_syscall(0, F_GETLK, fl);
	printf("%s(0, F_GETLK, {l_type=F_UNLCK, l_whence=SEEK_SET"
	       ", l_start=0, l_len=%d, l_pid=0}) = 0\n",
	       TEST_SYSCALL_STR, FILE_LEN);

	invoke_test_syscall(0, F_SETLKW, fl);
	printf("%s(0, F_SETLKW, {l_type=F_UNLCK, l_whence=SEEK_SET"
	       ", l_start=0, l_len=%d}) = 0\n",
	       TEST_SYSCALL_STR, FILE_LEN);
}

static void
test_flock64_ofd(void)
{
#if defined F_OFD_GETLK && defined F_OFD_SETLK && defined F_OFD_SETLKW
	TEST_FLOCK64_EINVAL(F_OFD_SETLK);
	TEST_FLOCK64_EINVAL(F_OFD_SETLKW);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct_kernel_flock64, fl);
	memset(fl, 0, sizeof(*fl));
	fl->l_type = F_RDLCK;
	fl->l_len = FILE_LEN;

	long rc = invoke_test_syscall(0, F_OFD_SETLK, fl);
	printf("%s(0, F_OFD_SETLK, {l_type=F_RDLCK, l_whence=SEEK_SET"
	       ", l_start=0, l_len=%d}) = %s\n",
	       TEST_SYSCALL_STR, FILE_LEN, errstr);
	if (rc)
		return;

	invoke_test_syscall(0, F_OFD_GETLK, fl);
	printf("%s(0, F_OFD_GETLK, {l_type=F_UNLCK, l_whence=SEEK_SET"
	       ", l_start=0, l_len=%d, l_pid=0}) = 0\n",
	       TEST_SYSCALL_STR, FILE_LEN);

	invoke_test_syscall(0, F_OFD_SETLKW, fl);
	printf("%s(0, F_OFD_SETLKW, {l_type=F_UNLCK, l_whence=SEEK_SET"
	       ", l_start=0, l_len=%d}) = 0\n",
	       TEST_SYSCALL_STR, FILE_LEN);
#endif /* F_OFD_GETLK && F_OFD_SETLK && F_OFD_SETLKW */
}

static void test_flock64_lk64(void);

static void
test_flock64(void)
{
	test_flock64_ofd();
	test_flock64_lk64();
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
	long rc = invoke_test_syscall(0, cmd, fo);
	printf("%s(0, %s, {type=%s, pid=%d}) = %s\n",
	       TEST_SYSCALL_STR, cmd_name, type_name, fo->pid, errstr);

	void *bad_addr = (void *) fo + 1;
	invoke_test_syscall(0, cmd, bad_addr);
	printf("%s(0, %s, %p) = %s\n",
	       TEST_SYSCALL_STR, cmd_name, bad_addr, errstr);

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

struct fcntl_cmd_check {
	int fd;
	int cmd;
	const char *cmd_str;
	long arg;
	const char *arg_str;
	void (*print_flags)(long rc);
};

static void
print_retval_flags(const struct fcntl_cmd_check *check, long rc)
{
	if (check->print_flags) {
		check->print_flags(rc);
	} else {
		printf("%s", errstr);
	}
	printf("\n");
}

static void
test_other_set_cmd(const struct fcntl_cmd_check *check)
{
	invoke_test_syscall(check->fd, check->cmd, (void *) check->arg);
	printf("%s(%d, %s, %s) = %s\n",
	       TEST_SYSCALL_STR, check->fd,
	       check->cmd_str, check->arg_str, errstr);

	/* bad file fd */
	invoke_test_syscall(-1, check->cmd, (void *) check->arg);
	printf("%s(-1, %s, %s) = %s\n",
	       TEST_SYSCALL_STR, check->cmd_str,
	       check->arg_str, errstr);
}

static void
test_other_get_cmd(const struct fcntl_cmd_check *check)
{
	long rc = invoke_test_syscall(check->fd, check->cmd, NULL);
	printf("%s(%d, %s) = ",
	       TEST_SYSCALL_STR, check->fd, check->cmd_str);
	print_retval_flags(check, rc);

	/* bad file fd */
	invoke_test_syscall(-1, check->cmd, NULL);
	printf("%s(-1, %s) = %s\n",
	       TEST_SYSCALL_STR, check->cmd_str, errstr);
}

static void
print_flags_getfd(long rc)
{
	assert(rc >= 0);
	printf("%#lx%s", rc, rc & 1 ? " (flags FD_CLOEXEC)" : "");
}

static void
print_flags_getsig(long rc)
{
	assert(rc >= 0);

	if (!rc) {
		printf("%ld", rc);
	} else {
		printf("%ld (%s)", rc, signal2name((int) rc));
	}
}

static void
print_flags_getlease(long rc)
{
	assert(rc >= 0);
	const char *text;

	switch (rc) {
	case F_RDLCK:
		text = "F_RDLCK";
		break;
	case F_WRLCK:
		text = "F_WRLCK";
		break;
	case F_UNLCK:
		text = "F_UNLCK";
		break;
	default:
		error_msg_and_fail("fcntl returned %#lx, does the"
				   " test have to be updated?", rc);
	}
	printf("%#lx (%s)", rc, text);
}

static void
test_fcntl_others(void)
{
	static const struct fcntl_cmd_check set_checks[] = {
		{ 0, ARG_STR(F_SETFD), ARG_STR(FD_CLOEXEC) },
		{ 0, ARG_STR(F_SETOWN), ARG_STR(20) },
#ifdef F_SETPIPE_SZ
		{ 0, ARG_STR(F_SETPIPE_SZ), ARG_STR(4097) },
#endif
		{ 0, ARG_STR(F_DUPFD), ARG_STR(0) },
#ifdef F_DUPFD_CLOEXEC
		{ 0, ARG_STR(F_DUPFD_CLOEXEC), ARG_STR(0) },
#endif
		{ 0, ARG_STR(F_SETFL), ARG_STR(O_RDWR|O_LARGEFILE) },
		{ 0, ARG_STR(F_NOTIFY), ARG_STR(DN_ACCESS) },
		{ 1, ARG_STR(F_SETLEASE), ARG_STR(F_RDLCK) },
		{ 0, ARG_STR(F_SETSIG), 0, "SIG_0" },
		{ 1, ARG_STR(F_SETSIG), 1, "SIGHUP" }
	};
	for (unsigned int i = 0; i < ARRAY_SIZE(set_checks); i++) {
		test_other_set_cmd(set_checks + i);
	}

	static const struct fcntl_cmd_check get_checks[] = {
		{ 0, ARG_STR(F_GETFD), .print_flags = print_flags_getfd },
		{ 1, ARG_STR(F_GETFD), .print_flags = print_flags_getfd },
		{ 0, ARG_STR(F_GETOWN) },
#ifdef F_GETPIPE_SZ
		{ 0, ARG_STR(F_GETPIPE_SZ) },
#endif
		{ 1, ARG_STR(F_GETLEASE), .print_flags = print_flags_getlease },
		{ 0, ARG_STR(F_GETSIG), .print_flags = print_flags_getsig },
		{ 1, ARG_STR(F_GETSIG), .print_flags = print_flags_getsig }
	};
	for (unsigned int j = 0; j < ARRAY_SIZE(get_checks); j++) {
		test_other_get_cmd(get_checks + j);
	}
}

static void
create_sample(void)
{
	char fname[] = TEST_SYSCALL_STR "_XXXXXX";

	(void) close(0);
	if (mkstemp(fname))
		perror_msg_and_fail("mkstemp: %s", fname);
	if (unlink(fname))
		perror_msg_and_fail("unlink: %s", fname);
	if (ftruncate(0, FILE_LEN))
		perror_msg_and_fail("ftruncate");
}

int
main(void)
{
	create_sample();
	test_flock();
	test_flock64();
#ifdef TEST_F_OWNER_EX
	test_f_owner_ex();
#endif
	test_fcntl_others();

	puts("+++ exited with 0 +++");
	return 0;
}
