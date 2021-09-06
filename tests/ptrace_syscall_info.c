/*
 * Check decoding of ptrace PTRACE_GET_SYSCALL_INFO request.
 *
 * Copyright (c) 2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2018-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include "ptrace.h"
#include "scno.h"

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <linux/audit.h>

#include "xlat.h"
#define XLAT_MACROS_ONLY
/* For xlat/audit_arch.h */
# include "xlat/elf_em.h"
#undef XLAT_MACROS_ONLY
#include "xlat/audit_arch.h"

static const char *errstr;

static long
do_ptrace(unsigned long request, unsigned long pid,
	  unsigned long addr, unsigned long data)
{
	long rc = syscall(__NR_ptrace, request, pid, addr, data);
	errstr = sprintrc(rc);
	return rc;
}

static pid_t pid;

static void
kill_tracee(void)
{
	if (!pid)
		return;
	int saved_errno = errno;
	kill(pid, SIGKILL);
	errno = saved_errno;
}

#define FAIL(fmt_, ...)							\
	do {								\
		kill_tracee();						\
		error_msg_and_fail("%s:%d: " fmt_,			\
				   __FILE__, __LINE__, ##__VA_ARGS__);	\
	} while (0)

#define PFAIL(fmt_, ...)						\
	do {								\
		kill_tracee();						\
		perror_msg_and_fail("%s:%d: " fmt_,			\
				    __FILE__, __LINE__, ##__VA_ARGS__);	\
	} while (0)

static const unsigned long args[][7] = {
	/* a sequence of architecture-agnostic syscalls */
	{
		__NR_chdir,
		(unsigned long) "",
		0xbad1fed1,
		0xbad2fed2,
		0xbad3fed3,
		0xbad4fed4,
		0xbad5fed5
	},
	{
		__NR_gettid,
		0xcaf0bea0,
		0xcaf1bea1,
		0xcaf2bea2,
		0xcaf3bea3,
		0xcaf4bea4,
		0xcaf5bea5
	},
	{
		__NR_exit_group,
		0,
		0xfac1c0d1,
		0xfac2c0d2,
		0xfac3c0d3,
		0xfac4c0d4,
		0xfac5c0d5
	}
};

static const char *sc_names[] = {
	"chdir",
	"gettid",
	"exit_group"
};

static const unsigned int expected_none_size =
	offsetof(struct_ptrace_syscall_info, entry);
static const unsigned int expected_entry_size =
	offsetofend(struct_ptrace_syscall_info, entry.args);
static const unsigned int expected_exit_size =
	offsetofend(struct_ptrace_syscall_info, exit.is_error);

static unsigned long end_of_page;
static unsigned int ptrace_stop;

static bool
test_none(void)
{
	do_ptrace(PTRACE_GET_SYSCALL_INFO, pid, 1, 0);
	printf("ptrace(PTRACE_GET_SYSCALL_INFO, %d, 1, NULL) = %s\n",
	       pid, errstr);

	do_ptrace(PTRACE_GET_SYSCALL_INFO, pid, 1, end_of_page);
	printf("ptrace(PTRACE_GET_SYSCALL_INFO, %d, 1, %#lx) = %s\n",
	       pid, end_of_page, errstr);

	for (unsigned int size = 0;
	     size <= sizeof(struct_ptrace_syscall_info); ++size) {
		unsigned long buf = end_of_page - size;
		memset((void *) buf, -1, size);

		long rc = do_ptrace(PTRACE_GET_SYSCALL_INFO, pid, size, buf);
		if (rc < 0) {
			printf("ptrace(PTRACE_GET_SYSCALL_INFO, %d, %u, %#lx)"
			       " = %s\n",
			       pid, (unsigned int) size, buf, errstr);
			return false;
		}
		if (rc < (long) expected_none_size)
			FAIL("signal stop mismatch");

		printf("ptrace(PTRACE_GET_SYSCALL_INFO, %d, %u, ",
		       pid, size);
		if (!size) {
			printf("%#lx) = %s\n", buf, errstr);
			continue;
		}

		/* copy to a local structure to avoid unaligned access */
		struct_ptrace_syscall_info info;
		memcpy(&info, (void *) buf,  MIN(size, expected_none_size));

		if (info.op != PTRACE_SYSCALL_INFO_NONE)
			FAIL("signal stop mismatch");
		printf("{op=PTRACE_SYSCALL_INFO_NONE");

		if (size < offsetofend(struct_ptrace_syscall_info, arch))
			goto printed_none;
		if (!info.arch)
			FAIL("signal stop mismatch");
		printf(", arch=");
		printxval(audit_arch, info.arch, "AUDIT_ARCH_???");

		if (size < offsetofend(struct_ptrace_syscall_info,
				       instruction_pointer))
			goto printed_none;
		if (!info.instruction_pointer)
			FAIL("signal stop mismatch");
		printf(", instruction_pointer=%#llx",
		       (unsigned long long) info.instruction_pointer);

		if (size < offsetofend(struct_ptrace_syscall_info,
				       stack_pointer))
			goto printed_none;
		if (!info.stack_pointer)
			FAIL("signal stop mismatch");
		printf(", stack_pointer=%#llx",
		       (unsigned long long) info.stack_pointer);

printed_none:
		printf("}) = %s\n", errstr);
	}

	return true;
}

static void
test_entry(void)
{
	for (unsigned int size = 0;
	     size <= sizeof(struct_ptrace_syscall_info); ++size) {
		unsigned long buf = end_of_page - size;
		memset((void *) buf, -1, size);

		long rc = do_ptrace(PTRACE_GET_SYSCALL_INFO, pid, size, buf);
		if (rc < 0)
			PFAIL("PTRACE_GET_SYSCALL_INFO");

		if (rc < (long) expected_entry_size)
			FAIL("#%d: entry stop mismatch", ptrace_stop);

		printf("ptrace(PTRACE_GET_SYSCALL_INFO, %d, %u, ",
		       pid, size);
		if (!size) {
			printf("%#lx) = %s\n", buf, errstr);
			continue;
		}

		/* copy to a local structure to avoid unaligned access */
		struct_ptrace_syscall_info info;
		memcpy(&info, (void *) buf,  MIN(size, expected_entry_size));

		if (info.op != PTRACE_SYSCALL_INFO_ENTRY)
			FAIL("#%d: entry stop mismatch", ptrace_stop);
		printf("{op=PTRACE_SYSCALL_INFO_ENTRY");

		if (size < offsetofend(struct_ptrace_syscall_info, arch))
			goto printed_entry_common;
		if (!info.arch)
			FAIL("#%d: entry stop mismatch", ptrace_stop);
		printf(", arch=");
		printxval(audit_arch, info.arch, "AUDIT_ARCH_???");

		if (size < offsetofend(struct_ptrace_syscall_info,
				       instruction_pointer))
			goto printed_entry_common;
		if (!info.instruction_pointer)
			FAIL("#%d: entry stop mismatch", ptrace_stop);
		printf(", instruction_pointer=%#llx",
		       (unsigned long long) info.instruction_pointer);

		if (size < offsetofend(struct_ptrace_syscall_info,
				       stack_pointer))
			goto printed_entry_common;
		if (!info.stack_pointer)
			FAIL("#%d: entry stop mismatch", ptrace_stop);
		printf(", stack_pointer=%#llx",
		       (unsigned long long) info.stack_pointer);

		if (size < offsetofend(struct_ptrace_syscall_info, entry.nr))
			goto printed_entry_common;
		const unsigned long *exp_args = args[ptrace_stop / 2];
		if (info.entry.nr != exp_args[0])
			FAIL("#%d: entry stop mismatch", ptrace_stop);
		printf(", entry={nr="
		       NABBR("%llu") VERB(" /* ") NRAW("__NR_%s") VERB(" */"),
		       XLAT_SEL((unsigned long long) info.entry.nr,
				sc_names[ptrace_stop / 2]));

		for (unsigned int i = 0; i < ARRAY_SIZE(info.entry.args); ++i) {
			const unsigned int i_size =
				offsetofend(struct_ptrace_syscall_info,
					    entry.args[i]);
			if (size < i_size) {
				if (i)
					break;
				goto printed_entry_nr;
			}
#if SIZEOF_KERNEL_LONG_T > SIZEOF_LONG
# define CAST (unsigned long)
#else
# define CAST
#endif
			if (CAST info.entry.args[i] != exp_args[i + 1])
				FAIL("#%d: entry stop mismatch", ptrace_stop);
			printf("%s%#llx", (i ? ", " : ", args=["),
			       (unsigned long long) info.entry.args[i]);
		}
		printf("]");

printed_entry_nr:
		printf("}");

printed_entry_common:
		printf("}) = %s\n", errstr);
	}
}

static void
test_exit(void)
{
	for (unsigned int size = 0;
	     size <= sizeof(struct_ptrace_syscall_info); ++size) {
		unsigned long buf = end_of_page - size;
		memset((void *) buf, -1, size);

		long rc = do_ptrace(PTRACE_GET_SYSCALL_INFO, pid, size, buf);
		if (rc < 0)
			PFAIL("PTRACE_GET_SYSCALL_INFO");

		if (rc < (long) expected_exit_size)
			FAIL("#%d: exit stop mismatch", ptrace_stop);

		printf("ptrace(PTRACE_GET_SYSCALL_INFO, %d, %u, ",
		       pid, size);
		if (!size) {
			printf("%#lx) = %s\n", buf, errstr);
			continue;
		}

		/* copy to a local structure to avoid unaligned access */
		struct_ptrace_syscall_info info;
		memcpy(&info, (void *) buf,  MIN(size, expected_exit_size));

		if (info.op != PTRACE_SYSCALL_INFO_EXIT)
			FAIL("#%d: exit stop mismatch", ptrace_stop);
		printf("{op=PTRACE_SYSCALL_INFO_EXIT");

		if (size < offsetofend(struct_ptrace_syscall_info, arch))
			goto printed_exit_common;
		if (!info.arch)
			FAIL("#%d: exit stop mismatch", ptrace_stop);
		printf(", arch=");
		printxval(audit_arch, info.arch, "AUDIT_ARCH_???");

		if (size < offsetofend(struct_ptrace_syscall_info,
				       instruction_pointer))
			goto printed_exit_common;
		if (!info.instruction_pointer)
			FAIL("#%d: exit stop mismatch", ptrace_stop);
		printf(", instruction_pointer=%#llx",
		       (unsigned long long) info.instruction_pointer);

		if (size < offsetofend(struct_ptrace_syscall_info,
				       stack_pointer))
			goto printed_exit_common;
		if (!info.stack_pointer)
			FAIL("#%d: exit stop mismatch", ptrace_stop);
		printf(", stack_pointer=%#llx",
		       (unsigned long long) info.stack_pointer);

		const struct {
			unsigned int is_error;
			int rval;
			const char *str;
		} exit_param[] = {
			{ 1, -ENOENT, "-ENOENT" },	/* chdir */
			{ 0, pid, NULL }		/* gettid */
		}, *exp_param = &exit_param[ptrace_stop / 2 - 1];

		if (size < offsetofend(struct_ptrace_syscall_info, exit.rval))
			goto printed_exit_common;
		if (info.exit.rval != exp_param->rval)
			FAIL("#%d: exit stop mismatch", ptrace_stop);
		if (size >= expected_exit_size && info.exit.is_error) {
			printf(", exit={rval=%s", exp_param->str);
		} else {
			printf(", exit={rval=%lld", (long long) info.exit.rval);
		}

		if (size >= expected_exit_size) {
			if (info.exit.is_error != exp_param->is_error)
				FAIL("#%d: exit stop mismatch", ptrace_stop);
			printf(", is_error=%u",
			       (unsigned int) info.exit.is_error);
		}

		printf("}");

printed_exit_common:
		printf("}) = %s\n", errstr);
	}
}

int
main(void)
{
	end_of_page = (unsigned long) tail_alloc(1) + 1;

	pid = getpid();
	do_ptrace(PTRACE_GET_SYSCALL_INFO, pid, 0, 0);
	printf("ptrace(PTRACE_GET_SYSCALL_INFO, %d, 0, NULL) = %s\n",
	       pid, errstr);

	pid = fork();
	if (pid < 0)
		PFAIL("fork");

	if (pid == 0) {
		/* get the pid before PTRACE_TRACEME */
		pid = getpid();
		if (do_ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) {
			/* exit with a nonzero exit status */
			PFAIL("PTRACE_TRACEME");
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

	for (ptrace_stop = 0; ; ++ptrace_stop) {
		int status;
		long rc = waitpid(pid, &status, 0);
		if (rc != pid) {
			/* cannot happen */
			PFAIL("#%d: unexpected wait result %ld",
					    ptrace_stop, rc);
		}
		if (WIFEXITED(status)) {
			/* tracee is no more */
			pid = 0;
			if (WEXITSTATUS(status) == 0)
				break;
			FAIL("#%d: unexpected exit status %u",
			     ptrace_stop, WEXITSTATUS(status));
		}
		if (WIFSIGNALED(status)) {
			/* tracee is no more */
			pid = 0;
			FAIL("#%d: unexpected signal %u",
			     ptrace_stop, WTERMSIG(status));
		}
		if (!WIFSTOPPED(status)) {
			/* cannot happen */
			FAIL("#%d: unexpected wait status %#x",
			     ptrace_stop, status);
		}

		switch (WSTOPSIG(status)) {
		case SIGSTOP:
			if (ptrace_stop)
				FAIL("#%d: unexpected signal stop",
					       ptrace_stop);
			if (do_ptrace(PTRACE_SETOPTIONS, pid, 0,
				   PTRACE_O_TRACESYSGOOD) < 0) {
				/* cannot happen */
				PFAIL("PTRACE_SETOPTIONS");
			}
			printf("ptrace(PTRACE_SETOPTIONS, %d, NULL"
			       ", PTRACE_O_TRACESYSGOOD) = 0\n", pid);

			if (!test_none())
				goto done;
			break;

		case SIGTRAP | 0x80:
			switch (ptrace_stop) {
			case 1: /* entering chdir */
			case 3: /* entering gettid */
			case 5: /* entering exit_group */
				test_entry();
				break;
			case 2: /* exiting chdir */
			case 4: /* exiting gettid */
				test_exit();
				break;
			default:
				FAIL("#%d: unexpected syscall stop",
				     ptrace_stop);
			}
			break;

		default:
			FAIL("#%d: unexpected stop signal %#x",
			     ptrace_stop, WSTOPSIG(status));
		}

		if (do_ptrace(PTRACE_SYSCALL, pid, 0, 0) < 0) {
			/* cannot happen */
			PFAIL("PTRACE_SYSCALL");
		}
		printf("ptrace(PTRACE_SYSCALL, %d, NULL, 0) = 0\n", pid);
	}

done:
	if (pid) {
		kill_tracee();
		waitpid(pid, NULL, 0);
	}

	puts("+++ exited with 0 +++");
	return 0;
}
