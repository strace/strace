/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <linux/fcntl.h>
#include "pidns.h"
#include "scno.h"

#define FILE_LEN 4096

#define TEST_FLOCK_EINVAL(cmd) test_flock_einval(cmd, #cmd)
#define TEST_FLOCK64_EINVAL(cmd) test_flock64_einval(cmd, #cmd)

#ifndef NEED_TEST_FLOCK64_EINVAL
# if defined F_OFD_GETLK && defined F_OFD_SETLK && defined F_OFD_SETLKW
#  define NEED_TEST_FLOCK64_EINVAL
# endif
#endif

#ifdef HAVE_TYPEOF
# define TYPEOF_FLOCK_OFF_T typeof(((struct flock *) NULL)->l_len)
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
	TAIL_ALLOC_OBJECT_CONST_PTR(struct flock, fl);
	memset(fl, 0, sizeof(*fl));
	fl->l_type = F_RDLCK;
	fl->l_start = (TYPEOF_FLOCK_OFF_T) 0xdefaced1facefeedULL;
	fl->l_len = (TYPEOF_FLOCK_OFF_T) 0xdefaced2cafef00dULL;

	invoke_test_syscall(0, cmd, fl);
	pidns_print_leader();
	printf("%s(0, %s, {l_type=F_RDLCK, l_whence=SEEK_SET"
	       ", l_start=%jd, l_len=%jd}) = %s\n", TEST_SYSCALL_STR, name,
	       (intmax_t) fl->l_start, (intmax_t) fl->l_len, errstr);

	void *const bad_addr = (void *) fl + 1;
	invoke_test_syscall(0, cmd, bad_addr);
	pidns_print_leader();
	printf("%s(0, %s, %p) = %s\n",
	       TEST_SYSCALL_STR, name, bad_addr, errstr);
}

#ifdef NEED_TEST_FLOCK64_EINVAL
static void
test_flock64_einval(const int cmd, const char *name)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(struct flock64, fl);
	memset(fl, 0, sizeof(*fl));
	fl->l_type = F_RDLCK;
	fl->l_start = (TYPEOF_FLOCK_OFF_T) 0xdefaced1facefeedULL;
	fl->l_len = (TYPEOF_FLOCK_OFF_T) 0xdefaced2cafef00dULL;

	invoke_test_syscall(0, cmd, fl);
	pidns_print_leader();
	printf("%s(0, %s, {l_type=F_RDLCK, l_whence=SEEK_SET"
	       ", l_start=%jd, l_len=%jd}) = %s\n", TEST_SYSCALL_STR, name,
	       (intmax_t) fl->l_start, (intmax_t) fl->l_len, errstr);

	void *const bad_addr = (void *) fl + 1;
	invoke_test_syscall(0, cmd, bad_addr);
	pidns_print_leader();
	printf("%s(0, %s, %p) = %s\n",
	       TEST_SYSCALL_STR, name, bad_addr, errstr);
}
#endif /* NEED_TEST_FLOCK64_EINVAL */

static void
test_flock(void)
{
	TEST_FLOCK_EINVAL(F_SETLK);
	TEST_FLOCK_EINVAL(F_SETLKW);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct flock, fl);
	memset(fl, 0, sizeof(*fl));
	fl->l_type = F_RDLCK;
	fl->l_len = FILE_LEN;

	long rc = invoke_test_syscall(0, F_SETLK, fl);
	pidns_print_leader();
	printf("%s(0, F_SETLK, {l_type=F_RDLCK, l_whence=SEEK_SET"
	       ", l_start=0, l_len=%d}) = %s\n",
	       TEST_SYSCALL_STR, FILE_LEN, errstr);
	if (rc)
		return;

	invoke_test_syscall(0, F_GETLK, fl);
	pidns_print_leader();
	printf("%s(0, F_GETLK, {l_type=F_UNLCK, l_whence=SEEK_SET"
	       ", l_start=0, l_len=%d, l_pid=0}) = 0\n",
	       TEST_SYSCALL_STR, FILE_LEN);

	invoke_test_syscall(0, F_SETLKW, fl);
	pidns_print_leader();
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

	TAIL_ALLOC_OBJECT_CONST_PTR(struct flock64, fl);
	memset(fl, 0, sizeof(*fl));
	fl->l_type = F_RDLCK;
	fl->l_len = FILE_LEN;

	long rc = invoke_test_syscall(0, F_OFD_SETLK, fl);
	pidns_print_leader();
	printf("%s(0, F_OFD_SETLK, {l_type=F_RDLCK, l_whence=SEEK_SET"
	       ", l_start=0, l_len=%d}) = %s\n",
	       TEST_SYSCALL_STR, FILE_LEN, errstr);
	if (rc)
		return;

	invoke_test_syscall(0, F_OFD_GETLK, fl);
	pidns_print_leader();
	printf("%s(0, F_OFD_GETLK, {l_type=F_UNLCK, l_whence=SEEK_SET"
	       ", l_start=0, l_len=%d, l_pid=0}) = 0\n",
	       TEST_SYSCALL_STR, FILE_LEN);

	invoke_test_syscall(0, F_OFD_SETLKW, fl);
	pidns_print_leader();
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

