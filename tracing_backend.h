/*
 * Tracing backend interface header.
 *
 * Copyright (c) 2017-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_TRACING_BACKEND_H
#define STRACE_TRACING_BACKEND_H

#include "defs.h"

#include "trace_event.h"


#if defined IN_MPERS || defined IN_MPERS_BOOTSTRAP
typedef int strace_stat_t;
#else
# include "largefile_wrappers.h"

strace_stat_t;
#endif

#if ADDITIONAL_TRACING_BACKENDS

struct msghdr;


/**
 * Structure with pointers to functions (and some other data) that are specific
 * to a tracing backend. Tracing backend is currently assumed to be more or less
 * ptrace-like, as a result, it has some peculiarities.
 */
struct tracing_backend {
	const char *name;

	/* Initialisation and cleanup routines */

	/** Parse tracing-backend-specific argument */
	bool (*handle_arg) (char arg, char *optarg);
	/** Initialisation of the tracing backend */
	bool (*init) (int argc, char *argv[]);
	/** Optional. Called at the final initialisation stage, after attach */
	void (*post_init) (void);
	/** Optional. Called at the end of strace.c:cleanup(). */
	void (*cleanup) (int sig);

	/* XXX Merge these two in start_init */
        bool (*prog_pid_check) (char *exec_name, int nprocs);
	bool (*verify_args) (const char *username, bool daemon,
			     unsigned int *follow_fork);

	/* Tracee creation/reaping and attaching/detaching */

	void (*startup_child) (char **argv, char **env);
	void (*attach_tcb) (struct tcb *tcp);
	void (*detach) (struct tcb *tcp);
	int (*kill) (struct tcb *tcp, int sig);

	/* Tracing loop data manipulation. */

	size_t trace_wait_data_size;
	struct tcb_wait_data * (*init_trace_wait_data) (void *p);
	struct tcb_wait_data * (*copy_trace_wait_data) (struct
							tcb_wait_data *wd);
	void (*free_trace_wait_data) (struct tcb_wait_data *wd);

	/* Functions related to the main tracing loop. */

	/** Return new event */
	struct tcb_wait_data * (*next_event) (void);
	/** Additional handling for TE_STOP_BEFORE_EXECVE */
	void (*handle_exec) (struct tcb **current_tcp,
			     struct tcb_wait_data *wd);
	/** Restart process after successful processing */
	bool (*restart_process)(struct tcb *tcp, struct tcb_wait_data *wd);

	/* Tracee's memory/register/metadata manipulation routines */

	void (*clear_regs) (struct tcb *tcp);
	long (*get_regs) (struct tcb * const tcp);
	int (*get_scno) (struct tcb *tcp);
	int (*set_scno) (struct tcb *tcp, kernel_ulong_t scno);
	/** Set tracee's error code in accordance with tcb's data */
	void (*set_error) (struct tcb *tcp, unsigned long new_error);
	/** Set tracee's return code in accordance with tcb's data */
	void (*set_success) (struct tcb *tcp, kernel_long_t new_rval);
	bool (*get_instruction_pointer) (struct tcb *tcp, kernel_ulong_t *ip);
	bool (*get_stack_pointer)(struct tcb *tcp, kernel_ulong_t *sp);
	int (*get_syscall_args) (struct tcb *tcp);
	int (*get_syscall_result) (struct tcb *tcp);

	int (*umoven) (struct tcb *const tcp, kernel_ulong_t addr,
		       unsigned int len, void *const our_addr);
	int (*umovestr) (struct tcb *const tcp, kernel_ulong_t addr,
			 unsigned int len, char *laddr);
	int (*upeek) (struct tcb *tcp, unsigned long off, kernel_ulong_t *res);
	int (*upoke) (struct tcb *tcp, unsigned long off, kernel_ulong_t val);

	/*
	 * File I/O
	 *
	 * As of now, these functions are deemed to be executed in the context
	 * (namespace) of the tracer on the target machine and not
	 * in the context (namespace) of the specific tracee.
	 */

