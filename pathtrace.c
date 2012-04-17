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
#ifdef HAVE_POLL_H
# include <poll.h>
#endif
#ifdef HAVE_SYS_POLL_H
# include <sys/poll.h>
#endif

#include "syscall.h"

#define MAXSELECTED  256	/* max number of "selected" paths */
static const char *selected[MAXSELECTED];	/* paths selected for tracing */

/*
 * Return true if specified path matches one that we're tracing.
 */
static int
pathmatch(const char *path)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(selected); ++i) {
		if (selected[i] == NULL)
			return 0;
		if (strcmp(path, selected[i]) == 0)
			return 1;
	}
	return 0;
}

/*
 * Return true if specified path (in user-space) matches.
 */
static int
upathmatch(struct tcb *tcp, unsigned long upath)
{
	char path[PATH_MAX + 1];

	return umovestr(tcp, upath, sizeof path, path) >= 0 &&
		pathmatch(path);
}

/*
 * Return true if specified fd maps to a path we're tracing.
 */
static int
fdmatch(struct tcb *tcp, int fd)
{
	const char *path = getfdpath(tcp, fd);

	return path && pathmatch(path);
}

/*
 * Add a path to the set we're tracing.
 * Secifying NULL will delete all paths.
 */
static int
storepath(const char *path)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(selected); ++i) {
		if (!selected[i]) {
			selected[i] = path;
			return 0;
		}
	}

	fprintf(stderr, "Max trace paths exceeded, only using first %u\n",
		(unsigned int) ARRAY_SIZE(selected));
	return -1;
}

/*
 * Get path associated with fd.
 */
const char *
getfdpath(struct tcb *tcp, int fd)
{
	static char path[PATH_MAX+1];
	char linkpath[sizeof("/proc/%u/fd/%u") + 2 * sizeof(int)*3];
	ssize_t n;

	if (fd < 0)
		return NULL;

	sprintf(linkpath, "/proc/%u/fd/%u", tcp->pid, fd);
	n = readlink(linkpath, path, (sizeof path) - 1);
	if (n <= 0)
		return NULL;
	path[n] = '\0';
	return path;
}

/*
 * Add a path to the set we're tracing.  Also add the canonicalized
 * version of the path.  Secifying NULL will delete all paths.
 */
int
pathtrace_select(const char *path)
{
	char *rpath;

	if (storepath(path))
		return -1;

	rpath = realpath(path, NULL);

	if (rpath == NULL)
		return 0;

	/* if realpath and specified path are same, we're done */
	if (strcmp(path, rpath) == 0) {
		free(rpath);
		return 0;
	}

	fprintf(stderr, "Requested path '%s' resolved into '%s'\n",
		path, rpath);
	return storepath(rpath);
}

/*
 * Return true if syscall accesses a selected path
 * (or if no paths have been specified for tracing).
 */
