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
bool ptrace_set_syscall_info_supported;

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

static bool
do_test_ptrace_set_syscall_info(void)
{
	const pid_t tracer_pid = getpid();
	unsigned int ptrace_stop = -1U;
	pid_t tracee_pid = 0;

	int splin[2] = { -1, -1 };
	int splout[2] = { -1, -1 };

	if (pipe(splin) ||
	    pipe(splout) ||
	    write(splin[1], splin, sizeof(splin)) != sizeof(splin))
		FAIL;

	const struct {
		struct si_entry entry[2];
		struct si_exit exit[2];
	} si[] = {
		{ /* change scno, keep non-error rval */
			{
				{
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
					__NR_getppid,
					{
						dummy_syscall_args[0],
						dummy_syscall_args[1],
						dummy_syscall_args[2],
						dummy_syscall_args[3],
						dummy_syscall_args[4],
						dummy_syscall_args[5]
					}
				}
			}, {
				{ 0, tracer_pid },
				{ 0, tracer_pid }
			}
		}, { /* set scno to -1, keep error rval */
			{
				{
					__NR_chdir,
					{
						(uintptr_t) ".",
						dummy_syscall_args[1],
						dummy_syscall_args[2],
						dummy_syscall_args[3],
						dummy_syscall_args[4],
						dummy_syscall_args[5]
					}
				}, {
					-1,
					{
						(uintptr_t) ".",
						dummy_syscall_args[1],
						dummy_syscall_args[2],
						dummy_syscall_args[3],
						dummy_syscall_args[4],
						dummy_syscall_args[5]
					}
				}
			}, {
				{ 1, -ENOSYS },
				{ 1, -ENOSYS }
			}
		}, { /* keep scno, change non-error rval */
			{
				{
					__NR_getppid,
					{
						dummy_syscall_args[0],
						dummy_syscall_args[1],
						dummy_syscall_args[2],
						dummy_syscall_args[3],
						dummy_syscall_args[4],
						dummy_syscall_args[5]
					}
				}, {
					__NR_getppid,
					{
						dummy_syscall_args[0],
						dummy_syscall_args[1],
						dummy_syscall_args[2],
						dummy_syscall_args[3],
						dummy_syscall_args[4],
						dummy_syscall_args[5]
					}
				}
			}, {
				{ 0, tracer_pid },
				{ 0, tracer_pid + 1 }
			}
		}, { /* change arg1, keep non-error rval */
			{
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
					__NR_chdir,
					{
						(uintptr_t) ".",
						dummy_syscall_args[1],
						dummy_syscall_args[2],
						dummy_syscall_args[3],
						dummy_syscall_args[4],
						dummy_syscall_args[5]
					}
				}
			}, {
				{ 0, 0 },
				{ 0, 0 }
			}
		}, { /* set scno to -1, change error rval to non-error */
			{
				{
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
					-1,
					{
						dummy_syscall_args[0],
						dummy_syscall_args[1],
						dummy_syscall_args[2],
						dummy_syscall_args[3],
						dummy_syscall_args[4],
						dummy_syscall_args[5]
					}
				}
			}, {
				{ 1, -ENOSYS },
				{ 0, tracer_pid }
			}
		}, { /* change scno, change non-error rval to error */
			{
				{
					__NR_chdir,
					{
						dummy_syscall_args[0],
						dummy_syscall_args[1],
						dummy_syscall_args[2],
						dummy_syscall_args[3],
						dummy_syscall_args[4],
						dummy_syscall_args[5]
					}
				}, {
					__NR_getppid,
					{
						dummy_syscall_args[0],
						dummy_syscall_args[1],
						dummy_syscall_args[2],
						dummy_syscall_args[3],
						dummy_syscall_args[4],
						dummy_syscall_args[5]
					}
				}
			}, {
				{ 0, tracer_pid },
				{ 1, -EISDIR }
			}
		}, { /* change scno and all args, change non-error rval */
			{
				{
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
					__NR_splice,
					{
						splin[0], 0,
						splout[1], 0,
						sizeof(splin), 2
					}
				}
			}, {
				{ 0, sizeof(splin) },
				{ 0, sizeof(splin) + 1 }
			}
		}, { /* change arg1, no exit stop */
			{
				{
					__NR_exit_group,
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
			}, {
				{ 0, 0 },
				{ 0, 0 }
			}
		},
	};

	long rc;
	unsigned int i;
	tracee_pid = fork();
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
		for (i = 0; i < ARRAY_SIZE(si); ++i) {
			rc = syscall(si[i].entry[0].nr,
				     si[i].entry[0].args[0],
				     si[i].entry[0].args[1],
				     si[i].entry[0].args[2],
				     si[i].entry[0].args[3],
				     si[i].entry[0].args[4],
				     si[i].entry[0].args[5]);
			/*
			 * Check that the syscall return value matches
			 * the expectations.
			 */
			if (si[i].exit[1].is_error) {
				if (rc != -1 || errno != -si[i].exit[1].rval)
					break;
			} else {
				if (rc != si[i].exit[1].rval)
					break;
			}
		}
		/*
		 * Something went wrong, but in this state tracee
		 * cannot reliably issue syscalls, so just crash.
		 */
		*(volatile unsigned char *) (uintptr_t) i = 42;
		/* unreachable */
		_exit(i + 1);
	}

	for (ptrace_stop = 0; ; ++ptrace_stop) {
		struct_ptrace_syscall_info info = {
			.op = 0xff	/* invalid PTRACE_SYSCALL_INFO_* op */
		};
		const size_t size = sizeof(info);
		int status;

		rc = waitpid(tracee_pid, &status, 0);
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
		if (ptrace_stop >= ARRAY_SIZE(si) * 2) {
			debug_func_msg("ptrace stop overflow");
			FAIL;
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
			if (ptrace_stop == 0) {
				debug_func_msg("unexpected syscall stop");
				FAIL;
			}
			rc = ptrace(PTRACE_GET_SYSCALL_INFO, tracee_pid,
				    (void *) size, &info);
			if (rc < 0) {
				debug_func_perror_msg("ptrace stop #%d"
					": PTRACE_GET_SYSCALL_INFO #1",
					ptrace_stop);
				FAIL;
			}
			if (ptrace_stop & 1) {
				/* entering syscall */
				const struct si_entry *exp_entry =
					&si[ptrace_stop / 2].entry[0];
				const struct si_entry *set_entry =
					&si[ptrace_stop / 2].entry[1];

				/* check syscall info before the changes */
				if (!check_psi_entry(&info, rc, exp_entry,
						"PTRACE_GET_SYSCALL_INFO #1",
						ptrace_stop)) {
					FAIL;
				}

				/* apply the changes */
				info.entry.nr = set_entry->nr;
				for (i = 0; i < ARRAY_SIZE(set_entry->args); ++i)
					info.entry.args[i] = set_entry->args[i];
				if (ptrace(PTRACE_SET_SYSCALL_INFO, tracee_pid,
					   (void *) size, &info)) {
					debug_func_perror_msg("ptrace stop #%d"
						": PTRACE_SET_SYSCALL_INFO",
						ptrace_stop);
					FAIL;
				}

				/* check syscall info after the changes */
				memset(&info, 0, sizeof(info));
				info.op = 0xff;
				rc = ptrace(PTRACE_GET_SYSCALL_INFO, tracee_pid,
					    (void *) size, &info);
				if (rc < 0) {
					debug_func_perror_msg("ptrace stop #%d"
						": PTRACE_GET_SYSCALL_INFO #2",
						ptrace_stop);
					FAIL;
				}
				if (!check_psi_entry(&info, rc, set_entry,
						"PTRACE_GET_SYSCALL_INFO #2",
						ptrace_stop)) {
					FAIL;
				}
			} else {
				/* exiting syscall */
				const struct si_exit *exp_exit =
					&si[ptrace_stop / 2 - 1].exit[0];
				const struct si_exit *set_exit =
					&si[ptrace_stop / 2 - 1].exit[1];

				/* check syscall info before the changes */
				if (!check_psi_exit(&info, rc, exp_exit,
						"PTRACE_GET_SYSCALL_INFO #1",
						ptrace_stop)) {
					FAIL;
				}

				/* apply the changes */
				info.exit.is_error = set_exit->is_error;
				info.exit.rval = set_exit->rval;
				if (ptrace(PTRACE_SET_SYSCALL_INFO, tracee_pid,
					   (void *) size, &info)) {
					debug_func_perror_msg("ptrace stop #%d"
						": PTRACE_SET_SYSCALL_INFO",
						ptrace_stop);
					FAIL;
				}

				/* check syscall info after the changes */
				memset(&info, 0, sizeof(info));
				info.op = 0xff;
				rc = ptrace(PTRACE_GET_SYSCALL_INFO, tracee_pid,
					    (void *) size, &info);
				if (rc < 0) {
					debug_func_perror_msg("ptrace stop #%d"
						": PTRACE_GET_SYSCALL_INFO #2",
						ptrace_stop);
					FAIL;
				}
				if (!check_psi_exit(&info, rc, set_exit,
						"PTRACE_GET_SYSCALL_INFO #2",
						ptrace_stop)) {
					FAIL;
				}
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
	if (splout[1] >= 0)
		close(splout[1]);
	if (splout[0] >= 0)
		close(splout[0]);
	if (splin[1] >= 0)
		close(splin[1]);
	if (splin[0] >= 0)
		close(splin[0]);

	return ptrace_stop == ARRAY_SIZE(si) * 2;
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
	 * If PTRACE_SET_SYSCALL_INFO is supported,
	 * then PTRACE_GET_SYSCALL_INFO is also supported.
	 */
	if (ptrace_set_syscall_info_supported)
		ptrace_get_syscall_info_supported = true;

	/*
	 * NOMMU provides no forks necessary for PTRACE_GET_SYSCALL_INFO test,
	 * leave the default unchanged.
	 */
#ifdef HAVE_FORK
	if (!ptrace_get_syscall_info_supported)
		ptrace_get_syscall_info_supported =
			do_test_ptrace_get_syscall_info();
#endif /* HAVE_FORK */

	if (ptrace_get_syscall_info_supported)
		debug_msg("PTRACE_GET_SYSCALL_INFO works");
	else
		debug_msg("PTRACE_GET_SYSCALL_INFO does not work");

	return ptrace_get_syscall_info_supported;
}

/*
 * Test that PTRACE_SET_SYSCALL_INFO API is supported by the kernel, and
 * that the semantics implemented in the kernel matches our expectations.
 */
bool
test_ptrace_set_syscall_info(void)
{
	/*
	 * NOMMU provides no forks necessary for PTRACE_SET_SYSCALL_INFO test,
	 * leave the default unchanged.
	 */
#ifdef HAVE_FORK
	ptrace_set_syscall_info_supported =
		do_test_ptrace_set_syscall_info();
#endif /* HAVE_FORK */

	if (ptrace_set_syscall_info_supported)
		debug_msg("PTRACE_SET_SYSCALL_INFO works");
	else
		debug_msg("PTRACE_SET_SYSCALL_INFO does not work");

	return ptrace_set_syscall_info_supported;
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
