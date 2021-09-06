/*
 * Copyright (c) 2018 Chen Jingpiao <chenjingpiao@gmail.com>
 * Copyright (c) 2019 Paul Chaignon <paul.chaignon@gmail.com>
 * Copyright (c) 2018-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include "ptrace.h"
#include <signal.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <linux/filter.h>

#include "filter_seccomp.h"
#include "number_set.h"
#include "scno.h"

bool seccomp_filtering;
bool seccomp_before_sysentry;

#include <linux/seccomp.h>

#ifndef BPF_MAXINSNS
# define BPF_MAXINSNS 4096
#endif

#define JMP_PLACEHOLDER_NEXT  ((unsigned char) -1)
#define JMP_PLACEHOLDER_TRACE ((unsigned char) -2)
#define JMP_PLACEHOLDER_ALLOW ((unsigned char) -3)

#define SET_BPF(filter, code, jt, jf, k) \
	(*(filter) = (struct sock_filter) { code, jt, jf, k })

#define SET_BPF_STMT(filter, code, k) \
	SET_BPF(filter, code, 0, 0, k)

#define SET_BPF_JUMP(filter, code, k, jt, jf) \
	SET_BPF(filter, BPF_JMP | code, jt, jf, k)

typedef unsigned short (*filter_generator_t)(struct sock_filter *,
					     bool *overflow);
static unsigned short linear_filter_generator(struct sock_filter *,
					      bool *overflow);
static unsigned short binary_match_filter_generator(struct sock_filter *,
						    bool *overflow);
static filter_generator_t filter_generators[] = {
	linear_filter_generator,
	binary_match_filter_generator,
};

/*
 * Keep some margin in seccomp_filter as programs larger than allowed may
 * be constructed before we discard them.
 */
static struct sock_filter
filters[ARRAY_SIZE(filter_generators)][2 * BPF_MAXINSNS];
static struct sock_fprog bpf_prog = {
	.len = USHRT_MAX,
	.filter = NULL,
};

#ifdef HAVE_FORK

static void ATTRIBUTE_NORETURN
check_seccomp_order_do_child(void)
{
	static const struct sock_filter filter[] = {
		/* return (nr == __NR_gettid) ? RET_TRACE : RET_ALLOW; */
		BPF_STMT(BPF_LD | BPF_W | BPF_ABS,
			 offsetof(struct seccomp_data, nr)),
		BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_gettid, 0, 1),
		BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_TRACE),
		BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW)
	};
	static const struct sock_fprog prog = {
		.len = ARRAY_SIZE(filter),
		.filter = (struct sock_filter *) filter
	};

	/* Get everything ready before PTRACE_TRACEME.  */
	if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) < 0)
		perror_func_msg_and_die("prctl(PR_SET_NO_NEW_PRIVS, 1");
	if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &prog) < 0)
		perror_func_msg_and_die("prctl(PR_SET_SECCOMP)");
	int pid = getpid();

	if (ptrace(PTRACE_TRACEME, 0L, 0L, 0L) < 0) {
		/* Exit with a nonzero exit status.  */
		perror_func_msg_and_die("PTRACE_TRACEME");
	}

	GCOV_DUMP;

	kill(pid, SIGSTOP);
	syscall(__NR_gettid);
	_exit(0);
}

