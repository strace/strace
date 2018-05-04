/*
 * Copyright (c) 2018 Chen Jingpiao <chenjingpiao@gmail.com>
 * Copyright (c) 2018 The strace developers.
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

#include "defs.h"

#include "ptrace.h"
#include <sys/prctl.h>
#include <sys/wait.h>
#include <linux/audit.h>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <asm/unistd.h>
#include <signal.h>

#include "filter_seccomp.h"
#include "number_set.h"

bool enable_seccomp_filter = false;
bool seccomp_before_ptrace;

static void
check_seccomp_order_do_child(void)
{
	struct sock_filter filter[] = {
		BPF_STMT(BPF_LD + BPF_W + BPF_ABS,
			 offsetof(struct seccomp_data, nr)),
		BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_getuid, 0, 1),
		BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_TRACE),
		BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_ALLOW)
	};

	struct sock_fprog prog = {
		.len = ARRAY_SIZE(filter),
		.filter = filter
	};
	if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) < 0)
		perror_func_msg_and_die("ptrace(PTRACE_TRACEME, ...");
	if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) < 0)
		perror_func_msg_and_die("prctl(PR_SET_NO_NEW_PRIVS, 1, ...");
	if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &prog) < 0)
		perror_func_msg_and_die("prctl(PR_SET_SECCOMP)");
	kill(getpid(), SIGSTOP);
	syscall(__NR_getuid);
	pause();
	_exit(0);
}

static void
check_seccomp_order_tracer(int pid)
{
	int status, tracee_pid, flags = 0;

	while (1) {
		errno = 0;
		tracee_pid = waitpid(pid, &status, 0);
		if (tracee_pid <= 0) {
			if (errno == EINTR)
				continue;
			perror_func_msg_and_die("unexpected wait result %d",
						tracee_pid);
		}
		if (flags == 0) {
			if (ptrace(PTRACE_SETOPTIONS, pid, 0,
				   PTRACE_O_TRACESECCOMP) < 0)
				perror_func_msg_and_die("ptrace(PTRACE_SETOPTIONS, ...");
			if (ptrace(PTRACE_SYSCALL, pid, NULL, NULL) < 0)
				perror_func_msg_and_die("ptrace(PTRACE_SYSCALL, ...");
		} else if (flags == 1) {
			if ((status >> 16) == PTRACE_EVENT_SECCOMP)
				seccomp_before_ptrace = true;
			else
				seccomp_before_ptrace = false;
			kill(pid, SIGKILL);
		} else {
			if (WIFSIGNALED(status))
				break;

			error_func_msg_and_die("unexpected wait status %#x",
					       status);
		}
		flags++;
	}
}

static void
check_seccomp_order(void)
{
	int pid;

	pid = fork();
	if (pid < 0)
		perror_func_msg_and_die("fork");

	if (pid == 0)
		check_seccomp_order_do_child();

	check_seccomp_order_tracer(pid);
}

static bool
traced_by_seccomp(unsigned int scno, unsigned int p)
{
	return !sysent_vec[p][scno].sys_func
	       || sysent_vec[p][scno].sys_flags & TRACE_INDIRECT_SUBCALL
	       || is_number_in_set_array(scno, trace_set, p)
	       || strcmp("execve", sysent_vec[p][scno].sys_name) == 0
	       || strcmp("execveat", sysent_vec[p][scno].sys_name) == 0
#if defined SPARC || defined SPARC64
	       || strcmp("execv", sysent_vec[p][scno].sys_name) == 0
#endif
	       || strcmp("socketcall", sysent_vec[p][scno].sys_name) == 0
	       || strcmp("ipc", sysent_vec[p][scno].sys_name) == 0
#ifdef LINUX_MIPSO32
	       || strcmp("syscall", sysent_vec[p][scno].sys_name) == 0
#endif
	       ;
}

static void
check_bpf_instruction_number(void)
{
	for (unsigned int p = 0; p < SUPPORTED_PERSONALITIES; ++p) {
		unsigned int lower = UINT_MAX, count = 0;

		for (unsigned int i = 0; i < nsyscall_vec[p]; ++i) {
			if (traced_by_seccomp(i, p)) {
				if (lower == UINT_MAX)
					lower = i;
				continue;
			}
			if (lower == UINT_MAX)
				continue;
			if (lower + 1 == i)
				count++;
			else
				count += 2;
			lower = UINT_MAX;
		}
		if (lower != UINT_MAX)
			count += 2;
		if (count > SECCOMP_TRACE_SYSCALL_MAX) {
			enable_seccomp_filter = false;
			break;
		}
	}
}

void
check_seccomp_filter(void)
{
	if (!enable_seccomp_filter)
		goto end;
#ifdef SECCOMP_MODE_FILTER
	int rc;

	if (NOMMU_SYSTEM) {
		enable_seccomp_filter = false;
		goto end;
	}

	rc = prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, NULL, 0, 0);
	if (rc < 0 && errno == EINVAL)
		enable_seccomp_filter = false;
	else
		enable_seccomp_filter = true;
	if (enable_seccomp_filter)
		check_bpf_instruction_number();
	if (enable_seccomp_filter)
		check_seccomp_order();
#else
	enable_seccomp_filter = false;
#endif
end:
	debug_msg("seccomp-filter: %s",
		  enable_seccomp_filter ? "enable" : "disable");
}

static unsigned short
bpf_add_traced_syscall(struct sock_filter *filter,
		       unsigned int lower, unsigned int upper)
{
	if (lower + 1 == upper) {
		/* filter[X].jt will set when return instruction added */
		SET_BPF_JUMP(filter, BPF_JMP + BPF_JEQ + BPF_K, lower, 0, 0);
		return 1;
	} else {
		SET_BPF_JUMP(filter, BPF_JMP + BPF_JGE + BPF_K, lower, 0, 1);
		++filter;
		/* filter[X].jf will set when return instruction added */
		SET_BPF_JUMP(filter, BPF_JMP + BPF_JGE + BPF_K, upper, 0, 0);
		return 2;
	}
}

