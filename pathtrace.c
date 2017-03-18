/*
 * Copyright (c) 2011, Comtrol Corp.
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
 *
 */

#include "defs.h"
#include <sys/param.h>
#include <poll.h>

#include "syscall.h"

const char **paths_selected = NULL;
static unsigned num_selected = 0;

/*
 * Return true if specified path matches one that we're tracing.
 */
static int
pathmatch(const char *path)
{
	unsigned i;

	for (i = 0; i < num_selected; ++i) {
		if (strcmp(path, paths_selected[i]) == 0)
			return 1;
	}
	return 0;
}

/*
 * Return true if specified path (in user-space) matches.
 */
static int
upathmatch(struct tcb *const tcp, const kernel_ulong_t upath)
{
	char path[PATH_MAX + 1];

	return umovestr(tcp, upath, sizeof path, path) > 0 &&
		pathmatch(path);
}

/*
 * Return true if specified fd maps to a path we're tracing.
 */
static int
fdmatch(struct tcb *tcp, int fd)
{
	char path[PATH_MAX + 1];
	int n = getfdpath(tcp, fd, path, sizeof(path));

	return n >= 0 && pathmatch(path);
}

/*
 * Add a path to the set we're tracing.
 * Specifying NULL will delete all paths.
 */
static void
storepath(const char *path)
{
	unsigned i;

	if (pathmatch(path))
		return; /* already in table */

	i = num_selected++;
	paths_selected = xreallocarray(paths_selected, num_selected,
				       sizeof(paths_selected[0]));
	paths_selected[i] = path;
}

/*
 * Get path associated with fd.
 */
int
getfdpath(struct tcb *tcp, int fd, char *buf, unsigned bufsize)
{
	char linkpath[sizeof("/proc/%u/fd/%u") + 2 * sizeof(int)*3];
	ssize_t n;

	if (fd < 0)
		return -1;

	sprintf(linkpath, "/proc/%u/fd/%u", tcp->pid, fd);
	n = readlink(linkpath, buf, bufsize - 1);
	/*
	 * NB: if buf is too small, readlink doesn't fail,
	 * it returns truncated result (IOW: n == bufsize - 1).
	 */
	if (n >= 0)
		buf[n] = '\0';
	return n;
}

/*
 * Add a path to the set we're tracing.  Also add the canonicalized
 * version of the path.  Secifying NULL will delete all paths.
 */
void
pathtrace_select(const char *path)
{
	char *rpath;

	storepath(path);

	rpath = realpath(path, NULL);

	if (rpath == NULL)
		return;

	/* if realpath and specified path are same, we're done */
	if (strcmp(path, rpath) == 0) {
		free(rpath);
		return;
	}

	error_msg("Requested path '%s' resolved into '%s'", path, rpath);
	storepath(rpath);
}

/*
 * Return true if syscall accesses a selected path
 * (or if no paths have been specified for tracing).
 */