static int
check_seccomp_order_tracer(int pid)
{
	unsigned int step;

	for (step = 0; ; ++step) {
		int status;

		for (;;) {
			long rc = waitpid(pid, &status, 0);
			if (rc < 0 && errno == EINTR)
				continue;
			if (rc == pid)
				break;
			/* Cannot happen.  */
			perror_func_msg("#%d: unexpected wait result %ld",
					step, rc);
			return pid;
		}

		if (WIFEXITED(status)) {
			/* The tracee is no more.  */
			pid = 0;

			int exitstatus = WEXITSTATUS(status);
			if (step == 5 && exitstatus == 0) {
				seccomp_filtering = true;
			} else {
				error_func_msg("#%d: unexpected exit status %u",
					       step, exitstatus);
			}
			break;
		}

		if (WIFSIGNALED(status)) {
			/* The tracee is no more.  */
			pid = 0;

			error_func_msg("#%d: unexpected signal %u",
				       step, WTERMSIG(status));
			break;
		}

		if (!WIFSTOPPED(status)) {
			/* Cannot happen.  */
			error_func_msg("#%d: unexpected wait status %#x",
				       step, status);
			break;
		}

		unsigned int event = (unsigned int) status >> 16;

		switch (WSTOPSIG(status)) {
		case SIGSTOP:
			if (step != 0) {
				error_func_msg("#%d: unexpected signal stop",
					       step);
				return pid;
			}
			if (ptrace(PTRACE_SETOPTIONS, pid, 0L,
				   PTRACE_O_TRACESYSGOOD|
				   PTRACE_O_TRACESECCOMP) < 0) {
				perror_func_msg("PTRACE_SETOPTIONS");
				return pid;
			}
			break;

		case SIGTRAP:
			if (event != PTRACE_EVENT_SECCOMP) {
				error_func_msg("#%d: unexpected trap %#x",
					       step, event);
				return pid;
			}

			switch (step) {
			case 1: /* Seccomp stop before entering gettid.  */
				seccomp_before_sysentry = true;
				break;
			case 2: /* Seccomp stop after entering gettid.  */
				if (!seccomp_before_sysentry)
					break;
				ATTRIBUTE_FALLTHROUGH;
			default:
				error_func_msg("#%d: unexpected seccomp stop",
					       step);
				return pid;
			}
			break;

		case SIGTRAP | 0x80:
			switch (step) {
			case 3: /* Exiting gettid.  */
			case 4: /* Entering exit_group.  */
				break;
			case 1: /* Entering gettid before seccomp stop.  */
				seccomp_before_sysentry = false;
				break;
			case 2: /* Entering gettid after seccomp stop.  */
				if (seccomp_before_sysentry)
					break;
				ATTRIBUTE_FALLTHROUGH;
			default:
				error_func_msg("#%d: unexpected syscall stop",
					       step);
				return pid;
			}
			break;

		default:
			error_func_msg("#%d: unexpected stop signal %#x",
				       step, WSTOPSIG(status));
			return pid;
		}

		if (ptrace(PTRACE_SYSCALL, pid, 0L, 0L) < 0) {
			/* Cannot happen.  */
			perror_func_msg("#%d: PTRACE_SYSCALL", step);
			break;
		}
	}

	return pid;
}
#endif /* HAVE_FORK */

static void
check_seccomp_order(void)
{
	seccomp_filtering = false;

	/* NOMMU provides no forks necessary for the test.  */
#ifdef HAVE_FORK
	int pid = fork();
	if (pid < 0) {
		perror_func_msg("fork");
		return;
	}

	if (pid == 0)
		check_seccomp_order_do_child();

	pid = check_seccomp_order_tracer(pid);
	if (pid) {
		kill(pid, SIGKILL);
		for (;;) {
			long rc = waitpid(pid, NULL, 0);
			if (rc < 0 && errno == EINTR)
				continue;
			break;
		}
	}
#endif /* HAVE_FORK */
}

static bool
traced_by_seccomp(unsigned int scno, unsigned int p)
{
	unsigned int always_trace_flags =
		TRACE_INDIRECT_SUBCALL | TRACE_SECCOMP_DEFAULT |
		(stack_trace_enabled ? MEMORY_MAPPING_CHANGE : 0);
	return sysent_vec[p][scno].sys_flags & always_trace_flags ||
		is_number_in_set_array(scno, trace_set, p);
}

static void
replace_jmp_placeholders(unsigned char *jmp_offset, unsigned char jmp_next,
			 unsigned char jmp_trace, unsigned char jmp_allow)
{
	switch (*jmp_offset) {
	case JMP_PLACEHOLDER_NEXT:
		*jmp_offset = jmp_next;
		break;
	case JMP_PLACEHOLDER_TRACE:
		*jmp_offset = jmp_trace;
		break;
	case JMP_PLACEHOLDER_ALLOW:
		*jmp_offset = jmp_allow;
		break;
	default:
		break;
	}
}

static unsigned short
bpf_syscalls_cmp(struct sock_filter *filter,
		 unsigned int lower, unsigned int upper)
{
	if (lower + 1 == upper) {
		/* if (nr == lower) return RET_TRACE; */
		SET_BPF_JUMP(filter, BPF_JEQ | BPF_K, lower,
			     JMP_PLACEHOLDER_TRACE, 0);
		return 1;
	} else {
		/* if (nr >= lower && nr < upper) return RET_TRACE; */
		SET_BPF_JUMP(filter, BPF_JGE | BPF_K, lower, 0, 1);
		SET_BPF_JUMP(filter + 1, BPF_JGE | BPF_K, upper, 0,
			     JMP_PLACEHOLDER_TRACE);
		return 2;
	}
}

