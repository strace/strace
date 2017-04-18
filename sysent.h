#ifndef STRACE_SYSENT_H
#define STRACE_SYSENT_H

typedef struct sysent {
	unsigned nargs;
	int	sys_flags;
	int	sen;
	int	(*sys_func)();
	const char *sys_name;
} struct_sysent;

#define TRACE_FILE			00000001	/* Trace file-related syscalls. */
#define TRACE_IPC			00000002	/* Trace IPC-related syscalls. */
#define TRACE_NETWORK			00000004	/* Trace network-related syscalls. */
#define TRACE_PROCESS			00000010	/* Trace process-related syscalls. */
#define TRACE_SIGNAL			00000020	/* Trace signal-related syscalls. */
#define TRACE_DESC			00000040	/* Trace file descriptor-related syscalls. */
#define TRACE_MEMORY			00000100	/* Trace memory mapping-related syscalls. */
#define SYSCALL_NEVER_FAILS		00000200	/* Syscall is always successful. */
#define STACKTRACE_INVALIDATE_CACHE	00000400	/* Trigger proc/maps cache updating */
#define STACKTRACE_CAPTURE_ON_ENTER	00001000	/* Capture stacktrace on "entering" stage */
#define TRACE_INDIRECT_SUBCALL		00002000	/* Syscall is an indirect socket/ipc subcall. */
#define COMPAT_SYSCALL_TYPES		00004000	/* A compat syscall that uses compat types. */
#define TRACE_STAT			00010000	/* Trace {,*_}{,old}{,x}stat{,64} syscalls. */
#define TRACE_LSTAT			00020000	/* Trace *lstat* syscalls. */
#define TRACE_STATFS			00040000	/* Trace statfs, statfs64, and statvfs syscalls. */
#define TRACE_FSTATFS			00100000	/* Trace fstatfs, fstatfs64 and fstatvfs syscalls. */
#define TRACE_STATFS_LIKE		00200000	/* Trace statfs-like, fstatfs-like and ustat syscalls. */
#define TRACE_FSTAT			00400000	/* Trace *fstat{,at}{,64} syscalls. */
#define TRACE_STAT_LIKE			01000000	/* Trace *{,l,f}stat{,x,at}{,64} syscalls. */

#endif /* !STRACE_SYSENT_H */