static void
dump_seccomp_bpf(const struct sock_filter *filter, unsigned short len)
{
	for (unsigned int i = 0; i < len; ++i) {
		if (filter[i].code == BPF_LD + BPF_W + BPF_ABS) {
			debug_msg("STMT(BPF_LD + BPF_W + BPF_ABS, %u)", filter[i].k);
		} else if (filter[i].code == BPF_RET + BPF_K) {
			debug_msg("STMT(BPF_RET + BPF_K, %u)", filter[i].k);
		} else if (filter[i].code == BPF_JMP + BPF_JEQ + BPF_K) {
			debug_msg("JUMP(BPF_JMP + BPF_JEQ + BPF_K, %u, %u, %u)",
				  filter[i].jt, filter[i].jf, filter[i].k);
		} else if (filter[i].code == BPF_JMP + BPF_JGE + BPF_K) {
			debug_msg("JUMP(BPF_JMP + BPF_JGE + BPF_K, %u, %u, %u)",
				  filter[i].jt, filter[i].jf, filter[i].k);
		} else {
			debug_msg("STMT(%u, %u, %u, %u)",
				  filter[i].code, filter[i].jt,
				  filter[i].jf, filter[i].k);
		}
	}
}

static unsigned short
init_sock_filter(struct sock_filter *filter)
{
	unsigned short pos = 0;
#if SUPPORTED_PERSONALITIES > 1
	unsigned int audit_arch_vec[] = {
# if defined X86_64
		AUDIT_ARCH_X86_64,
		AUDIT_ARCH_I386,
		AUDIT_ARCH_X86_64
# elif SUPPORTED_PERSONALITIES == 2
		AUDIT_ARCH_X86_64,
		AUDIT_ARCH_I386
# endif
	};
#endif
	unsigned int syscall_bit_vec[] = {
#if defined X86_64
		0, 0, __X32_SYSCALL_BIT
#elif defined X32
		__X32_SYSCALL_BIT, 0
#elif SUPPORTED_PERSONALITIES == 2
		0, 0
#else
		0
#endif
	};

#if SUPPORTED_PERSONALITIES > 1
	SET_BPF_STMT(&filter[pos++], BPF_LD + BPF_W + BPF_ABS,
		     offsetof(struct seccomp_data, arch));
#endif
	for (unsigned int p = 0; p < SUPPORTED_PERSONALITIES; ++p) {
		unsigned int lower = UINT_MAX;
		unsigned short previous = pos, start, end;

#if SUPPORTED_PERSONALITIES > 1
		/* filter[X].jf will set when return instruction added */
		SET_BPF_JUMP(&filter[pos++], BPF_JMP + BPF_JEQ + BPF_K,
			     audit_arch_vec[p], 0, 0);
#endif
		SET_BPF_STMT(&filter[pos++], BPF_LD + BPF_W + BPF_ABS,
			     offsetof(struct seccomp_data, nr));

		start = pos;
		for (unsigned int i = 0; i < nsyscall_vec[p]; ++i) {
			if (traced_by_seccomp(i, p)) {
				if (lower == UINT_MAX)
					lower = i;
				continue;
			}
			if (lower == UINT_MAX)
				continue;
			pos += bpf_add_traced_syscall(filter + pos,
						      lower + syscall_bit_vec[p],
						      i + syscall_bit_vec[p]);
			lower = UINT_MAX;
		}
		if (lower != UINT_MAX)
			pos += bpf_add_traced_syscall(filter + pos,
						      lower + syscall_bit_vec[p],
						      nsyscall_vec[p] + syscall_bit_vec[p]);
		end = pos;

#ifdef X86_64
		if (p == 0) {
			SET_BPF_JUMP(&filter[pos++], BPF_JMP + BPF_JGE + BPF_K,
				     __X32_SYSCALL_BIT, 0, 2);
			SET_BPF_STMT(&filter[pos++], BPF_LD + BPF_W + BPF_ABS,
				     offsetof(struct seccomp_data, arch));
			SET_BPF_JUMP(&filter[pos++], BPF_JMP + BPF_JEQ + BPF_K,
				     AUDIT_ARCH_X86_64, 3, 0);

			SET_BPF_STMT(&filter[pos++], BPF_LD + BPF_W + BPF_ABS,
				     offsetof(struct seccomp_data, nr));
		}
#endif
		SET_BPF_JUMP(&filter[pos++], BPF_JMP + BPF_JGE + BPF_K,
			     nsyscall_vec[p] + syscall_bit_vec[p], 1, 0);

		SET_BPF_STMT(&filter[pos++], BPF_RET + BPF_K,
			     SECCOMP_RET_ALLOW);
		SET_BPF_STMT(&filter[pos++], BPF_RET + BPF_K,
			     SECCOMP_RET_TRACE);
		filter[previous].jf = pos - previous - 1;
		for (unsigned int i = start; i < end; ++i) {
			if (BPF_CLASS(filter[i].code) != BPF_JMP)
				continue;
			if (BPF_OP(filter[i].code) == BPF_JEQ)
				filter[i].jt = pos - i - 2;
			else if (BPF_OP(filter[i].code) == BPF_JGE
				 && filter[i].jf == 0)
				filter[i].jf = pos - i - 2;
		}
	}
#if SUPPORTED_PERSONALITIES > 1
	SET_BPF_STMT(&filter[pos++], BPF_RET + BPF_K, SECCOMP_RET_TRACE);
#endif

	dump_seccomp_bpf(filter, pos);

	return pos;
}

static void
do_seccomp(struct sock_fprog *prog)
{
	if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, prog) < 0)
		perror_msg_and_die("prctl");
}

void
init_seccomp_filter(void)
{
	struct sock_filter filter[SECCOMP_BPF_MAXINSNS];
	unsigned short len;

	len = init_sock_filter(filter);

	struct sock_fprog prog = {
		.len = len,
		.filter = filter
	};

	if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) < 0)
		perror_msg_and_die("prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)");

	do_seccomp(&prog);
}

int
seccomp_filter_restart_operator(const struct tcb *tcp)
{
	if (tcp
	    && (tcp->flags & TCB_INSYSCALL)
	    && traced_by_seccomp(tcp->scno, current_personality))
		return PTRACE_SYSCALL;
	return PTRACE_CONT;
}