static unsigned short
linear_filter_generator(struct sock_filter *filter, bool *overflow)
{
	/*
	 * Generated program looks like:
	 * if (arch == AUDIT_ARCH_A && nr >= flag) {
	 *	if (nr == 59)
	 *		return SECCOMP_RET_TRACE;
	 *	if (nr >= 321 && nr <= 323)
	 *		return SECCOMP_RET_TRACE;
	 *	...
	 *	return SECCOMP_RET_ALLOW;
	 * }
	 * if (arch == AUDIT_ARCH_A) {
	 *	...
	 * }
	 * if (arch == AUDIT_ARCH_B) {
	 *	...
	 * }
	 * return SECCOMP_RET_TRACE;
	 */
	unsigned short pos = 0;

#if SUPPORTED_PERSONALITIES > 1
	SET_BPF_STMT(&filter[pos++], BPF_LD | BPF_W | BPF_ABS,
		     offsetof(struct seccomp_data, arch));
#endif

	/*
	 * Personalities are iterated in reverse-order in the BPF program so
	 * that the x86 case is naturally handled.  On x86, the first and third
	 * personalities have the same arch identifier.  The third can be
	 * distinguished based on its associated syscall flag, so we check it
	 * first.  The only drawback here is that the first personality is more
	 * common, which may make the BPF program slower to match syscalls on
	 * average.
	 */
	for (int p = SUPPORTED_PERSONALITIES - 1; p >= 0; --p) {
		unsigned int lower = UINT_MAX;
		unsigned short start = pos, end;

#if SUPPORTED_PERSONALITIES > 1
		/* if (arch != audit_arch_vec[p].arch) goto next; */
		SET_BPF_JUMP(&filter[pos++], BPF_JEQ | BPF_K,
			     audit_arch_vec[p].arch, 0, JMP_PLACEHOLDER_NEXT);
#endif
		SET_BPF_STMT(&filter[pos++], BPF_LD | BPF_W | BPF_ABS,
			     offsetof(struct seccomp_data, nr));

#if SUPPORTED_PERSONALITIES > 1
		if (audit_arch_vec[p].flag) {
			/* if (nr < audit_arch_vec[p].flag) goto next; */
			SET_BPF_JUMP(&filter[pos++], BPF_JGE | BPF_K,
				     audit_arch_vec[p].flag, 2, 0);
			SET_BPF_STMT(&filter[pos++], BPF_LD | BPF_W | BPF_ABS,
				     offsetof(struct seccomp_data, arch));
			SET_BPF_JUMP(&filter[pos++], BPF_JA,
				     JMP_PLACEHOLDER_NEXT, 0, 0);
		}
#endif

		for (unsigned int i = 0; i < nsyscall_vec[p]; ++i) {
			if (traced_by_seccomp(i, p)) {
				if (lower == UINT_MAX)
					lower = i;
				continue;
			}
			if (lower == UINT_MAX)
				continue;
			pos += bpf_syscalls_cmp(filter + pos,
						lower | audit_arch_vec[p].flag,
						i | audit_arch_vec[p].flag);
			lower = UINT_MAX;
		}
		if (lower != UINT_MAX)
			pos += bpf_syscalls_cmp(filter + pos,
						lower | audit_arch_vec[p].flag,
						nsyscall_vec[p]
						| audit_arch_vec[p].flag);
		end = pos;

		/* if (nr >= max_nr) return RET_TRACE; */
		SET_BPF_JUMP(&filter[pos++], BPF_JGE | BPF_K,
			     nsyscall_vec[p] | audit_arch_vec[p].flag, 1, 0);

		SET_BPF_STMT(&filter[pos++], BPF_RET | BPF_K,
			     SECCOMP_RET_ALLOW);
		SET_BPF_STMT(&filter[pos++], BPF_RET | BPF_K,
			     SECCOMP_RET_TRACE);

		/*
		 * Within generated BPF programs, the origin and destination of
		 * jumps are always in the same personality section.  The
		 * largest jump is therefore the jump from the first
		 * instruction of the section to the last, to skip the
		 * personality and try to compare .arch to the next
		 * personality.
		 * If we have a personality section with more than 255
		 * instructions, the jump offset will overflow.  Such program
		 * is unlikely to happen, so we simply disable seccomp-filter
		 * in such a case.
		 */
		if (pos - start > UCHAR_MAX) {
			*overflow = true;
			return pos;
		}

		for (unsigned int i = start; i < end; ++i) {
			if (BPF_CLASS(filter[i].code) != BPF_JMP)
				continue;
			unsigned char jmp_next = pos - i - 1;
			unsigned char jmp_trace = pos - i - 2;
			unsigned char jmp_allow = pos - i - 3;
			replace_jmp_placeholders(&filter[i].jt, jmp_next,
						 jmp_trace, jmp_allow);
			replace_jmp_placeholders(&filter[i].jf, jmp_next,
						 jmp_trace, jmp_allow);
			if (BPF_OP(filter[i].code) == BPF_JA)
				filter[i].k = (unsigned int) jmp_next;
		}
	}

#if SUPPORTED_PERSONALITIES > 1
	/* Jumps conditioned on .arch default to this RET_TRACE. */
	SET_BPF_STMT(&filter[pos++], BPF_RET | BPF_K, SECCOMP_RET_TRACE);
#endif

	return pos;
}