	/**
	 * Resolve a path.
	 * Optional (can be implemented via open+readlink+close).
         * Used by: pathtrace_select_set.
	 */
	char * (*realpath) (struct tcb *tcp, const char *path,
			    char *resolved_path);
	/**
	 * Open a file.
	 * Used by: read_int_from_file, realpath.
	 */
	int (*open) (struct tcb *tcp, const char *path, int flags, int mode);
	/**
	 * Read from file, with interface similar to pread(2).
	 * Used by: read_int_from_file.
	 */
	ssize_t (*pread) (struct tcb *tcp, int fd, void *buf, size_t count,
			  off_t offset);
	/**
	 * Close file.
	 * Used by: read_int_from_file, realpath.
	 */
	int (*close) (struct tcb *tcp, int fd);
	/**
	 * Read symlink contents.
	 * Used by: getfdpath, realpath.
	 */
	ssize_t (*readlink) (struct tcb *tcp, const char *path, char *buf,
			     size_t buf_size);
	/**
	 * Get file status.
	 * Optional. Used by: printdev
	 */
	int (*stat) (struct tcb *tcp, const char *path, strace_stat_t *buf);
	/**
	 * Get file status.
	 * Optional. Used by: tracee_stat
	 */
	int (*fstat) (struct tcb *tcp, int fd, strace_stat_t *buf);
	/**
	 * Get extended attributes of a file.
	 * Optional. Used by: getfdproto.
	 */
	ssize_t (*getxattr) (struct tcb *tcp, const char *path,
			     const char *name, void *buf, size_t buf_size);
	/**
	 * Create a socket.
	 * Optional. Used by: get_sockaddr_by_inode_uncached,
	 *                    genl_families_xlat.
	 */
	int (*socket) (struct tcb *tcp, int domain, int type, int protocol);
	/**
	 * Send a message via socket.
	 * Optional. Used by: send_query
	 * It's tracing backend responsibility to convert struct msghdr fields
	 * to tracee's format and back, it should be a drop-in replacement
	 * for users. Users, however, are responsible for proper generation
	 * of message data in target's format. */
	ssize_t (*sendmsg) (struct tcb *tcp, int fd, const struct msghdr *msg,
			    int flags);
	/**
	 * Receive a message via socket.
	 * Optional. Used by: receive_response
	 * It's tracing backend responsibility to convert struct msghdr fields
	 * to tracee's format and back, it should be a drop-in replacement
	 * for users. Users, however, are responsible for proper interpretation
	 * of message data as provided in target's format. */
	ssize_t (*recvmsg) (struct tcb *tcp, int fd, struct msghdr *msg,
			    int flags);
};


/* Tracing backend management interface. */

extern const struct tracing_backend *cur_tracing_backend;

extern void set_tracing_backend(struct tracing_backend *backend);

/**
 * Fallback handler for tracing backends that implement fstat, but no stat.
 */
extern int tracing_backend_stat_via_fstat(struct tcb *tcp, const char *path,
					  strace_stat_t *buf);


/* I/O function wrappers for tracing backends that are run on the same system */

extern int local_kill(struct tcb *tcp, int sig);
extern char *local_realpath(struct tcb *tcp, const char *path,
			    char *resolved_path);
extern int local_open(struct tcb *tcp, const char *path, int flags, int mode);
extern ssize_t local_pread(struct tcb *tcp, int fd, void *buf, size_t count,
			   off_t offset);
extern int local_close(struct tcb *tcp, int fd);
extern ssize_t local_readlink(struct tcb *tcp, const char *path, char *buf,
			      size_t buf_size);
extern int local_stat(struct tcb *tcp, const char *path, strace_stat_t *buf);
extern int local_fstat(struct tcb *tcp, int fd, strace_stat_t *buf);
extern ssize_t local_getxattr(struct tcb *tcp, const char *path,
			      const char *name, void *buf, size_t buf_size);
extern int local_socket(struct tcb *tcp, int domain, int type, int protocol);
extern ssize_t local_sendmsg(struct tcb *tcp, int fd, const struct msghdr *msg,
			     int flags);
extern ssize_t local_recvmsg(struct tcb *tcp, int fd, struct msghdr *msg,
			     int flags);


/* Wrappers for tracing-backend-specific calls. */

static inline const char *
tracing_backend_name(void)
{
	return cur_tracing_backend->name;
}

static inline bool
tracing_backend_handle_arg(char arg, char *optarg)
{
	if (cur_tracing_backend->handle_arg)
		return cur_tracing_backend->handle_arg(arg, optarg);

	return false;
}

static inline bool
tracing_backend_init(int argc, char *argv[])
{
	if (cur_tracing_backend->init)
		return cur_tracing_backend->init(argc, argv);

	return true;
}

static inline void
tracing_backend_post_init(void)
{
	if (cur_tracing_backend->post_init)
		cur_tracing_backend->post_init();
}

static inline void
tracing_backend_cleanup(int sig)
{
	if (cur_tracing_backend->cleanup)
		cur_tracing_backend->cleanup(sig);
}

static inline void
startup_child(char **argv, char **env)
{
	if (cur_tracing_backend->startup_child)
		cur_tracing_backend->startup_child(argv, env);
}

