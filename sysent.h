#ifndef STRACE_SYSENT_H
#define STRACE_SYSENT_H

typedef struct sysent {
	unsigned nargs;
	int	sys_flags;
	int	sen;
	int	(*sys_func)();
	const char *sys_name;
} struct_sysent;

#define TRACE_FILE		001	/* Trace file-related syscalls. */
#define TRACE_IPC		002	/* Trace IPC-related syscalls. */
#define TRACE_NETWORK		004	/* Trace network-related syscalls. */
#define TRACE_PROCESS		010	/* Trace process-related syscalls. */
#define TRACE_SIGNAL		020	/* Trace signal-related syscalls. */
#define TRACE_DESC		040	/* Trace file descriptor-related syscalls. */
#define TRACE_MEMORY		0100	/* Trace memory mapping-related syscalls. */
#define SYSCALL_NEVER_FAILS	0200	/* Syscall is always successful. */
#define STACKTRACE_INVALIDATE_CACHE 0400  /* Trigger proc/maps cache updating */
#define STACKTRACE_CAPTURE_ON_ENTER 01000 /* Capture stacktrace on "entering" stage */
#define TRACE_INDIRECT_SUBCALL	02000	/* Syscall is an indirect socket/ipc subcall. */
#define COMPAT_SYSCALL_TYPES	04000	/* A compat syscall that uses compat types. */

#endif /* !STRACE_SYSENT_H */