static long
test_f_owner_ex_type_pid(const int cmd, const char *const cmd_name,
			 const int type, const char *const type_name,
			 enum pid_type pid_type, pid_t pid)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(struct f_owner_ex, fo);

	fo->type = type;
	fo->pid = pid;
	long rc = invoke_test_syscall(0, cmd, fo);
	pidns_print_leader();
	printf("%s(0, %s, {type=%s, pid=%d%s}) = %s\n",
	       TEST_SYSCALL_STR, cmd_name, type_name,
	       fo->pid, pidns_pid2str(pid_type), errstr);

	void *bad_addr = (void *) fo + 1;
	invoke_test_syscall(0, cmd, bad_addr);
	pidns_print_leader();
	printf("%s(0, %s, %p) = %s\n",
	       TEST_SYSCALL_STR, cmd_name, bad_addr, errstr);

	return rc;
}

static void
test_f_owner_ex_umove_or_printaddr(const int type, const char *const type_name,
				   enum pid_type pid_type, pid_t pid)
{
	long rc = test_f_owner_ex_type_pid(ARG_STR(F_SETOWN_EX),
					   type, type_name, pid_type, pid);
	if (!rc)
		test_f_owner_ex_type_pid(ARG_STR(F_GETOWN_EX),
					 type, type_name, pid_type, pid);
}

static void
test_f_owner_ex(void)
{
	struct {
		int type;
		const char *type_name;
		enum pid_type pid_type;
		pid_t pid;
	} a[] = {
		{ ARG_STR(F_OWNER_TID), PT_NONE, 1234567890 },
		{ ARG_STR(F_OWNER_PID), PT_NONE, 1234567890 },
		{ ARG_STR(F_OWNER_PGRP), PT_NONE, 1234567890 },
		{ ARG_STR(F_OWNER_TID), PT_TID, syscall(__NR_gettid) },
		{ ARG_STR(F_OWNER_PID), PT_TGID, getpid() },
		{ ARG_STR(F_OWNER_PGRP), PT_PGID, getpgid(0) },
	};

	for (unsigned int i = 0; i < ARRAY_SIZE(a); i++)
		test_f_owner_ex_umove_or_printaddr(a[i].type, a[i].type_name,
			a[i].pid_type, a[i].pid);
}
struct fcntl_cmd_check {
	int fd;
	int cmd;
	const char *cmd_str;
	long arg;
	const char *arg_str;
	void (*print_flags)(long rc);
};

static void
test_xetown(void)
{
	const int pid = getpid();
	const char *pid_str = pidns_pid2str(PT_TGID);

	invoke_test_syscall(0, F_SETOWN, (void *) (intptr_t) pid);
	pidns_print_leader();
	printf("%s(0, F_SETOWN, %d%s) = %s\n",
		TEST_SYSCALL_STR, pid, pid_str, errstr);

	invoke_test_syscall(0, F_GETOWN, NULL);
	pidns_print_leader();
	printf("%s(0, F_GETOWN) = %d%s\n",
		TEST_SYSCALL_STR, pid, pid_str);
}

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
	pidns_print_leader();
	printf("%s(%d, %s, %s) = %s\n",
	       TEST_SYSCALL_STR, check->fd,
	       check->cmd_str, check->arg_str, errstr);

	/* bad file fd */
	invoke_test_syscall(-1, check->cmd, (void *) check->arg);
	pidns_print_leader();
	printf("%s(-1, %s, %s) = %s\n",
	       TEST_SYSCALL_STR, check->cmd_str,
	       check->arg_str, errstr);
}

