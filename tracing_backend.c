/*
 * Implementation of functions related to the tracing backend interface.
 *
 * Copyright (c) 2017-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <fcntl.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "tracing_backend.h"


#if ADDITIONAL_TRACING_BACKENDS

#include <assert.h>
#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/xattr.h>

#include "ptrace_backend.h"

/* Tracing backeng management functions */

const struct tracing_backend *cur_tracing_backend = &ptrace_backend;

void
set_tracing_backend(struct tracing_backend *backend)
{
	/* Trying to catch uninitialised tracing backend fields early */
	assert(backend->name);

	assert(backend->kill);

	assert(backend->next_event);
	assert(backend->restart_process);

	assert(backend->clear_regs);
	assert(backend->get_regs);
	assert(backend->get_scno);
	assert(backend->set_scno);
	assert(backend->set_error);
	assert(backend->set_success);
	assert(backend->get_syscall_args);
	assert(backend->get_syscall_result);

	assert(backend->umoven);
	assert(backend->umovestr);
	assert(backend->upeek);
	assert(backend->upoke);

	assert(backend->open);
	assert(backend->pread);
	assert(backend->close);
	assert(backend->readlink);

	cur_tracing_backend = backend;
}

# ifdef O_PATH
#  define O_PATH_ O_PATH
# else
#  define O_PATH_ 0
# endif

int
tracing_backend_stat_via_fstat(struct tcb *tcp, const char *path,
			       strace_stat_t *buf)
{
	int fd = cur_tracing_backend->open(tcp, path, O_NOATIME | O_PATH_, 0);
	int ret;
	int errnum = 0;

	if (fd < 0)
		return -1;

	ret = cur_tracing_backend->fstat(tcp, fd, buf);
	if (ret < 0)
		errnum = errno;

	cur_tracing_backend->close(tcp, fd);
	errno = errnum;

	return ret;
}

#undef O_PATH_

/* Simple syscall wrappers for a local tracing backend */

int
local_kill(struct tcb *tcp, int sig)
{
	return kill(tcp->pid, sig);
}

char *
local_realpath(struct tcb *tcp, const char *path, char *resolved_path)
{
	return realpath(path, resolved_path);
}

int
local_open(struct tcb *tcp, const char *path, int flags, int mode)
{
	return open_file(path, flags, mode);
}

ssize_t
local_pread(struct tcb *tcp, int fd, void *buf, size_t count, off_t offset)
{
	return pread(fd, buf, count, offset);
}

int
local_close(struct tcb *tcp, int fd)
{
	return close(fd);
}

ssize_t
local_readlink(struct tcb *tcp, const char *path, char *buf, size_t buf_size)
{
	return readlink(path, buf, buf_size);
}

int
local_stat(struct tcb *tcp, const char *path, strace_stat_t *buf)
{
	return stat_file(path, buf);
}

int
local_fstat(struct tcb *tcp, int fd, strace_stat_t *buf)
{
	return fstat_file(fd, buf);
}

ssize_t
local_getxattr(struct tcb *tcp, const char *path, const char *name, void *buf,
		size_t buf_size)
{
	return getxattr(path, name, buf, buf_size);
}

int
local_socket(struct tcb *tcp, int domain, int type, int protocol)
{
	return socket(domain, type, protocol);
}

ssize_t
local_sendmsg(struct tcb *tcp, int fd, const struct msghdr *msg, int flags)
{
	return sendmsg(fd, msg, flags);
}

ssize_t
local_recvmsg(struct tcb *tcp, int fd, struct msghdr *msg, int flags)
{
	return recvmsg(fd, msg, flags);
}

#endif  /* ADDITIONAL_TRACING_BACKENDS */