static unsigned short
bpf_syscalls_match(struct sock_filter *filter, unsigned int bitarray,
		   unsigned int bitarray_idx)
{
	if (!bitarray) {
		/* return RET_ALLOW; */
		SET_BPF_JUMP(filter, BPF_JMP | BPF_JEQ | BPF_K, bitarray_idx,
			     JMP_PLACEHOLDER_ALLOW, 0);
		return 1;
	}
	if (bitarray == UINT_MAX) {
		/* return RET_TRACE; */
		SET_BPF_JUMP(filter, BPF_JMP | BPF_JEQ | BPF_K, bitarray_idx,
			     JMP_PLACEHOLDER_TRACE, 0);
		return 1;
	}
	/*
	 * if (A == nr / 32)
	 *   return (X & bitarray) ? RET_TRACE : RET_ALLOW;
	 */
	SET_BPF_JUMP(filter, BPF_JMP | BPF_JEQ | BPF_K, bitarray_idx,
		     0, 2);
	SET_BPF_STMT(filter + 1, BPF_MISC | BPF_TXA, 0);
	SET_BPF_JUMP(filter + 2, BPF_JMP | BPF_JSET | BPF_K, bitarray,
		     JMP_PLACEHOLDER_TRACE, JMP_PLACEHOLDER_ALLOW);
	return 3;
}

