/*
 * Check decoding of clone3 syscall.
 *
 * Copyright (c) 2019-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <errno.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include <linux/sched.h>

#ifdef HAVE_STRUCT_USER_DESC
# include <asm/ldt.h>
#endif

#include "scno.h"

#ifndef VERBOSE
# define VERBOSE 0
#endif
#ifndef RETVAL_INJECTED
# define RETVAL_INJECTED 0
#endif

enum validity_flag_bits {
	STRUCT_VALID_BIT,
	PIDFD_VALID_BIT,
	CHILD_TID_VALID_BIT,
	PARENT_TID_VALID_BIT,
	TLS_VALID_BIT,
};

enum validity_flags {
	FLAG(STRUCT_VALID),
	FLAG(PIDFD_VALID),
	FLAG(CHILD_TID_VALID),
	FLAG(PARENT_TID_VALID),
	FLAG(TLS_VALID),
};

#define MAX_SET_TID_SIZE 32

static const int child_exit_status = 42;

#if RETVAL_INJECTED
static const long injected_retval = 42;

# define INJ_STR " (INJECTED)\n"
#else /* !RETVAL_INJECTED */
# define INJ_STR "\n"
#endif /* RETVAL_INJECTED */

#define ERR(b_) ((1ULL << (b_)) + FAIL_BUILD_ON_ZERO((b_) < 64))


#if !RETVAL_INJECTED
static void
wait_cloned(int pid)
{
	int status;

	errno = 0;
	while (waitpid(pid, &status, WEXITED | __WCLONE) != pid) {
		if (errno != EINTR)
			perror_msg_and_fail("waitpid(%d)", pid);
	}
}
#endif

static long
do_clone3_(void *args, kernel_ulong_t size, uint64_t possible_errors, int line)
{
	long rc = syscall(__NR_clone3, args, size);

#if RETVAL_INJECTED
	if (rc != injected_retval)
		perror_msg_and_fail("%d: Unexpected injected return value "
				    "of a clone3() call (%ld instead of %ld)",
				    line, rc, injected_retval);
#else

	static int unimplemented_error = -1;

	if (!(possible_errors & ERR(0))) {
		if (rc >= 0)
			error_msg_and_fail("%d: Unexpected success"
					   " of a clone3() call", line);
		if (unimplemented_error < 0)
			unimplemented_error =
				(errno == EINVAL) ? ENOSYS : errno;
	}

	/*
	 * This code works as long as all the errors we care about (EFAULT
	 * and EINVAL so far) fit inside 64 bits, otherwise it should
	 * be rewritten.
	 */
	if (rc < 0 && errno != unimplemented_error
	    && (errno >= 64 || errno < 0 || !(ERR(errno) & possible_errors))) {
		perror_msg_and_fail("%d: Unexpected failure of a clone3() call"
				    " (got errno %d, expected errno bitmask"
				    " %#" PRIx64 ")",
				    line, errno, possible_errors);
	}

	if (!rc)
		_exit(child_exit_status);

	if (rc > 0 && ((struct clone_args *) args)->exit_signal)
		wait_cloned(rc);
#endif

	return rc;
}

#define do_clone3(args_, size_, errors_) \
	do_clone3_((args_), (size_), (errors_), __LINE__)

static void
print_addr64(const char *pfx, uint64_t addr)
{
	if (addr)
		printf("%s%#" PRIx64, pfx, addr);
	else
		printf("%sNULL", pfx);
}

static void
print_tls(const char *pfx, uint64_t arg_ptr, enum validity_flags vf)
{
#if defined HAVE_STRUCT_USER_DESC && defined __i386__
	if (!(vf & TLS_VALID)) {
		print_addr64(pfx, arg_ptr);
		return;
	}

	struct user_desc *arg = (struct user_desc *) (uintptr_t) arg_ptr;

	printf("%s{entry_number=%d"
	       ", base_addr=%#08x"
	       ", limit=%#08x"
	       ", seg_32bit=%u"
	       ", contents=%u"
	       ", read_exec_only=%u"
	       ", limit_in_pages=%u"
	       ", seg_not_present=%u"
	       ", useable=%u}",
	       pfx,
	       arg->entry_number,
	       arg->base_addr,
	       arg->limit,
	       arg->seg_32bit,
	       arg->contents,
	       arg->read_exec_only,
	       arg->limit_in_pages,
	       arg->seg_not_present,
	       arg->useable);
#else
	print_addr64(pfx, arg_ptr);
#endif
}