static inline void
attach_tcb(struct tcb *const tcp)
{
	if (cur_tracing_backend->attach_tcb)
		cur_tracing_backend->attach_tcb(tcp);
}

static inline void
detach(struct tcb *tcp)
{
	if (cur_tracing_backend->detach)
		cur_tracing_backend->detach(tcp);
}

/*
 * trace_wait_data_size, init_trace_wait_data, copy_trace_wait_data,
 * and free_trace_wait_data shorthands are defined int tcb_wait_data.h.
 */

static inline struct tcb_wait_data *
next_event(void)
{
	return cur_tracing_backend->next_event();
}

static inline void
handle_exec(struct tcb **current_tcp, struct tcb_wait_data *wd)
{
	if (cur_tracing_backend->handle_exec)
		cur_tracing_backend->handle_exec(current_tcp, wd);
}

static inline bool
restart_process(struct tcb *current_tcp, struct tcb_wait_data *wd)
{
	return cur_tracing_backend->restart_process(current_tcp, wd);
}

static inline void
clear_regs(struct tcb *tcp)
{
	cur_tracing_backend->clear_regs(tcp);
}

static inline long
get_regs(struct tcb * const tcp)
{
	return cur_tracing_backend->get_regs(tcp);
}

static inline int
get_scno(struct tcb *tcp)
{
	return cur_tracing_backend->get_scno(tcp);
}

static inline int
set_scno(struct tcb *tcp, kernel_ulong_t scno)
{
	return cur_tracing_backend->set_scno(tcp, scno);
}

static inline void
set_error(struct tcb *tcp, unsigned long new_error)
{
	cur_tracing_backend->set_error(tcp, new_error);
}

static inline void
set_success(struct tcb *tcp, kernel_long_t new_rval)
{
	cur_tracing_backend->set_success(tcp, new_rval);
}

static inline bool
get_instruction_pointer(struct tcb *tcp, kernel_ulong_t *ip)
{
	return (cur_tracing_backend->get_instruction_pointer
		?: generic_get_instruction_pointer)(tcp, ip);
}

static inline bool
get_stack_pointer(struct tcb *tcp, kernel_ulong_t *sp)
{
	return (cur_tracing_backend->get_stack_pointer
		?: generic_get_stack_pointer)(tcp, sp);
}

static inline int
get_syscall_args(struct tcb *tcp)
{
	return cur_tracing_backend->get_syscall_args(tcp);
}

static inline int
get_syscall_result(struct tcb *tcp)
{
	return cur_tracing_backend->get_syscall_result(tcp);
}

static inline int
umoven(struct tcb *tcp, kernel_ulong_t addr, unsigned int len, void *laddr)
{
	return cur_tracing_backend->umoven(tcp, addr, len, laddr);
}

static inline int
umovestr(struct tcb *tcp, kernel_ulong_t addr, unsigned int len, char *laddr)
{
	return cur_tracing_backend->umovestr(tcp, addr, len, laddr);
}

static inline int
upeek(struct tcb *tcp, unsigned long off, kernel_ulong_t *res)
{
	return cur_tracing_backend->upeek(tcp, off, res);
}

static inline int
upoke(struct tcb *tcp, unsigned long off, kernel_ulong_t val)
{
	return cur_tracing_backend->upoke(tcp, off, val);
}

static inline int
tracee_kill(struct tcb *tcp, int sig)
{
	return cur_tracing_backend->kill(tcp, sig);
}

static inline char *
tracee_realpath(struct tcb *tcp, const char *path, char *resolved_path)
{
	if (cur_tracing_backend->realpath)
		return cur_tracing_backend->realpath(tcp, path, resolved_path);

	return NULL;
}

# define error_set_errno(err_code_) (errno = (err_code_), -1)

static inline int
tracee_open(struct tcb *tcp, const char *path, int flags, int mode)
{
	return cur_tracing_backend->open(tcp, path, flags, mode);
}

static inline ssize_t
tracee_pread(struct tcb *tcp, int fd, void *buf, size_t count, off_t offset)
{
	return cur_tracing_backend->pread(tcp, fd, buf, count, offset);
}

static inline int
tracee_close(struct tcb *tcp, int fd)
{
	return cur_tracing_backend->close(tcp, fd);
}

static inline ssize_t
tracee_readlink(struct tcb *tcp, const char *path, char *buf, size_t buf_size)
{
	return cur_tracing_backend->readlink(tcp, path, buf, buf_size);
}

