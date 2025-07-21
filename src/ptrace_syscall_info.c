/*
 * Copyright (c) 2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2018-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "kill_save_errno.h"
#include "ptrace.h"
#include "ptrace_syscall_info.h"
#include "scno.h"

#include <signal.h>
#include <sys/wait.h>

#include "xlat/ptrace_syscall_info_op.h"

bool ptrace_get_syscall_info_supported;

static const unsigned int expected_exit_size =
	offsetofend(struct_ptrace_syscall_info, exit.is_error);
static const unsigned int expected_seccomp_size =
	offsetofend(struct_ptrace_syscall_info, seccomp.ret_data);

#ifdef HAVE_FORK

# define FAIL	do { ptrace_stop = -1U; goto done; } while (0)

static const kernel_ulong_t dummy_syscall_args[] = {
	(kernel_ulong_t) 0xdad0bef0bad0fed0ULL,
	(kernel_ulong_t) 0xdad1bef1bad1fed1ULL,
	(kernel_ulong_t) 0xdad2bef2bad2fed2ULL,
	(kernel_ulong_t) 0xdad3bef3bad3fed3ULL,
	(kernel_ulong_t) 0xdad4bef4bad4fed4ULL,
	(kernel_ulong_t) 0xdad5bef5bad5fed5ULL,
};

static const unsigned int expected_none_size =
	offsetof(struct_ptrace_syscall_info, entry);
static const unsigned int expected_entry_size =
	offsetofend(struct_ptrace_syscall_info, entry.args);

struct si_entry {
	int nr;
	kernel_ulong_t args[6];
};
struct si_exit {
	unsigned int is_error;
	int rval;
};

static int
kill_tracee(pid_t pid)
{
	return kill_save_errno(pid, SIGKILL);
}

static bool
check_psi_none(const struct_ptrace_syscall_info *info,
	       const unsigned int psi_size,
	       const char *text,
	       unsigned int ptrace_stop)
{
	if (psi_size < expected_none_size ||
	    info->op != PTRACE_SYSCALL_INFO_NONE ||
	    !info->arch ||
	    !info->instruction_pointer ||
	    !info->stack_pointer) {
		debug_func_msg("%s: ptrace stop #%d: signal stop mismatch",
			       text, ptrace_stop);
		return false;
	}
	return true;
}

static bool
check_psi_entry(const struct_ptrace_syscall_info *info,
		const unsigned int psi_size,
		const struct si_entry *exp_entry,
		const char *text,
		unsigned int ptrace_stop)
{
	int exp_nr = exp_entry->nr;
# if defined __s390__ || defined __s390x__
	/* s390 is the only architecture that has 16-bit syscall numbers */
	exp_nr &= 0xffff;
# endif

	if (psi_size < expected_entry_size ||
	    info->op != PTRACE_SYSCALL_INFO_ENTRY ||
	    !info->arch ||
	    !info->instruction_pointer ||
	    !info->stack_pointer) {
		debug_func_msg("%s: ptrace stop #%d: entry stop mismatch",
			       text, ptrace_stop);
		return false;
	}
	if (info->entry.nr != (typeof(info->entry.nr)) exp_nr) {
		debug_func_msg("%s: ptrace stop #%d: expected nr=%d, got nr=%ju",
			       text, ptrace_stop, exp_nr,
			       (uintmax_t) info->entry.nr);
		return false;
	}
	for (unsigned int i = 0; i < ARRAY_SIZE(exp_entry->args); ++i) {
		if (info->entry.args[i] != exp_entry->args[i]) {
			debug_func_msg("%s: ptrace stop #%d: args[%u] mismatch:"
				       "expected %#jx, got %#jx",
				       text, ptrace_stop, i,
				       (uintmax_t) exp_entry->args[i],
				       (uintmax_t) info->entry.args[i]);
			return false;
		}
	}
	return true;
}