static void
print_set_tid(uint64_t set_tid, uint64_t set_tid_size)
{
	if (!set_tid || set_tid != (uintptr_t) set_tid ||
	    !set_tid_size || set_tid_size > MAX_SET_TID_SIZE) {
		print_addr64(", set_tid=", set_tid);
	} else {
		printf(", set_tid=");
		int *tids = (int *) (uintptr_t) set_tid;
		for (unsigned int i = 0; i < set_tid_size; ++i)
			printf("%s%d", i ? ", " : "[", tids[i]);
		printf("]");
	}

	printf(", set_tid_size=%" PRIu64, set_tid_size);
}

static void
print_clone3(struct clone_args *const arg, long rc, kernel_ulong_t sz,
	     enum validity_flags valid,
	     const char *flags_str, const char *es_str)
{
	int saved_errno = errno;

	printf("clone3(");
	if (!(valid & STRUCT_VALID)) {
		printf("%p", arg);
		goto out;
	}

#if XLAT_RAW
	printf("{flags=%#" PRIx64, (uint64_t) arg->flags);
#elif XLAT_VERBOSE
	if (flags_str[0] == '0')
		printf("{flags=%#" PRIx64, (uint64_t) arg->flags);
	else
		printf("{flags=%#" PRIx64 " /* %s */",
		       (uint64_t) arg->flags, flags_str);
#else
	printf("{flags=%s", flags_str);
#endif

	if (arg->flags & CLONE_PIDFD)
		print_addr64(", pidfd=", arg->pidfd);

	if (arg->flags & (CLONE_CHILD_SETTID | CLONE_CHILD_CLEARTID)) {
		if (valid & CHILD_TID_VALID)
			printf(", child_tid=[%d]",
			       *(int *) (uintptr_t) arg->child_tid);
		else
			print_addr64(", child_tid=", arg->child_tid);
	}

	if (arg->flags & CLONE_PARENT_SETTID)
		print_addr64(", parent_tid=", arg->parent_tid);

	printf(", exit_signal=%s", es_str);
	print_addr64(", stack=", arg->stack);
	printf(", stack_size=%" PRIx64, (uint64_t) arg->stack_size);

	if (arg->flags & CLONE_SETTLS)
		print_tls("tls=", arg->tls, valid);

	if (sz >= offsetofend(struct clone_args, set_tid_size) &&
	    (arg->set_tid || arg->set_tid_size))
		print_set_tid(arg->set_tid, arg->set_tid_size);

	if (sz > offsetof(struct clone_args, cgroup) &&
	    (arg->cgroup || arg->flags & CLONE_INTO_CGROUP))
		printf(", cgroup=%" PRIu64, (uint64_t) arg->cgroup);

	printf("}");

	if (rc < 0)
		goto out;

	bool comma = false;

	if (arg->flags & CLONE_PIDFD) {
		if (valid & PIDFD_VALID)
			printf(" => {pidfd=[%d]",
			       *(int *) (uintptr_t) arg->pidfd);
		else
			print_addr64(" => {pidfd=", arg->pidfd);

		comma = true;
	}

	if (arg->flags & CLONE_PARENT_SETTID) {
		printf(comma ? ", " : " => {");

		if (valid & PARENT_TID_VALID)
			printf("parent_tid=[%d]",
			       *(int *) (uintptr_t) arg->parent_tid);
		else
			print_addr64("parent_tid=", arg->parent_tid);

		comma = true;
	}

	if (comma)
		printf("}");

out:
	errno = saved_errno;
}