static void
test_other_get_cmd(const struct fcntl_cmd_check *check)
{
	long rc = invoke_test_syscall(check->fd, check->cmd, NULL);
	pidns_print_leader();
	printf("%s(%d, %s) = ",
	       TEST_SYSCALL_STR, check->fd, check->cmd_str);
	print_retval_flags(check, rc);

	/* bad file fd */
	invoke_test_syscall(-1, check->cmd, NULL);
	pidns_print_leader();
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
test_rw_hint_invalid_pointers(const int cmd, const char *const cmd_str,
			      uint64_t *const hint_ptr)
{
	invoke_test_syscall(0, cmd, NULL);
	pidns_print_leader();
	printf("%s(0, %s, NULL) = %s\n",
	       TEST_SYSCALL_STR, cmd_str, errstr);

	char *const ptr_range_begin = (char *) hint_ptr + 1;
	char *const ptr_range_end = (char *) (hint_ptr + 1);
	for (char *ptr = ptr_range_begin; ptr <= ptr_range_end; ++ptr) {
		invoke_test_syscall(0, cmd, ptr);
		pidns_print_leader();
		printf("%s(0, %s, %p) = %s\n",
		       TEST_SYSCALL_STR, cmd_str, ptr, errstr);
	}
}

static void
test_rw_hint_pair(const int set_cmd, const char *const set_cmd_str,
		  const int get_cmd, const char *const get_cmd_str)
{
	static const struct strval64 hints[] = {
		{ ARG_STR(RWH_WRITE_LIFE_NOT_SET) },
		{ ARG_STR(RWH_WRITE_LIFE_NONE) },
		{ ARG_STR(RWH_WRITE_LIFE_SHORT) },
		{ ARG_STR(RWH_WRITE_LIFE_MEDIUM) },
		{ ARG_STR(RWH_WRITE_LIFE_LONG) },
		{ ARG_STR(RWH_WRITE_LIFE_EXTREME) },
		{ ARG_STR(0xdeadc0de) " /* RWH_WRITE_LIFE_??? */" },
		{ ARG_STR(0xffffffff00000000) " /* RWH_WRITE_LIFE_??? */" },
	};

	TAIL_ALLOC_OBJECT_CONST_PTR(uint64_t, hint_ptr);

	/* Test failure case with an invalid fd */
	invoke_test_syscall(-1, get_cmd, hint_ptr);
	pidns_print_leader();
	printf("%s(-1, %s, %p) = %s\n",
	       TEST_SYSCALL_STR, get_cmd_str, hint_ptr, errstr);

	/* Test failure cases with a valid fd and invalid pointers */
	test_rw_hint_invalid_pointers(set_cmd, set_cmd_str, hint_ptr);
	test_rw_hint_invalid_pointers(get_cmd, get_cmd_str, hint_ptr);

	/* Loop over all hint values, testing SET followed by GET for each */
	for (unsigned int i = 0; i < ARRAY_SIZE(hints); i++) {
		*hint_ptr = hints[i].val;

		/* Test F_SET_* with an invalid fd */
		invoke_test_syscall(-1, set_cmd, hint_ptr);
		pidns_print_leader();
		printf("%s(-1, %s, [%s]) = %s\n",
		       TEST_SYSCALL_STR, set_cmd_str, hints[i].str, errstr);

		/* Test F_SET_* with a valid fd */
		long rc = invoke_test_syscall(0, set_cmd, hint_ptr);
		pidns_print_leader();
		printf("%s(0, %s, [%s]) = %s\n",
		       TEST_SYSCALL_STR, set_cmd_str, hints[i].str, errstr);

		/*
		 * If F_SET_* succeeded, test corresponding F_GET_*.
		 * We know what val to expect because we just set it.
		 * Invalid hint values usually cause F_SET_* to fail.
		 * If they don't, e.g. before Linux kernel commit
		 * v6.9-rc1~12^2~16, F_GET_* would return a truncated val,
		 * in that case we skip the corresponding F_GET_ invocation.
		 */
		if (rc == 0 && hints[i].val == (uint32_t) hints[i].val) {
			*hint_ptr = -1;
			rc = invoke_test_syscall(0, get_cmd, hint_ptr);
			pidns_print_leader();
			if (rc == 0) {
				/* Success: verify we got back what we set */
				printf("%s(0, %s, [%s]) = 0\n",
				       TEST_SYSCALL_STR, get_cmd_str,
				       hints[i].str);
			} else {
				/* GET failed: print address */
				printf("%s(0, %s, %p) = %s\n",
				       TEST_SYSCALL_STR, get_cmd_str,
				       hint_ptr, errstr);
			}
		}
		/* If SET failed (e.g., invalid hint value), we don't test GET */
	}
}

static void
test_rw_hints(void)
{
	test_rw_hint_pair(ARG_STR(F_SET_RW_HINT),
			  ARG_STR(F_GET_RW_HINT));

	test_rw_hint_pair(ARG_STR(F_SET_FILE_RW_HINT),
			  ARG_STR(F_GET_FILE_RW_HINT));
}

static void
test_delegation_invalid_pointers(const int cmd, const char *const cmd_str,
				 struct delegation *const deleg_ptr)
{
	invoke_test_syscall(0, cmd, NULL);
	pidns_print_leader();
	printf("%s(0, %s, NULL) = %s\n",
	       TEST_SYSCALL_STR, cmd_str, errstr);

	char *const ptr_range_begin = (char *) deleg_ptr + 1;
	char *const ptr_range_end = (char *) (deleg_ptr + 1);
	for (char *ptr = ptr_range_begin; ptr <= ptr_range_end; ++ptr) {
		invoke_test_syscall(0, cmd, ptr);
		pidns_print_leader();
		printf("%s(0, %s, %p) = %s\n",
		       TEST_SYSCALL_STR, cmd_str, ptr, errstr);
	}
}

static void
test_delegation_fields(struct delegation *const deleg,
		       const uint32_t d_flags, const char *const d_flags_str,
		       const uint16_t d_type, const char *const d_type_str,
		       const uint16_t d_pad, const char *const d_pad_str)
{
	memset(deleg, 0, sizeof(*deleg));
	deleg->d_flags = d_flags;
	deleg->d_type = d_type;
	deleg->__pad = d_pad;

	/* Test F_SETDELEG with an invalid fd */
	invoke_test_syscall(-1, F_SETDELEG, deleg);
	pidns_print_leader();
	printf("%s(-1, F_SETDELEG, {d_flags=%s, d_type=%s, __pad=%s}) = "
	       "%s\n", TEST_SYSCALL_STR, d_flags_str, d_type_str, d_pad_str,
	       errstr);

	/* Test F_SETDELEG with a valid fd */
	long rc = invoke_test_syscall(0, F_SETDELEG, deleg);
	pidns_print_leader();
	printf("%s(0, F_SETDELEG, {d_flags=%s, d_type=%s, __pad=%s}) = "
	       "%s\n", TEST_SYSCALL_STR, d_flags_str, d_type_str, d_pad_str,
	       errstr);

	/* If SET succeeded, test corresponding F_GETDELEG */
	/* Note: Invalid field values may cause SET to fail with EINVAL */
	if (!rc) {
		memset(deleg, 0, sizeof(*deleg));
		rc = invoke_test_syscall(0, F_GETDELEG, deleg);
		pidns_print_leader();
		if (rc == 0) {
			/* Success: assume d_type returned is the same as
			 * what was set */
			printf("%s(0, F_GETDELEG, "
			       "{d_flags=0, d_type=%s, __pad=0}) = 0\n",
			       TEST_SYSCALL_STR, d_type_str);
		} else {
			/* GET failed: print address */
			printf("%s(0, F_GETDELEG, %p) = %s\n",
			       TEST_SYSCALL_STR, deleg, errstr);
		}
	}
	/* If SET failed (e.g., invalid field values), we don't test GET */
}

static void
test_delegation(void)
{
	static const struct strval32 flags[] = {
		{ ARG_STR(0) },
		{ ARG_STR(0x1) },
		{ ARG_STR(0xffffffff) },
	};

	static const struct strval16 types[] = {
		{ ARG_STR(F_RDLCK) },
		{ ARG_STR(F_WRLCK) },
		{ ARG_STR(F_UNLCK) },
		{ ARG_STR(0xffff) " /* F_??? */" },
	};

	static const struct strval16 pads[] = {
		{ ARG_STR(0) },
		{ ARG_STR(0x1) },
		{ ARG_STR(0xffff) },
	};

	TAIL_ALLOC_OBJECT_CONST_PTR(struct delegation, deleg);

	/* Test failure case with an invalid fd */
	invoke_test_syscall(-1, F_GETDELEG, deleg);
	pidns_print_leader();
	printf("%s(-1, F_GETDELEG, %p) = %s\n",
	       TEST_SYSCALL_STR, deleg, errstr);

	/* Test failure cases with a valid fd and invalid pointers */
	test_delegation_invalid_pointers(F_SETDELEG, "F_SETDELEG", deleg);
	test_delegation_invalid_pointers(F_GETDELEG, "F_GETDELEG", deleg);

	/* Test all combinations of d_flags, d_type, and __pad */
	for (unsigned int i = 0; i < ARRAY_SIZE(flags); i++) {
		for (unsigned int j = 0; j < ARRAY_SIZE(types); j++) {
			for (unsigned int k = 0; k < ARRAY_SIZE(pads); k++) {
				test_delegation_fields(deleg,
					flags[i].val, flags[i].str,
					types[j].val, types[j].str,
					pads[k].val, pads[k].str);
			}
		}
	}
}

static void
test_fcntl_others(void)
{
	static const struct fcntl_cmd_check set_checks[] = {
		{ 0, ARG_STR(F_SETFD), ARG_STR(FD_CLOEXEC) },
		{ 0, ARG_STR(F_SETPIPE_SZ), ARG_STR(4097) },
		{ 0, ARG_STR(F_DUPFD), ARG_STR(0) },
		{ 0, ARG_STR(F_DUPFD_CLOEXEC), ARG_STR(0) },
		{ 0, ARG_STR(F_SETFL), ARG_STR(O_RDWR|O_LARGEFILE) },
		{ 0, ARG_STR(F_NOTIFY), ARG_STR(DN_ACCESS) },
		{ 0, ARG_STR(F_DUPFD_QUERY), ARG_STR(0) },
		{ 0, ARG_STR(F_CREATED_QUERY), ARG_STR(0) },
		{ 1, ARG_STR(F_SETLEASE), ARG_STR(F_RDLCK) },
		{ 0, ARG_STR(F_SETSIG), 0, "0" },
		{ 1, ARG_STR(F_SETSIG), 1, "SIGHUP" }
	};
	for (unsigned int i = 0; i < ARRAY_SIZE(set_checks); i++) {
		test_other_set_cmd(set_checks + i);
	}

	static const struct fcntl_cmd_check get_checks[] = {
		{ 0, ARG_STR(F_GETFD), .print_flags = print_flags_getfd },
		{ 1, ARG_STR(F_GETFD), .print_flags = print_flags_getfd },
		{ 0, ARG_STR(F_GETPIPE_SZ) },
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
	(void) close(0);
	if (ftruncate(create_tmpfile(O_RDWR), FILE_LEN))
		perror_msg_and_fail("ftruncate");
}

int
main(void)
{
	PIDNS_TEST_INIT;

	create_sample();
	test_flock();
	test_flock64();
	test_f_owner_ex();
	test_fcntl_others();
	test_xetown();
	test_rw_hints();
	test_delegation();

	pidns_print_leader();
	puts("+++ exited with 0 +++");
	return 0;
}