static bool
check_psi_exit(const struct_ptrace_syscall_info *info,
	       const unsigned int psi_size,
	       const struct si_exit *exp_exit,
	       const char *text,
	       unsigned int ptrace_stop)
{
	if (psi_size < expected_exit_size ||
	    info->op != PTRACE_SYSCALL_INFO_EXIT ||
	    !info->arch ||
	    !info->instruction_pointer ||
	    !info->stack_pointer ||
	    info->exit.is_error != exp_exit->is_error ||
	    info->exit.rval != exp_exit->rval) {
		debug_func_msg("%s: ptrace stop #%d: exit stop mismatch",
			       text, ptrace_stop);
		return false;
	}
	return true;
}

static bool
do_test_ptrace_get_syscall_info(void)
{
	const struct si_entry gsi_entry[] = {
		/* a sequence of architecture-agnostic syscalls */
		{
			__NR_chdir,
			{
				(uintptr_t) "",
				dummy_syscall_args[1],
				dummy_syscall_args[2],
				dummy_syscall_args[3],
				dummy_syscall_args[4],
				dummy_syscall_args[5]
			}
		}, {
			__NR_gettid,
			{
				dummy_syscall_args[0],
				dummy_syscall_args[1],
				dummy_syscall_args[2],
				dummy_syscall_args[3],
				dummy_syscall_args[4],
				dummy_syscall_args[5]
			}
		}, {
			__NR_exit_group,
			{
				0,
				dummy_syscall_args[1],
				dummy_syscall_args[2],
				dummy_syscall_args[3],
				dummy_syscall_args[4],
				dummy_syscall_args[5]
			}
		}
	};

	int tracee_pid = fork();
	if (tracee_pid < 0)
		perror_func_msg_and_die("fork");

	if (tracee_pid == 0) {
		/* get the pid before PTRACE_TRACEME */
		tracee_pid = getpid();
		if (ptrace(PTRACE_TRACEME, 0L, 0L, 0L) < 0) {
			/* exit with a nonzero exit status */
			perror_func_msg_and_die("PTRACE_TRACEME");
		}
		kill(tracee_pid, SIGSTOP);
		for (unsigned int i = 0; i < ARRAY_SIZE(gsi_entry); ++i) {
			syscall(gsi_entry[i].nr,
				gsi_entry[i].args[0], gsi_entry[i].args[1],
				gsi_entry[i].args[2], gsi_entry[i].args[3],
				gsi_entry[i].args[4], gsi_entry[i].args[5]);
		}
		/* unreachable */
		_exit(1);
	}

	const struct si_exit gsi_exit[] = {
		{ 1, -ENOENT },		/* chdir */
		{ 0, tracee_pid }	/* gettid */
	};

	unsigned int ptrace_stop;

	for (ptrace_stop = 0; ; ++ptrace_stop) {
		struct_ptrace_syscall_info info = {
			.op = 0xff	/* invalid PTRACE_SYSCALL_INFO_* op */
		};
		const size_t size = sizeof(info);
		int status;
		long rc = waitpid(tracee_pid, &status, 0);
		if (rc != tracee_pid) {
			/* cannot happen */
			kill_tracee(tracee_pid);
			perror_func_msg_and_die("ptrace stop #%d:"
						" unexpected wait result %ld",
						ptrace_stop, rc);
		}
		if (WIFEXITED(status)) {
			/* the tracee is no more */
			tracee_pid = 0;
			if (WEXITSTATUS(status) == 0)
				break;
			debug_func_msg("ptrace stop #%d:"
				       " unexpected exit status %u",
				       ptrace_stop, WEXITSTATUS(status));
			FAIL;
		}
		if (WIFSIGNALED(status)) {
			/* the tracee is no more */
			tracee_pid = 0;
			debug_func_msg("ptrace stop #%d: unexpected signal %u",
				       ptrace_stop, WTERMSIG(status));
			FAIL;
		}
		if (!WIFSTOPPED(status)) {
			/* cannot happen */
			kill_tracee(tracee_pid);
			error_func_msg_and_die("ptrace stop #%d:"
					       " unexpected wait status %#x",
					       ptrace_stop, status);
		}

		switch (WSTOPSIG(status)) {
		case SIGSTOP:
			if (ptrace_stop) {
				debug_func_msg("ptrace stop #%d:"
					       " unexpected signal stop",
					       ptrace_stop);
				FAIL;
			}
			if (ptrace(PTRACE_SETOPTIONS, tracee_pid,
				   0L, PTRACE_O_TRACESYSGOOD) < 0) {
				/* cannot happen */
				kill_tracee(tracee_pid);
				perror_func_msg_and_die("PTRACE_SETOPTIONS");
			}
			rc = ptrace(PTRACE_GET_SYSCALL_INFO, tracee_pid,
				    (void *) size, &info);
			if (rc < 0) {
				debug_func_perror_msg("PTRACE_GET_SYSCALL_INFO");
				FAIL;
			}
			if (!check_psi_none(&info, rc,
					"PTRACE_GET_SYSCALL_INFO",
					ptrace_stop))
				FAIL;
			break;

		case SIGTRAP | 0x80:
			rc = ptrace(PTRACE_GET_SYSCALL_INFO, tracee_pid,
				    (void *) size, &info);
			if (rc < 0) {
				debug_func_perror_msg("ptrace stop #%d"
					": PTRACE_GET_SYSCALL_INFO",
					ptrace_stop);
				FAIL;
			}
			switch (ptrace_stop) {
			case 1: /* entering chdir */
			case 3: /* entering gettid */
			case 5: /* entering exit_group */
				if (!check_psi_entry(&info, rc,
						&gsi_entry[ptrace_stop / 2],
						"PTRACE_GET_SYSCALL_INFO",
						ptrace_stop))
					FAIL;
				break;
			case 2: /* exiting chdir */
			case 4: /* exiting gettid */
				if (!check_psi_exit(&info, rc,
						&gsi_exit[ptrace_stop / 2 - 1],
						"PTRACE_GET_SYSCALL_INFO",
						ptrace_stop))
					FAIL;
				break;
			default:
				debug_func_msg("ptrace stop #%d:"
					       " unexpected syscall stop",
					       ptrace_stop);
				FAIL;
			}
			break;

		default:
			debug_func_msg("ptrace stop #%d:"
				       " unexpected stop signal %#x",
				       ptrace_stop, WSTOPSIG(status));
			FAIL;
		}

		if (ptrace(PTRACE_SYSCALL, tracee_pid, 0L, 0L) < 0) {
			/* cannot happen */
			kill_tracee(tracee_pid);
			perror_func_msg_and_die("PTRACE_SYSCALL");
		}
	}

done:
	if (tracee_pid) {
		kill_tracee(tracee_pid);
		waitpid(tracee_pid, NULL, 0);
		ptrace_stop = -1U;
	}

	return ptrace_stop == ARRAY_SIZE(gsi_entry) + ARRAY_SIZE(gsi_exit) + 1;
}