int
pathtrace_match(struct tcb *tcp)
{
	const struct sysent *s;

	if (selected[0] == NULL)
		return 1;

	if (!SCNO_IN_RANGE(tcp->scno))
		return 0;

	s = &sysent[tcp->scno];

	if (!(s->sys_flags & (TRACE_FILE | TRACE_DESC)))
		return 0;

	/*
	 * Check for special cases where we need to do something
	 * other than test arg[0].
	 */

	if (s->sys_func == sys_dup2 ||
	    s->sys_func == sys_dup3 ||
	    s->sys_func == sys_sendfile ||
	    s->sys_func == sys_sendfile64 ||
	    s->sys_func == sys_tee)
	{
		/* fd, fd */
		return fdmatch(tcp, tcp->u_arg[0]) ||
			fdmatch(tcp, tcp->u_arg[1]);
	}

	if (s->sys_func == sys_inotify_add_watch ||
	    s->sys_func == sys_faccessat ||
	    s->sys_func == sys_fchmodat ||
	    s->sys_func == sys_futimesat ||
	    s->sys_func == sys_mkdirat ||
	    s->sys_func == sys_unlinkat ||
	    s->sys_func == sys_newfstatat ||
	    s->sys_func == sys_mknodat ||
	    s->sys_func == sys_openat ||
	    s->sys_func == sys_readlinkat ||
	    s->sys_func == sys_utimensat ||
	    s->sys_func == sys_fchownat ||
	    s->sys_func == sys_pipe2)
	{
		/* fd, path */
		return fdmatch(tcp, tcp->u_arg[0]) ||
			upathmatch(tcp, tcp->u_arg[1]);
	}

	if (s->sys_func == sys_link ||
	    s->sys_func == sys_mount)
	{
		/* path, path */
		return upathmatch(tcp, tcp->u_arg[0]) ||
			upathmatch(tcp, tcp->u_arg[1]);
	}

	if (s->sys_func == sys_renameat ||
	    s->sys_func == sys_linkat)
	{
		/* fd, path, fd, path */
		return fdmatch(tcp, tcp->u_arg[0]) ||
			fdmatch(tcp, tcp->u_arg[2]) ||
			upathmatch(tcp, tcp->u_arg[1]) ||
			upathmatch(tcp, tcp->u_arg[3]);
	}

	if (
	    s->sys_func == sys_old_mmap ||
	    s->sys_func == sys_mmap) {
		/* x, x, x, x, fd */
		return fdmatch(tcp, tcp->u_arg[4]);
	}

	if (s->sys_func == sys_symlinkat) {
		/* path, fd, path */
		return fdmatch(tcp, tcp->u_arg[1]) ||
			upathmatch(tcp, tcp->u_arg[0]) ||
			upathmatch(tcp, tcp->u_arg[2]);
	}

	if (s->sys_func == sys_splice) {
		/* fd, x, fd, x, x */
		return fdmatch(tcp, tcp->u_arg[0]) ||
			fdmatch(tcp, tcp->u_arg[2]);
	}

	if (s->sys_func == sys_epoll_ctl) {
		/* x, x, fd, x */
		return fdmatch(tcp, tcp->u_arg[2]);
	}

	if (s->sys_func == sys_select ||
	    s->sys_func == sys_oldselect ||
	    s->sys_func == sys_pselect6)
	{
		int     i, j;
		unsigned nfds;
		long   *args, oldargs[5];
		unsigned fdsize;
		fd_set *fds;

		if (s->sys_func == sys_oldselect) {
			if (umoven(tcp, tcp->u_arg[0], sizeof oldargs,
				   (char*) oldargs) < 0)
			{
				fprintf(stderr, "umoven() failed\n");
				return 0;
			}
			args = oldargs;
		} else
			args = tcp->u_arg;

		nfds = args[0];
		/* Beware of select(2^31-1, NULL, NULL, NULL) and similar... */
		if (args[0] > 1024*1024)
			nfds = 1024*1024;
		if (args[0] < 0)
			nfds = 0;
		fdsize = ((((nfds + 7) / 8) + sizeof(long) - 1)
			  & -sizeof(long));
		fds = malloc(fdsize);
		if (!fds)
			die_out_of_memory();

		for (i = 1; i <= 3; ++i) {
			if (args[i] == 0)
				continue;

			if (umoven(tcp, args[i], fdsize, (char *) fds) < 0) {
				fprintf(stderr, "umoven() failed\n");
				continue;
			}

			for (j = 0; j < nfds; ++j)
				if (FD_ISSET(j, fds) && fdmatch(tcp, j)) {
					free(fds);
					return 1;
				}
		}
		free(fds);
		return 0;
	}

	if (s->sys_func == sys_poll ||
	    s->sys_func == sys_ppoll)
	{
		struct pollfd fds;
		unsigned nfds;
		unsigned long start, cur, end;

		start = tcp->u_arg[0];
		nfds = tcp->u_arg[1];

		end = start + sizeof(fds) * nfds;

		if (nfds == 0 || end < start)
			return 0;

		for (cur = start; cur < end; cur += sizeof(fds))
			if ((umoven(tcp, cur, sizeof fds, (char *) &fds) == 0)
			    && fdmatch(tcp, fds.fd))
				return 1;

		return 0;
	}

	if (s->sys_func == printargs ||
	    s->sys_func == sys_pipe ||
	    s->sys_func == sys_pipe2 ||
	    s->sys_func == sys_eventfd2 ||
	    s->sys_func == sys_eventfd ||
	    s->sys_func == sys_inotify_init1 ||
	    s->sys_func == sys_timerfd_create ||
	    s->sys_func == sys_timerfd_settime ||
	    s->sys_func == sys_timerfd_gettime ||
	    s->sys_func == sys_epoll_create ||
	    strcmp(s->sys_name, "fanotify_init") == 0)
	{
		/*
		 * These have TRACE_FILE or TRACE_DESCRIPTOR set, but they
		 * don't have any file descriptor or path args to test.
		 */
		return 0;
	}

	/*
	 * Our fallback position for calls that haven't already
	 * been handled is to just check arg[0].
	 */

	if (s->sys_flags & TRACE_FILE)
		return upathmatch(tcp, tcp->u_arg[0]);

	if (s->sys_flags & TRACE_DESC)
		return fdmatch(tcp, tcp->u_arg[0]);

	return 0;
}