static inline int
tracee_stat(struct tcb *tcp, const char *path, strace_stat_t *buf)
{
	if (cur_tracing_backend->stat)
		return cur_tracing_backend->stat(tcp, path, buf);

	if (cur_tracing_backend->fstat)
		return tracing_backend_stat_via_fstat(tcp, path, buf);

	return error_set_errno(ENOSYS);
}

static inline int
tracee_fstat(struct tcb *tcp, int fd, strace_stat_t *buf)
{
	if (cur_tracing_backend->fstat)
		return cur_tracing_backend->fstat(tcp, fd, buf);

	return error_set_errno(ENOSYS);
}

static inline ssize_t
tracee_getxattr(struct tcb *tcp, const char *path, const char *name, void *buf,
		size_t buf_size)
{
	if (cur_tracing_backend->getxattr)
		return cur_tracing_backend->getxattr(tcp, path, name, buf,
						     buf_size);

	return error_set_errno(ENOSYS);
}

static inline int
tracee_socket(struct tcb *tcp, int domain, int type, int protocol)
{
	if (cur_tracing_backend->socket)
		return cur_tracing_backend->socket(tcp, domain, type, protocol);

	return error_set_errno(ENOSYS);
}

static inline ssize_t
tracee_sendmsg(struct tcb *tcp, int fd, const struct msghdr *msg, int flags)
{
	if (cur_tracing_backend->sendmsg)
		return cur_tracing_backend->sendmsg(tcp, fd, msg, flags);

	return error_set_errno(ENOSYS);
}

static inline ssize_t
tracee_recvmsg(struct tcb *tcp, int fd, struct msghdr *msg, int flags)
{
	if (cur_tracing_backend->recvmsg)
		return cur_tracing_backend->recvmsg(tcp, fd, msg, flags);

	return error_set_errno(ENOSYS);
}

#else /* !ADDITIONAL_TRACING_BACKENDS */

# include "ptrace_backend.h"

# define tracing_backend_name()      "ptrace"
# define tracing_backend_handle_arg(a_, o_) false
# define tracing_backend_init        ptrace_init
# define tracing_backend_post_init() ((void) 0)
# define tracing_backend_cleanup(s_) ((void) 0)

# define startup_child               ptrace_startup_child
# define attach_tcb                  ptrace_attach_tcb
# define detach                      ptrace_detach

# define next_event                  ptrace_next_event
# define handle_group_stop           ptrace_handle_group_stop
# define handle_exec                 ptrace_handle_exec
# define restart_process             ptrace_restart_process

# define clear_regs                  ptrace_clear_regs
# define get_regs                    ptrace_get_regs
# define get_scno                    ptrace_get_scno
# define set_scno                    ptrace_set_scno
# define set_error                   ptrace_set_error
# define set_success                 ptrace_set_success
# define get_instruction_pointer     ptrace_get_instruction_pointer
# define get_stack_pointer           ptrace_get_stack_pointer
# define get_syscall_args            ptrace_get_syscall_args
# define get_syscall_result          ptrace_get_syscall_result

# define umoven                      ptrace_umoven
# define umovestr                    ptrace_umovestr
# define upeek                       ptrace_upeek
# define upoke                       ptrace_upoke

# define tracee_kill(tcp_, sig_) \
	kill((tcp_)->pid, (sig_))
# define tracee_realpath(tcp_, path_, resolved_path_) \
	realpath((path_), (resolved_path_))
# define tracee_open(tcp_, path_, flags_, mode_) \
	open_file((path_), (flags_), (mode_))
# define tracee_pread(tcp_, fd_, buf_, count_, offset_) \
	pread((fd_), (buf_), (count_), (offset_))
# define tracee_close(tcp_, fd_) \
	close(fd_)
# define tracee_readlink(tcp_, path_, buf_, buf_size_) \
	readlink((path_), (buf_), (buf_size_))
# define tracee_stat(tcp_, path_, buf_) \
	stat_file((path_), (buf_))
# define tracee_fstat(tcp_, fd_, buf_) \
	fstat_file((fd_), (buf_))
# define tracee_getxattr(tcp_, path_, name_, buf_, buf_size_) \
	getxattr((path_), (name_), (buf_), (buf_size_))
# define tracee_socket(tcp_, domain_, type_, protocol_) \
	socket((domain_), (type_), (protocol_))
# define tracee_sendmsg(tcp_, fd_, msg_, flags_) \
	sendmsg((fd_), (msg_), (flags_))
# define tracee_recvmsg(tcp_, fd_, msg_, flags_) \
	recvmsg((fd_), (msg_), (flags_))

#endif /* !ADDITIONAL_TRACING_BACKENDS */

#endif /* !STRACE_TRACING_BACKEND_H */