#endif /* HAVE_FORK */

/*
 * Test that PTRACE_GET_SYSCALL_INFO API is supported by the kernel, and
 * that the semantics implemented in the kernel matches our expectations.
 */
bool
test_ptrace_get_syscall_info(void)
{
	/*
	 * NOMMU provides no forks necessary for PTRACE_GET_SYSCALL_INFO test,
	 * leave the default unchanged.
	 */
#ifdef HAVE_FORK
	ptrace_get_syscall_info_supported =
		do_test_ptrace_get_syscall_info();
#endif /* HAVE_FORK */

	if (ptrace_get_syscall_info_supported)
		debug_msg("PTRACE_GET_SYSCALL_INFO works");
	else
		debug_msg("PTRACE_GET_SYSCALL_INFO does not work");

	return ptrace_get_syscall_info_supported;
}

static void
print_psi_entry(const typeof_field(struct_ptrace_syscall_info, entry) *const p,
		const kernel_ulong_t fetch_size, struct tcb *const tcp,
		unsigned int arch)
{
	tprint_struct_begin();
	PRINT_FIELD_SYSCALL_NAME(*p, nr, arch);
	const kernel_ulong_t nargs =
		(fetch_size - offsetof(struct_ptrace_syscall_info, entry.args))
		/ sizeof(p->args[0]);
	if (nargs) {
		tprint_struct_next();
		PRINT_FIELD_ARRAY_UPTO(*p, args, nargs, tcp,
				       print_xint_array_member);
	}
	tprint_struct_end();
}