static unsigned short
binary_match_filter_generator(struct sock_filter *filter, bool *overflow)
{
	unsigned short pos = 0;

#if SUPPORTED_PERSONALITIES > 1
	SET_BPF_STMT(&filter[pos++], BPF_LD | BPF_W | BPF_ABS,
		     offsetof(struct seccomp_data, arch));
#endif

	/* Personalities are iterated in reverse-order in the BPF program so that
	 * the x86 case is naturally handled.  In x86, the first and third
	 * personalities have the same arch identifier.  The third can be
	 * distinguished based on its associated bit mask, so we check it first.
	 * The only drawback here is that the first personality is more common,
	 * which may make the BPF program slower to match syscalls on average. */
	for (int p = SUPPORTED_PERSONALITIES - 1;
		 p >= 0 && pos <= BPF_MAXINSNS;
		 --p) {
		unsigned short start = pos, end;
		unsigned int bitarray = 0;
		unsigned int i;

#if SUPPORTED_PERSONALITIES > 1
		SET_BPF_JUMP(&filter[pos++], BPF_JMP | BPF_JEQ | BPF_K,
			     audit_arch_vec[p].arch, 0, JMP_PLACEHOLDER_NEXT);
#endif
		SET_BPF_STMT(&filter[pos++], BPF_LD | BPF_W | BPF_ABS,
			     offsetof(struct seccomp_data, nr));

#if SUPPORTED_PERSONALITIES > 1
		if (audit_arch_vec[p].flag) {
			SET_BPF_JUMP(&filter[pos++], BPF_JMP | BPF_JGE | BPF_K,
				     audit_arch_vec[p].flag, 2, 0);
			SET_BPF_STMT(&filter[pos++], BPF_LD | BPF_W | BPF_ABS,
				     offsetof(struct seccomp_data, arch));
			SET_BPF_JUMP(&filter[pos++], BPF_JMP | BPF_JA,
				     JMP_PLACEHOLDER_NEXT, 0, 0);

			/* nr = nr & ~mask */
			SET_BPF_STMT(&filter[pos++], BPF_ALU | BPF_AND | BPF_K,
				     ~audit_arch_vec[p].flag);
		}
#endif

		/* X = 1 << nr % 32 = 1 << nr & 0x1F; */
		SET_BPF_STMT(&filter[pos++], BPF_ALU | BPF_AND | BPF_K, 0x1F);
		SET_BPF_STMT(&filter[pos++], BPF_MISC | BPF_TAX, 0);
		SET_BPF_STMT(&filter[pos++], BPF_LD | BPF_IMM, 1);
		SET_BPF_STMT(&filter[pos++], BPF_ALU | BPF_LSH | BPF_X, 0);
		SET_BPF_STMT(&filter[pos++], BPF_MISC | BPF_TAX, 0);

		/* A = nr / 32 = n >> 5; */
		SET_BPF_STMT(&filter[pos++], BPF_LD | BPF_W | BPF_ABS,
			     offsetof(struct seccomp_data, nr));
		if (audit_arch_vec[p].flag) {
			/* nr = nr & ~mask */
			SET_BPF_STMT(&filter[pos++], BPF_ALU | BPF_AND | BPF_K,
				     ~audit_arch_vec[p].flag);
		}
		SET_BPF_STMT(&filter[pos++], BPF_ALU | BPF_RSH | BPF_K, 5);

		for (i = 0; i < nsyscall_vec[p] && pos <= BPF_MAXINSNS; ++i) {
			if (traced_by_seccomp(i, p))
				bitarray |= (1 << i % 32);
			if (i % 32 == 31) {
				pos += bpf_syscalls_match(filter + pos,
							  bitarray, i / 32);
				bitarray = 0;
			}
		}
		if (i % 32 != 0)
			pos += bpf_syscalls_match(filter + pos, bitarray,
						  i / 32);

		end = pos;

		SET_BPF_STMT(&filter[pos++], BPF_RET | BPF_K,
			     SECCOMP_RET_ALLOW);
		SET_BPF_STMT(&filter[pos++], BPF_RET | BPF_K,
			     SECCOMP_RET_TRACE);

		if (pos - start > UCHAR_MAX) {
			*overflow = true;
			return pos;
		}

		for (unsigned int i = start; i < end; ++i) {
			if (BPF_CLASS(filter[i].code) != BPF_JMP)
				continue;
			unsigned char jmp_next = pos - i - 1;
			unsigned char jmp_trace = pos - i - 2;
			unsigned char jmp_allow = pos - i - 3;
			replace_jmp_placeholders(&filter[i].jt, jmp_next,
						 jmp_trace, jmp_allow);
			replace_jmp_placeholders(&filter[i].jf, jmp_next,
						 jmp_trace, jmp_allow);
			if (BPF_OP(filter[i].code) == BPF_JA)
				filter[i].k = (unsigned int)jmp_next;
		}
	}

#if SUPPORTED_PERSONALITIES > 1
	SET_BPF_STMT(&filter[pos++], BPF_RET | BPF_K, SECCOMP_RET_TRACE);
#endif

	return pos;
}

static void
check_seccomp_filter_properties(void)
{
	int rc = prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, NULL, 0, 0);
	seccomp_filtering = rc < 0 && errno != EINVAL;
	if (!seccomp_filtering) {
		debug_func_perror_msg("prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER)");
		return;
	}

	for (unsigned int i = 0; i < ARRAY_SIZE(filter_generators); ++i) {
		bool overflow = false;
		unsigned short len = filter_generators[i](filters[i],
							  &overflow);
		if (len < bpf_prog.len && !overflow) {
			bpf_prog.len = len;
			bpf_prog.filter = filters[i];
		}
	}
	if (bpf_prog.len == USHRT_MAX) {
		debug_msg("seccomp filter disabled due to jump offset "
			  "overflow");
		seccomp_filtering = false;
	} else if (bpf_prog.len > BPF_MAXINSNS) {
		debug_msg("seccomp filter disabled due to BPF program "
			  "being oversized (%u > %d)", bpf_prog.len,
			  BPF_MAXINSNS);
		seccomp_filtering = false;
	}

	if (seccomp_filtering)
		check_seccomp_order();
}

