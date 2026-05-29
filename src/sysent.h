/*
 * Copyright (c) 2016-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_SYSENT_H
# define STRACE_SYSENT_H

struct tcb;

typedef struct sysent {
	unsigned nargs;
	int	sys_flags;
	int	sen;
	int	(*sys_func)(struct tcb *);
	const char *sys_name;
} struct_sysent;

# define TRACE_FILE			000000001	/* Trace file-related syscalls. */
# define TRACE_IPC			000000002	/* Trace IPC-related syscalls. */
# define TRACE_NETWORK			000000004	/* Trace network-related syscalls. */
# define TRACE_PROCESS			000000010	/* Trace process-related syscalls. */
# define TRACE_SIGNAL			000000020	/* Trace signal-related syscalls. */
# define TRACE_DESC			000000040	/* Trace file descriptor-related syscalls. */
# define TRACE_MEMORY			000000100	/* Trace memory mapping-related syscalls. */
# define SYSCALL_NEVER_FAILS		000000200	/* Syscall is always successful. */
# define MEMORY_MAPPING_CHANGE		000000400	/* Trigger proc/maps cache updating */
# define TRACE_INDIRECT_SUBCALL		000001000	/* Syscall is an indirect socket/ipc subcall. */
# define COMPAT_SYSCALL_TYPES		000002000	/* A compat syscall that uses compat types. */
# define TRACE_STAT			000004000	/* Trace {,*_}{,old}{,x}stat{,64} syscalls. */
# define TRACE_LSTAT			000010000	/* Trace *lstat* syscalls. */
# define TRACE_STATFS			000020000	/* Trace statfs, statfs64, and statvfs syscalls. */
# define TRACE_FSTATFS			000040000	/* Trace fstatfs, fstatfs64 and fstatvfs syscalls. */
# define TRACE_STATFS_LIKE		000100000	/* Trace statfs-like, fstatfs-like and ustat syscalls. */
# define TRACE_FSTAT			000200000	/* Trace *fstat{,at}{,64} syscalls. */
# define TRACE_STAT_LIKE		000400000	/* Trace *{,l,f}stat{,x,at}{,64} syscalls. */
# define TRACE_PURE			001000000	/* Trace getter syscalls with no arguments. */
# define TRACE_SECCOMP_DEFAULT		002000000	/* Syscall is traced by seccomp filter by default. */
# define TRACE_CREDS			004000000	/* Trace process credentials-related syscalls. */
# define TRACE_CLOCK			010000000	/* Trace syscalls reading or modifying system clocks. */
# define COMM_CHANGE			020000000	/* Trigger /proc/$pid/comm cache update. */

#endif /* !STRACE_SYSENT_H */