static void
print_psi_seccomp(const typeof_field(struct_ptrace_syscall_info, seccomp) *const p,
		const kernel_ulong_t fetch_size, struct tcb *const tcp,
		unsigned int arch)
{
	tprint_struct_begin();
	PRINT_FIELD_SYSCALL_NAME(*p, nr, arch);
	const kernel_ulong_t nargs =
		(fetch_size - offsetof(struct_ptrace_syscall_info, seccomp.args))
		/ sizeof(p->args[0]);
	if (nargs) {
		tprint_struct_next();
		PRINT_FIELD_ARRAY_UPTO(*p, args, nargs, tcp,
				       print_xint_array_member);
	}
	if (fetch_size >= expected_seccomp_size) {
		tprint_struct_next();
		PRINT_FIELD_U(*p, ret_data);
	}
	tprint_struct_end();
}

static void
print_psi_exit(const typeof_field(struct_ptrace_syscall_info, exit) *const p,
	       const kernel_ulong_t fetch_size, struct tcb *const tcp)
{
	tprint_struct_begin();
	if (fetch_size >= expected_exit_size && p->is_error) {
		PRINT_FIELD_ERR_D(*p, rval);
	} else {
		PRINT_FIELD_D(*p, rval);
	}
	if (fetch_size >= expected_exit_size) {
		tprint_struct_next();
		PRINT_FIELD_U(*p, is_error);
	}
	tprint_struct_end();
}

void
print_ptrace_syscall_info(struct tcb *tcp, kernel_ulong_t addr,
			  kernel_ulong_t user_len,
			  kernel_ulong_t kernel_len)
{
	struct_ptrace_syscall_info info = { 0 };
	kernel_ulong_t ret_len = MIN(user_len, kernel_len);
	kernel_ulong_t fetch_size = MIN(ret_len, sizeof(info));

	if (!fetch_size || !tfetch_mem(tcp, addr, fetch_size, &info)) {
		printaddr(addr);
		return;
	}

	tprint_struct_begin();
	PRINT_FIELD_XVAL(info, op, ptrace_syscall_info_op,
			 "PTRACE_SYSCALL_INFO_???");

	if (fetch_size <= offsetof(struct_ptrace_syscall_info, reserved))
		goto printed;
	if (info.reserved) {
		tprint_struct_next();
		PRINT_FIELD_X(info, reserved);
	}

	if (fetch_size <= offsetof(struct_ptrace_syscall_info, flags))
		goto printed;
	if (info.flags) {
		tprint_struct_next();
		PRINT_FIELD_X(info, flags);
	}

	if (fetch_size < offsetofend(struct_ptrace_syscall_info, arch))
		goto printed;
	tprint_struct_next();
	PRINT_FIELD_XVAL(info, arch, audit_arch, "AUDIT_ARCH_???");

	if (fetch_size < offsetofend(struct_ptrace_syscall_info,
				     instruction_pointer))
		goto printed;
	tprint_struct_next();
	PRINT_FIELD_ADDR64(info, instruction_pointer);

	if (fetch_size < offsetofend(struct_ptrace_syscall_info, stack_pointer))
		goto printed;
	tprint_struct_next();
	PRINT_FIELD_ADDR64(info, stack_pointer);

	if (fetch_size < offsetofend(struct_ptrace_syscall_info, entry.nr))
		goto printed;

	switch(info.op) {
		case PTRACE_SYSCALL_INFO_ENTRY:
			tprint_struct_next();
			PRINT_FIELD_OBJ_PTR(info, entry,
					    print_psi_entry, fetch_size, tcp,
					    info.arch);
			break;
		case PTRACE_SYSCALL_INFO_SECCOMP:
			tprint_struct_next();
			PRINT_FIELD_OBJ_PTR(info, seccomp,
					    print_psi_seccomp, fetch_size, tcp,
					    info.arch);
			break;
		case PTRACE_SYSCALL_INFO_EXIT:
			tprint_struct_next();
			PRINT_FIELD_OBJ_PTR(info, exit,
					    print_psi_exit, fetch_size, tcp);
			break;
	}

printed:
	tprint_struct_end();
}