static void
dump_seccomp_bpf(void)
{
	const struct sock_filter *filter = bpf_prog.filter;
	for (unsigned int i = 0; i < bpf_prog.len; ++i) {
		switch (filter[i].code) {
		case BPF_LD | BPF_W | BPF_ABS:
			switch (filter[i].k) {
			case offsetof(struct seccomp_data, arch):
				error_msg("STMT(BPF_LDWABS, data->arch)");
				break;
			case offsetof(struct seccomp_data, nr):
				error_msg("STMT(BPF_LDWABS, data->nr)");
				break;
			default:
				error_msg("STMT(BPF_LDWABS, 0x%x)",
					  filter[i].k);
			}
			break;
		case BPF_LD + BPF_W + BPF_IMM:
			error_msg("STMT(BPF_LDWIMM, 0x%x)", filter[i].k);
			break;
		case BPF_RET | BPF_K:
			switch (filter[i].k) {
			case SECCOMP_RET_TRACE:
				error_msg("STMT(BPF_RET, SECCOMP_RET_TRACE)");
				break;
			case SECCOMP_RET_ALLOW:
				error_msg("STMT(BPF_RET, SECCOMP_RET_ALLOW)");
				break;
			default:
				error_msg("STMT(BPF_RET, 0x%x)", filter[i].k);
			}
			break;
		case BPF_JMP | BPF_JEQ | BPF_K:
			error_msg("JUMP(BPF_JEQ, %u, %u, %u)",
				  filter[i].jt, filter[i].jf,
				  filter[i].k);
			break;
		case BPF_JMP | BPF_JGE | BPF_K:
			error_msg("JUMP(BPF_JGE, %u, %u, %u)",
				  filter[i].jt, filter[i].jf,
				  filter[i].k);
			break;
		case BPF_JMP + BPF_JSET + BPF_K:
			error_msg("JUMP(BPF_JSET, %u, %u, 0x%x)",
				  filter[i].jt, filter[i].jf,
				  filter[i].k);
			break;
		case BPF_JMP | BPF_JA:
			error_msg("JUMP(BPF_JA, %u)", filter[i].k);
			break;
		case BPF_ALU + BPF_RSH + BPF_K:
			error_msg("STMT(BPF_RSH, %u)", filter[i].k);
			break;
		case BPF_ALU + BPF_LSH + BPF_X:
			error_msg("STMT(BPF_LSH, X)");
			break;
		case BPF_ALU + BPF_AND + BPF_K:
			error_msg("STMT(BPF_AND, 0x%x)", filter[i].k);
			break;
		case BPF_MISC + BPF_TAX:
			error_msg("STMT(BPF_TAX)");
			break;
		case BPF_MISC + BPF_TXA:
			error_msg("STMT(BPF_TXA)");
			break;
		default:
			error_msg("STMT(0x%x, %u, %u, 0x%x)", filter[i].code,
				  filter[i].jt, filter[i].jf, filter[i].k);
		}
	}
}

void
init_seccomp_filter(void)
{
	if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) < 0)
		perror_func_msg_and_die("prctl(PR_SET_NO_NEW_PRIVS)");

	if (debug_flag)
		dump_seccomp_bpf();

	if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &bpf_prog) < 0)
		perror_func_msg_and_die("prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER)");
}

int
seccomp_filter_restart_operator(const struct tcb *tcp)
{
	if (exiting(tcp) && tcp->scno < nsyscall_vec[current_personality]
	    && traced_by_seccomp(tcp->scno, current_personality))
		return PTRACE_SYSCALL;
	return PTRACE_CONT;
}

void
check_seccomp_filter(void)
{
	/* Let's avoid enabling seccomp if all syscalls are traced. */
	seccomp_filtering = !is_complete_set_array(trace_set, nsyscall_vec,
						   SUPPORTED_PERSONALITIES);
	if (!seccomp_filtering) {
		error_msg("Seccomp filter is requested "
			  "but there are no syscalls to filter.  "
			  "See -e trace to filter syscalls.");
		return;
	}

	check_seccomp_filter_properties();

	if (!seccomp_filtering)
		error_msg("seccomp filter is requested but unavailable");
}