int
pathtrace_match(struct tcb *tcp)
{
	const struct_sysent *s;

	s = tcp->s_ent;

	if (!(s->sys_flags & (TRACE_FILE | TRACE_DESC | TRACE_NETWORK)))
		return 0;

	/*
	 * Check for special cases where we need to do something
	 * other than test arg[0].
	 */

	switch (s->sen) {
	case SEN_dup2:
	case SEN_dup3:
	case SEN_kexec_file_load:
	case SEN_sendfile:
	case SEN_sendfile64:
	case SEN_tee:
		/* fd, fd */
		return fdmatch(tcp, tcp->u_arg[0]) ||
			fdmatch(tcp, tcp->u_arg[1]);

	case SEN_faccessat:
	case SEN_fchmodat:
	case SEN_fchownat:
	case SEN_fstatat64:
	case SEN_futimesat:
	case SEN_inotify_add_watch:
	case SEN_mkdirat:
	case SEN_mknodat:
	case SEN_name_to_handle_at:
	case SEN_newfstatat:
	case SEN_openat:
	case SEN_readlinkat:
	case SEN_statx:
	case SEN_unlinkat:
	case SEN_utimensat:
		/* fd, path */
		return fdmatch(tcp, tcp->u_arg[0]) ||
			upathmatch(tcp, tcp->u_arg[1]);

	case SEN_link:
	case SEN_mount:
	case SEN_pivotroot:
		/* path, path */
		return upathmatch(tcp, tcp->u_arg[0]) ||
			upathmatch(tcp, tcp->u_arg[1]);

	case SEN_quotactl:
		/* x, path */
		return upathmatch(tcp, tcp->u_arg[1]);

	case SEN_linkat:
	case SEN_renameat2:
	case SEN_renameat:
		/* fd, path, fd, path */
		return fdmatch(tcp, tcp->u_arg[0]) ||
			fdmatch(tcp, tcp->u_arg[2]) ||
			upathmatch(tcp, tcp->u_arg[1]) ||
			upathmatch(tcp, tcp->u_arg[3]);

	case SEN_old_mmap:
#if defined(S390)
	case SEN_old_mmap_pgoff:
#endif
	case SEN_mmap:
	case SEN_mmap_4koff:
	case SEN_mmap_pgoff:
	case SEN_ARCH_mmap:
		/* x, x, x, x, fd */
		return fdmatch(tcp, tcp->u_arg[4]);

	case SEN_symlinkat:
		/* path, fd, path */
		return fdmatch(tcp, tcp->u_arg[1]) ||
			upathmatch(tcp, tcp->u_arg[0]) ||
			upathmatch(tcp, tcp->u_arg[2]);

	case SEN_copy_file_range:
	case SEN_splice:
		/* fd, x, fd, x, x, x */
		return fdmatch(tcp, tcp->u_arg[0]) ||
			fdmatch(tcp, tcp->u_arg[2]);

	case SEN_epoll_ctl:
		/* x, x, fd, x */
		return fdmatch(tcp, tcp->u_arg[2]);


	case SEN_fanotify_mark:
		/* x, x, x, fd, path */
		return fdmatch(tcp, tcp->u_arg[3]) ||
			upathmatch(tcp, tcp->u_arg[4]);

	case SEN_oldselect:
	case SEN_pselect6:
	case SEN_select:
	{
		int     i, j;
		int     nfds;
		kernel_ulong_t *args;
		kernel_ulong_t select_args[5];
		unsigned int oldselect_args[5];
		unsigned int fdsize;
		fd_set *fds;

		if (SEN_oldselect == s->sen) {
			if (sizeof(*select_args) == sizeof(*oldselect_args)) {
				if (umove(tcp, tcp->u_arg[0], &select_args)) {
					return 0;
				}
			} else {
				unsigned int n;

				if (umove(tcp, tcp->u_arg[0], &oldselect_args)) {
					return 0;
				}

				for (n = 0; n < 5; ++n) {
					select_args[n] = oldselect_args[n];
				}
			}
			args = select_args;
		} else {
			args = tcp->u_arg;
		}

		/* Kernel truncates arg[0] to int, we do the same. */
		nfds = (int) args[0];
		/* Kernel rejects negative nfds, so we don't parse it either. */
		if (nfds <= 0)
			return 0;
		/* Beware of select(2^31-1, NULL, NULL, NULL) and similar... */
		if (nfds > 1024*1024)
			nfds = 1024*1024;
		fdsize = (((nfds + 7) / 8) + current_wordsize-1) & -current_wordsize;
		fds = xmalloc(fdsize);

		for (i = 1; i <= 3; ++i) {
			if (args[i] == 0)
				continue;
			if (umoven(tcp, args[i], fdsize, fds) < 0) {
				continue;
			}
			for (j = 0;; j++) {
				j = next_set_bit(fds, j, nfds);
				if (j < 0)
					break;
				if (fdmatch(tcp, j)) {
					free(fds);
					return 1;
				}
			}
		}
		free(fds);
		return 0;
	}

	case SEN_poll:
	case SEN_ppoll:
	{
		struct pollfd fds;
		unsigned nfds;
		kernel_ulong_t start, cur, end;

		start = tcp->u_arg[0];
		nfds = tcp->u_arg[1];

		end = start + sizeof(fds) * nfds;

		if (nfds == 0 || end < start)
			return 0;

		for (cur = start; cur < end; cur += sizeof(fds))
			if ((umove(tcp, cur, &fds) == 0)
			    && fdmatch(tcp, fds.fd))
				return 1;

		return 0;
	}

	case SEN_bpf:
	case SEN_epoll_create:
	case SEN_epoll_create1:
	case SEN_eventfd2:
	case SEN_eventfd:
	case SEN_fanotify_init:
	case SEN_inotify_init1:
	case SEN_memfd_create:
	case SEN_perf_event_open:
	case SEN_pipe:
	case SEN_pipe2:
	case SEN_printargs:
	case SEN_socket:
	case SEN_socketpair:
	case SEN_timerfd_create:
	case SEN_timerfd_gettime:
	case SEN_timerfd_settime:
	case SEN_userfaultfd:
		/*
		 * These have TRACE_FILE or TRACE_DESCRIPTOR or TRACE_NETWORK set,
		 * but they don't have any file descriptor or path args to test.
		 */
		return 0;
	}

	/*
	 * Our fallback position for calls that haven't already
	 * been handled is to just check arg[0].
	 */

	if (s->sys_flags & TRACE_FILE)
		return upathmatch(tcp, tcp->u_arg[0]);

	if (s->sys_flags & (TRACE_DESC | TRACE_NETWORK))
		return fdmatch(tcp, tcp->u_arg[0]);

	return 0;
}
