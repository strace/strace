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

#ifndef STRACE_SECCOMP_FILTER_H
#define STRACE_SECCOMP_FILTER_H

#include "defs.h"

#ifdef HAVE_LINUX_SECCOMP_H
# include <linux/seccomp.h>
#endif

#define SECCOMP_TRACE_SYSCALL_MAX	(SUPPORTED_PERSONALITIES * 150)
#define SECCOMP_BPF_MAXINSNS		(SECCOMP_TRACE_SYSCALL_MAX + 200)

extern bool enable_seccomp_filter;
extern bool seccomp_before_ptrace;

extern void check_seccomp_filter(void);
extern void init_seccomp_filter(void);
extern int seccomp_filter_restart_operator(const struct tcb *);

#define SET_BPF(filter, code, jt, jf, k) \
	(*(filter) = (struct sock_filter) { code, jt, jf, k })

#define SET_BPF_STMT(filter, code, k) \
	SET_BPF(filter, code, 0, 0, k)

#define SET_BPF_JUMP(filter, code, k, jt, jf) \
	SET_BPF(filter, code, jt, jf, k)

#endif /* !STRACE_SECCOMP_FILTER_H */