int
main(int argc, char *argv[])
{
	static const struct {
		struct clone_args args;
		uint64_t possible_errors;
		enum validity_flags vf;
		const char *flags_str;
		const char *es_str;
	} arg_vals[] = {
		{ { .flags = 0 },
			ERR(0), 0, "0", "0" },
		{ { .flags = CLONE_PARENT_SETTID },
			ERR(0), 0, "CLONE_PARENT_SETTID", "0" },

		/* check clone3_flags/clone_flags interoperation */
		{ { .flags = CLONE_CLEAR_SIGHAND },
			ERR(EINVAL) | ERR(0), 0, "CLONE_CLEAR_SIGHAND", "0" },
		{ { .flags = CLONE_PARENT_SETTID | CLONE_CLEAR_SIGHAND },
			ERR(EINVAL) | ERR(0), 0,
			"CLONE_PARENT_SETTID|CLONE_CLEAR_SIGHAND", "0" },

		{ { .set_tid = 0xfacefeedcafebabe },
			ERR(E2BIG) | ERR(EINVAL), 0, "0", "0" },
		{ { .set_tid_size = 0xfacecafefeedbabe },
			ERR(E2BIG) | ERR(EINVAL), 0, "0", "0" },
		{ { .set_tid = 0xfacefeedcafebabe,
		    .set_tid_size = MAX_SET_TID_SIZE + 1 },
			ERR(E2BIG) | ERR(EINVAL), 0, "0", "0" },
	};
	TAIL_ALLOC_OBJECT_CONST_PTR(struct clone_args, arg);
	const size_t arg1_size = offsetofend(struct clone_args, tls);
	struct clone_args *const arg1 = tail_alloc(arg1_size);
	const size_t arg2_size = sizeof(*arg) + 8;
	struct clone_args *const arg2 = tail_alloc(arg2_size);
	TAIL_ALLOC_OBJECT_CONST_PTR(int, pidfd);
	TAIL_ALLOC_OBJECT_CONST_PTR(int, child_tid);
	TAIL_ALLOC_OBJECT_CONST_PTR(int, parent_tid);
	int *const tids = tail_alloc(sizeof(*tids) * MAX_SET_TID_SIZE);
	long rc;

#if defined HAVE_STRUCT_USER_DESC
	TAIL_ALLOC_OBJECT_CONST_PTR(struct user_desc, tls);

	fill_memory(tls, sizeof(*tls));
#else
	TAIL_ALLOC_OBJECT_CONST_PTR(int, tls);
#endif

	*pidfd = 0xbadc0ded;
	*child_tid = 0xdeadface;
	*parent_tid = 0xfeedbeef;
	fill_memory(tids, sizeof(*tids) * MAX_SET_TID_SIZE);

	rc = do_clone3(NULL, 0, ERR(EINVAL));
	printf("clone3(NULL, 0) = %s" INJ_STR, sprintrc(rc));

	rc = do_clone3(arg + 1, sizeof(*arg), ERR(EFAULT));
	printf("clone3(%p, %zu) = %s" INJ_STR,
	       arg + 1, sizeof(*arg), sprintrc(rc));

	size_t short_size = arg1_size - sizeof(uint64_t);
	char *short_start = (char *) arg1 + sizeof(uint64_t);
	rc = do_clone3(short_start, short_size, ERR(EINVAL));
	printf("clone3(%p, %zu) = %s" INJ_STR,
	       short_start, short_size, sprintrc(rc));


	memset(arg, 0, sizeof(*arg));
	memset(arg1, 0, arg1_size);
	memset(arg2, 0, arg2_size);

	rc = do_clone3(arg, 64, ERR(0));
	printf("clone3({flags=0, exit_signal=0, stack=NULL, stack_size=0}, 64)"
	       " = %s" INJ_STR,
	       sprintrc(rc));

	rc = do_clone3(arg, sizeof(*arg) + 8, ERR(EFAULT));
	printf("clone3({flags=0, exit_signal=0, stack=NULL, stack_size=0, ???}"
#if RETVAL_INJECTED
	       " => {???}"
#endif
	       ", %zu) = %s" INJ_STR,
	       sizeof(*arg) + 8, sprintrc(rc));

	rc = do_clone3(arg1, arg1_size, ERR(0));
	printf("clone3({flags=0, exit_signal=0, stack=NULL, stack_size=0}"
	       ", %zu) = %s" INJ_STR,
	       arg1_size, sprintrc(rc));

	rc = do_clone3(arg2, arg2_size, ERR(0));
	printf("clone3({flags=0, exit_signal=0, stack=NULL, stack_size=0}"
	       ", %zu) = %s" INJ_STR,
	       arg2_size, sprintrc(rc));

	arg->set_tid = (uintptr_t) tids;
	arg->set_tid_size = MAX_SET_TID_SIZE;
	rc = do_clone3(arg, sizeof(*arg), ERR(E2BIG) | ERR(EINVAL));
	print_clone3(arg, rc, sizeof(*arg), STRUCT_VALID, "0", "0");
	printf(", %zu) = %s" INJ_STR, sizeof(*arg), sprintrc(rc));
	memset(arg, 0, sizeof(*arg));

	arg->cgroup = 0xfacefeedbadc0ded;
	rc = do_clone3(arg, sizeof(*arg), ERR(0) | ERR(E2BIG));
	print_clone3(arg, rc, sizeof(*arg), STRUCT_VALID, "0", "0");
	printf(", %zu) = %s" INJ_STR, sizeof(*arg), sprintrc(rc));
	memset(arg, 0, sizeof(*arg));

	arg->flags = CLONE_INTO_CGROUP;
	rc = do_clone3(arg, sizeof(*arg), ERR(0) | ERR(EINVAL) | ERR(EBADF));
	print_clone3(arg, rc, sizeof(*arg), STRUCT_VALID,
		     "CLONE_INTO_CGROUP", "0");
	printf(", %zu) = %s" INJ_STR, sizeof(*arg), sprintrc(rc));
	memset(arg, 0, sizeof(*arg));

	/*
	 * NB: the following check is purposefully fragile (it will break
	 *     when system's struct clone_args has additional fields,
	 *     so it signalises that the decoder needs to be updated.
	 */
	arg2[1].flags = 0xfacefeeddeadc0de;
	arg2->exit_signal = 0xdeadface00000000ULL | SIGCHLD;
	rc = do_clone3(arg2, sizeof(*arg2) + 8, ERR(E2BIG));
	printf("clone3({flags=0, exit_signal=%llu, stack=NULL, stack_size=0"
	       ", /* bytes %zu..%zu */ "
	       BE_LE("\"\\xfa\\xce\\xfe\\xed\\xde\\xad\\xc0\\xde\"",
		     "\"\\xde\\xc0\\xad\\xde\\xed\\xfe\\xce\\xfa\"")
#if RETVAL_INJECTED
	       "} => {/* bytes %zu..%zu */ "
	       BE_LE("\"\\xfa\\xce\\xfe\\xed\\xde\\xad\\xc0\\xde\"",
		     "\"\\xde\\xc0\\xad\\xde\\xed\\xfe\\xce\\xfa\"")
#endif /* RETVAL_INJECTED */
	       "}, %zu) = %s" INJ_STR,
	       0xdeadface00000000ULL | SIGCHLD,
	       sizeof(*arg2), sizeof(*arg2) + 7,
#if RETVAL_INJECTED
	       sizeof(*arg2), sizeof(*arg2) + 7,
#endif
	       sizeof(*arg2) + 8, sprintrc(rc));

	arg2->exit_signal = 0xdeadc0de;
	rc = do_clone3(arg2, sizeof(*arg) + 16, ERR(E2BIG));
	printf("clone3({flags=0, exit_signal=3735929054, stack=NULL"
	       ", stack_size=0, ???}"
#if RETVAL_INJECTED
	       " => {???}"
#endif
	       ", %zu) = %s" INJ_STR,
	       sizeof(*arg) + 16, sprintrc(rc));

	arg->flags = 0xface3eefbeefc0de;
	arg->exit_signal = 0x1e55c0de;
	rc = do_clone3(arg, 64, ERR(EINVAL));
	printf("clone3({flags=%s, child_tid=NULL, exit_signal=508936414"
	       ", stack=NULL, stack_size=0, tls=NULL}, 64) = %s" INJ_STR,
	       XLAT_KNOWN(0xface3eefbeefc0de, "CLONE_VFORK|CLONE_PARENT"
	       "|CLONE_THREAD|CLONE_NEWNS|CLONE_SYSVSEM|CLONE_SETTLS"
	       "|CLONE_CHILD_CLEARTID|CLONE_UNTRACED|CLONE_NEWCGROUP"
	       "|CLONE_NEWUTS|CLONE_NEWIPC|CLONE_NEWUSER|CLONE_NEWPID|CLONE_IO"
	       "|CLONE_NEWTIME|CLONE_CLEAR_SIGHAND|CLONE_INTO_CGROUP"
	       "|0xface3eec0040005e"),
	       sprintrc(rc));

	arg->flags = 0xdec0deac0040007fULL;
	arg->exit_signal = 250;
	arg->stack = 0xface1e55beeff00dULL;
	arg->stack_size = 0xcaffeedefacedca7ULL;
	rc = do_clone3(arg, 64, ERR(EINVAL));
	printf("clone3({flags=%s, exit_signal=250"
	       ", stack=0xface1e55beeff00d, stack_size=0xcaffeedefacedca7}, 64)"
	       " = %s" INJ_STR,
	       XLAT_UNKNOWN(0xdec0deac0040007f, "CLONE_???"),
	       sprintrc(rc));

	arg->exit_signal = SIGCHLD;

	struct {
		uint64_t flag;
		const char *flag_str;
		uint64_t *field;
		const char *field_name;
		int *ptr;
		bool deref_exiting;
	} pid_fields[] = {
		{ ARG_STR(CLONE_PIDFD),
			(uint64_t *) &arg->pidfd,
			"pidfd", pidfd, true },
		{ ARG_STR(CLONE_CHILD_SETTID),
			(uint64_t *) &arg->child_tid,
			"child_tid", child_tid },
		{ ARG_STR(CLONE_CHILD_CLEARTID),
			(uint64_t *) &arg->child_tid,
			"child_tid", child_tid },
		{ ARG_STR(CLONE_CHILD_CLEARTID|CLONE_CHILD_SETTID),
			(uint64_t *) &arg->child_tid,
			"child_tid", child_tid },
		{ ARG_STR(CLONE_PARENT_SETTID),
			(uint64_t *) &arg->parent_tid,
			"parent_tid", parent_tid, true },
	};

	for (size_t i = 0; i < ARRAY_SIZE(pid_fields); i++) {
		char flag_str[128];
		const char *rc_str;

		arg->flags = 0xbad0000000000001ULL | pid_fields[i].flag;

#if XLAT_RAW
		snprintf(flag_str, sizeof(flag_str), "%#" PRIx64,
			 (uint64_t) arg->flags);
#elif XLAT_VERBOSE
		snprintf(flag_str, sizeof(flag_str),
			 "%#" PRIx64 " /* %s|0xbad0000000000001 */",
			 (uint64_t) arg->flags, pid_fields[i].flag_str);
#else
		snprintf(flag_str, sizeof(flag_str), "%s|0xbad0000000000001",
			 pid_fields[i].flag_str);
#endif

		pid_fields[i].field[0] = 0;
		rc = do_clone3(arg, 64, ERR(EINVAL));
		rc_str = sprintrc(rc);
		printf("clone3({flags=%s, %s=NULL"
		       ", exit_signal=" XLAT_KNOWN(SIGCHLD, "SIGCHLD")
		       ", stack=0xface1e55beeff00d"
		       ", stack_size=0xcaffeedefacedca7}",
		       flag_str, pid_fields[i].field_name);
#if RETVAL_INJECTED
		if (pid_fields[i].deref_exiting)
			printf(" => {%s=NULL}", pid_fields[i].field_name);
#endif /* RETVAL_INJECTED */
		printf(", 64) = %s" INJ_STR, rc_str);

		pid_fields[i].field[0] = (uintptr_t) (pid_fields[i].ptr + 1);
		rc = do_clone3(arg, 64, ERR(EINVAL));
		rc_str = sprintrc(rc);
		printf("clone3({flags=%s, %s=%p"
		       ", exit_signal=" XLAT_KNOWN(SIGCHLD, "SIGCHLD")
		       ", stack=0xface1e55beeff00d"
		       ", stack_size=0xcaffeedefacedca7}",
		       flag_str, pid_fields[i].field_name,
		       pid_fields[i].ptr + 1);
#if RETVAL_INJECTED
		if (pid_fields[i].deref_exiting)
			printf(" => {%s=%p}",
			       pid_fields[i].field_name, pid_fields[i].ptr + 1);
#endif /* RETVAL_INJECTED */
		printf(", 64) = %s" INJ_STR, rc_str);

		pid_fields[i].field[0] = (uintptr_t) pid_fields[i].ptr;
		rc = do_clone3(arg, 64, ERR(EINVAL));
		rc_str = sprintrc(rc);
		printf("clone3({flags=%s, %s=%p"
		       ", exit_signal=" XLAT_KNOWN(SIGCHLD, "SIGCHLD")
		       ", stack=0xface1e55beeff00d"
		       ", stack_size=0xcaffeedefacedca7}",
		       flag_str, pid_fields[i].field_name,
		       pid_fields[i].ptr);
#if RETVAL_INJECTED
		if (pid_fields[i].deref_exiting)
			printf(" => {%s=[%d]}",
			       pid_fields[i].field_name, *pid_fields[i].ptr);
#endif /* RETVAL_INJECTED */
		printf(", 64) = %s" INJ_STR, rc_str);
	}

	arg->flags = 0xbad0000000000001ULL | CLONE_SETTLS;
	rc = do_clone3(arg, 64, ERR(EINVAL));
	printf("clone3({flags="
	       XLAT_KNOWN(0xbad0000000080001, "CLONE_SETTLS|0xbad0000000000001")
	       ", exit_signal=" XLAT_KNOWN(SIGCHLD, "SIGCHLD")
	       ", stack=0xface1e55beeff00d"
	       ", stack_size=0xcaffeedefacedca7, tls=NULL}, 64) = %s" INJ_STR,
	       sprintrc(rc));

	arg->tls = (uintptr_t) (tls + 1);
	rc = do_clone3(arg, 64, ERR(EINVAL));
	printf("clone3({flags="
	       XLAT_KNOWN(0xbad0000000080001, "CLONE_SETTLS|0xbad0000000000001")
	       ", exit_signal=" XLAT_KNOWN(SIGCHLD, "SIGCHLD")
	       ", stack=0xface1e55beeff00d"
	       ", stack_size=0xcaffeedefacedca7, tls=%p}, 64) = %s" INJ_STR,
	       tls + 1, sprintrc(rc));

	arg->tls = (uintptr_t) tls;
	rc = do_clone3(arg, 64, ERR(EINVAL));
	printf("clone3({flags="
	       XLAT_KNOWN(0xbad0000000080001, "CLONE_SETTLS|0xbad0000000000001")
	       ", exit_signal=" XLAT_KNOWN(SIGCHLD, "SIGCHLD")
	       ", stack=0xface1e55beeff00d, stack_size=0xcaffeedefacedca7, tls="
#if defined HAVE_STRUCT_USER_DESC && defined __i386__
	       "{entry_number=2206368128, base_addr=0x87868584"
	       ", limit=0x8b8a8988, seg_32bit=0, contents=2, read_exec_only=1"
	       ", limit_in_pages=0, seg_not_present=0, useable=0}"
#else
	       "%p"
#endif
	       "}, 64) = %s" INJ_STR,
#if !defined HAVE_STRUCT_USER_DESC || !defined __i386__
	       tls,
#endif
	       sprintrc(rc));

	for (size_t i = 0; i < ARRAY_SIZE(arg_vals); i++) {
		memcpy(arg, &arg_vals[i].args, sizeof(*arg));

		rc = do_clone3(arg, sizeof(*arg), arg_vals[i].possible_errors);
		print_clone3(arg, rc, sizeof(*arg),
			     arg_vals[i].vf | STRUCT_VALID,
			     arg_vals[i].flags_str, arg_vals[i].es_str);
		printf(", %zu) = %s" INJ_STR, sizeof(*arg), sprintrc(rc));
	}

	puts("+++ exited with 0 +++");

	return 0;
}
