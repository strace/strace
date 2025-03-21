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

#define FAIL	do { ptrace_stop = -1U; goto done; } while (0)

#ifdef HAVE_FORK
static int
kill_tracee(pid_t pid)
{
	return kill_save_errno(pid, SIGKILL);
}

static const unsigned int expected_none_size =
	offsetof(struct_ptrace_syscall_info, entry);
static const unsigned int expected_entry_size =
	offsetofend(struct_ptrace_syscall_info, entry.args);
#endif /* HAVE_FORK */
static const unsigned int expected_exit_size =
	offsetofend(struct_ptrace_syscall_info, exit.is_error);
static const unsigned int expected_seccomp_size =
	offsetofend(struct_ptrace_syscall_info, seccomp.ret_data);

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
	const kernel_ulong_t args[][7] = {
		/* a sequence of architecture-agnostic syscalls */
		{
			__NR_chdir,
			(uintptr_t) "",
			(kernel_ulong_t) 0xdad1bef1bad1fed1ULL,
			(kernel_ulong_t) 0xdad2bef2bad2fed2ULL,
			(kernel_ulong_t) 0xdad3bef3bad3fed3ULL,
			(kernel_ulong_t) 0xdad4bef4bad4fed4ULL,
			(kernel_ulong_t) 0xdad5bef5bad5fed5ULL
		},
		{
			__NR_gettid,
			(kernel_ulong_t) 0xdad0bef0caf0bea0ULL,
			(kernel_ulong_t) 0xdad1bef1caf1bea1ULL,
			(kernel_ulong_t) 0xdad2bef2caf2bea2ULL,
			(kernel_ulong_t) 0xdad3bef3caf3bea3ULL,
			(kernel_ulong_t) 0xdad4bef4caf4bea4ULL,
			(kernel_ulong_t) 0xdad5bef5caf5bea5ULL
		},
		{
			__NR_exit_group,
			0,
			(kernel_ulong_t) 0xdad1bef1fac1c0d1ULL,
			(kernel_ulong_t) 0xdad2bef2fac2c0d2ULL,
			(kernel_ulong_t) 0xdad3bef3fac3c0d3ULL,
			(kernel_ulong_t) 0xdad4bef4fac4c0d4ULL,
			(kernel_ulong_t) 0xdad5bef5fac5c0d5ULL
		}
	};
	const kernel_ulong_t *exp_args;

	int pid = fork();
	if (pid < 0)
		perror_func_msg_and_die("fork");

	if (pid == 0) {
		/* get the pid before PTRACE_TRACEME */
		pid = getpid();
		if (ptrace(PTRACE_TRACEME, 0L, 0L, 0L) < 0) {
			/* exit with a nonzero exit status */
			perror_func_msg_and_die("PTRACE_TRACEME");
		}
		kill(pid, SIGSTOP);
		for (unsigned int i = 0; i < ARRAY_SIZE(args); ++i) {
			syscall(args[i][0],
				args[i][1], args[i][2], args[i][3],
				args[i][4], args[i][5], args[i][6]);
		}
		/* unreachable */
		_exit(1);
	}

	const struct {
		unsigned int is_error;
		int rval;
	} *exp_param, exit_param[] = {
		{ 1, -ENOENT },	/* chdir */
		{ 0, pid }	/* gettid */
	};

	unsigned int ptrace_stop;

	for (ptrace_stop = 0; ; ++ptrace_stop) {
		struct_ptrace_syscall_info info = {
			.op = 0xff	/* invalid PTRACE_SYSCALL_INFO_* op */
		};
		const size_t size = sizeof(info);
		int status;
		long rc = waitpid(pid, &status, 0);
		if (rc != pid) {
			/* cannot happen */
			kill_tracee(pid);
			perror_func_msg_and_die("#%d: unexpected wait result"
						" %ld", ptrace_stop, rc);
		}
		if (WIFEXITED(status)) {
			/* tracee is no more */
			pid = 0;
			if (WEXITSTATUS(status) == 0)
				break;
			debug_func_msg("#%d: unexpected exit status %u",
				       ptrace_stop, WEXITSTATUS(status));
			FAIL;
		}
		if (WIFSIGNALED(status)) {
			/* tracee is no more */
			pid = 0;
			debug_func_msg("#%d: unexpected signal %u",
				       ptrace_stop, WTERMSIG(status));
			FAIL;
		}
		if (!WIFSTOPPED(status)) {
			/* cannot happen */
			kill_tracee(pid);
			error_func_msg_and_die("#%d: unexpected wait status"
					       " %#x", ptrace_stop, status);
		}

		switch (WSTOPSIG(status)) {
		case SIGSTOP:
			if (ptrace_stop) {
				debug_func_msg("#%d: unexpected signal stop",
					       ptrace_stop);
				FAIL;
			}
			if (ptrace(PTRACE_SETOPTIONS, pid, 0L,
				   PTRACE_O_TRACESYSGOOD) < 0) {
				/* cannot happen */
				kill_tracee(pid);
				perror_func_msg_and_die("PTRACE_SETOPTIONS");
			}
			rc = ptrace(PTRACE_GET_SYSCALL_INFO, pid,
				    (void *) size, &info);
			if (rc < 0) {
				debug_perror_msg("PTRACE_GET_SYSCALL_INFO");
				FAIL;
			}
			if (rc < (long) expected_none_size
			    || info.op != PTRACE_SYSCALL_INFO_NONE
			    || !info.arch
			    || !info.instruction_pointer
			    || !info.stack_pointer) {
				debug_func_msg("signal stop mismatch");
				FAIL;
			}
			break;

		case SIGTRAP | 0x80:
			rc = ptrace(PTRACE_GET_SYSCALL_INFO, pid,
				    (void *) size, &info);
			if (rc < 0) {
				debug_perror_msg("#%d: PTRACE_GET_SYSCALL_INFO",
						 ptrace_stop);
				FAIL;
			}
			switch (ptrace_stop) {
			case 1: /* entering chdir */
			case 3: /* entering gettid */
			case 5: /* entering exit_group */
				exp_args = args[ptrace_stop / 2];
				if (rc < (long) expected_entry_size
				    || info.op != PTRACE_SYSCALL_INFO_ENTRY
				    || !info.arch
				    || !info.instruction_pointer
				    || !info.stack_pointer
				    || (info.entry.nr != exp_args[0])
				    || ((kernel_ulong_t) info.entry.args[0] != exp_args[1])
				    || ((kernel_ulong_t) info.entry.args[1] != exp_args[2])
				    || ((kernel_ulong_t) info.entry.args[2] != exp_args[3])
				    || ((kernel_ulong_t) info.entry.args[3] != exp_args[4])
				    || ((kernel_ulong_t) info.entry.args[4] != exp_args[5])
				    || ((kernel_ulong_t) info.entry.args[5] != exp_args[6])) {
					debug_func_msg("#%d: entry stop"
						       " mismatch",
						       ptrace_stop);
					FAIL;
				}
				break;
			case 2: /* exiting chdir */
			case 4: /* exiting gettid */
				exp_param = &exit_param[ptrace_stop / 2 - 1];
				if (rc < (long) expected_exit_size
				    || info.op != PTRACE_SYSCALL_INFO_EXIT
				    || !info.arch
				    || !info.instruction_pointer
				    || !info.stack_pointer
				    || info.exit.is_error != exp_param->is_error
				    || info.exit.rval != exp_param->rval) {
					debug_func_msg("#%d: exit stop"
						       " mismatch",
						       ptrace_stop);
					FAIL;
				}
				break;
			default:
				debug_func_msg("#%d: unexpected syscall stop",
					       ptrace_stop);
				FAIL;
			}
			break;

		default:
			debug_func_msg("#%d: unexpected stop signal %#x",
				       ptrace_stop, WSTOPSIG(status));
			FAIL;
		}

		if (ptrace(PTRACE_SYSCALL, pid, 0L, 0L) < 0) {
			/* cannot happen */
			kill_tracee(pid);
			perror_func_msg_and_die("PTRACE_SYSCALL");
		}
	}

done:
	if (pid) {
		kill_tracee(pid);
		waitpid(pid, NULL, 0);
		ptrace_stop = -1U;
	}

	ptrace_get_syscall_info_supported =
		ptrace_stop == ARRAY_SIZE(args) * 2;

	if (ptrace_get_syscall_info_supported)
		debug_msg("PTRACE_GET_SYSCALL_INFO works");
	else
		debug_msg("PTRACE_GET_SYSCALL_INFO does not work");
#endif /* HAVE_FORK */

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
