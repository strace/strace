/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
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
 *	$Id$
 */

#include "defs.h"

#include <fcntl.h>
#include <sys/file.h>
#ifdef LINUX
#include <inttypes.h>
#endif
#ifdef HAVE_SYS_EPOLL_H
#include <sys/epoll.h>
#endif
#ifdef HAVE_LIBAIO_H
#include <libaio.h>
#endif

#if HAVE_LONG_LONG_OFF_T
/*
 * Hacks for systems that have a long long off_t
 */
#define flock64	flock		/* Horrid hack */
#define printflock printflock64	/* Horrider hack */
#endif


static const struct xlat fcntlcmds[] = {
	{ F_DUPFD,	"F_DUPFD"	},
	{ F_GETFD,	"F_GETFD"	},
	{ F_SETFD,	"F_SETFD"	},
	{ F_GETFL,	"F_GETFL"	},
	{ F_SETFL,	"F_SETFL"	},
	{ F_GETLK,	"F_GETLK"	},
	{ F_SETLK,	"F_SETLK"	},
	{ F_SETLKW,	"F_SETLKW"	},
	{ F_GETOWN,	"F_GETOWN"	},
	{ F_SETOWN,	"F_SETOWN"	},
#ifdef F_RSETLK
	{ F_RSETLK,	"F_RSETLK"	},
#endif
#ifdef F_RSETLKW
	{ F_RSETLKW,	"F_RSETLKW"	},
#endif
#ifdef F_RGETLK
	{ F_RGETLK,	"F_RGETLK"	},
#endif
#ifdef F_CNVT
	{ F_CNVT,	"F_CNVT"	},
#endif
#ifdef F_SETSIG
	{ F_SETSIG,	"F_SETSIG"	},
#endif
#ifdef F_GETSIG
	{ F_GETSIG,	"F_GETSIG"	},
#endif
#ifdef F_CHKFL
	{ F_CHKFL,	"F_CHKFL"	},
#endif
#ifdef F_DUP2FD
	{ F_DUP2FD,	"F_DUP2FD"	},
#endif
#ifdef F_ALLOCSP
	{ F_ALLOCSP,	"F_ALLOCSP"	},
#endif
#ifdef F_ISSTREAM
	{ F_ISSTREAM,	"F_ISSTREAM"	},
#endif
#ifdef F_PRIV
	{ F_PRIV,	"F_PRIV"	},
#endif
#ifdef F_NPRIV
	{ F_NPRIV,	"F_NPRIV"	},
#endif
#ifdef F_QUOTACL
	{ F_QUOTACL,	"F_QUOTACL"	},
#endif
#ifdef F_BLOCKS
	{ F_BLOCKS,	"F_BLOCKS"	},
#endif
#ifdef F_BLKSIZE
	{ F_BLKSIZE,	"F_BLKSIZE"	},
#endif
#ifdef F_GETOWN
	{ F_GETOWN,	"F_GETOWN"	},
#endif
#ifdef F_SETOWN
	{ F_SETOWN,	"F_SETOWN"	},
#endif
#ifdef F_REVOKE
	{ F_REVOKE,	"F_REVOKE"	},
#endif
#ifdef F_SETLK
	{ F_SETLK,	"F_SETLK"	},
#endif
#ifdef F_SETLKW
	{ F_SETLKW,	"F_SETLKW"	},
#endif
#ifdef F_FREESP
	{ F_FREESP,	"F_FREESP"	},
#endif
#ifdef F_GETLK
	{ F_GETLK,	"F_GETLK"	},
#endif
#ifdef F_SETLK64
	{ F_SETLK64,	"F_SETLK64"	},
#endif
#ifdef F_SETLKW64
	{ F_SETLKW64,	"F_SETLKW64"	},
#endif
#ifdef F_FREESP64
	{ F_FREESP64,	"F_FREESP64"	},
#endif
#ifdef F_GETLK64
	{ F_GETLK64,	"F_GETLK64"	},
#endif
#ifdef F_SHARE
	{ F_SHARE,	"F_SHARE"	},
#endif
#ifdef F_UNSHARE
	{ F_UNSHARE,	"F_UNSHARE"	},
#endif
	{ 0,		NULL		},
};

static const struct xlat fdflags[] = {
#ifdef FD_CLOEXEC
	{ FD_CLOEXEC,	"FD_CLOEXEC"	},
#endif
	{ 0,		NULL		},
};

#ifdef LOCK_SH

static const struct xlat flockcmds[] = {
	{ LOCK_SH,	"LOCK_SH"	},
	{ LOCK_EX,	"LOCK_EX"	},
	{ LOCK_NB,	"LOCK_NB"	},
	{ LOCK_UN,	"LOCK_UN"	},
	{ 0,		NULL		},
};

#endif /* LOCK_SH */

static const struct xlat lockfcmds[] = {
	{ F_RDLCK,	"F_RDLCK"	},
	{ F_WRLCK,	"F_WRLCK"	},
	{ F_UNLCK,	"F_UNLCK"	},
#ifdef F_EXLCK
	{ F_EXLCK,	"F_EXLCK"	},
#endif
#ifdef F_SHLCK
	{ F_SHLCK,	"F_SHLCK"	},
#endif
	{ 0,		NULL		},
};

static const struct xlat whence[] = {
	{ SEEK_SET,	"SEEK_SET"	},
	{ SEEK_CUR,	"SEEK_CUR"	},
	{ SEEK_END,	"SEEK_END"	},
	{ 0,		NULL		},
};

#ifndef HAVE_LONG_LONG_OFF_T
/* fcntl/lockf */
static void
printflock(tcp, addr, getlk)
struct tcb *tcp;
long addr;
int getlk;
{
	struct flock fl;

	if (umove(tcp, addr, &fl) < 0) {
		tprintf("{...}");
		return;
	}
	tprintf("{type=");
	printxval(lockfcmds, fl.l_type, "F_???");
	tprintf(", whence=");
	printxval(whence, fl.l_whence, "SEEK_???");
	tprintf(", start=%ld, len=%ld", fl.l_start, fl.l_len);
	if (getlk)
		tprintf(", pid=%lu}", (unsigned long) fl.l_pid);
	else
		tprintf("}");
}
#endif

#if _LFS64_LARGEFILE || HAVE_LONG_LONG_OFF_T
/* fcntl/lockf */
static void
printflock64(tcp, addr, getlk)
struct tcb *tcp;
long addr;
int getlk;
{
	struct flock64 fl;

	if (umove(tcp, addr, &fl) < 0) {
		tprintf("{...}");
		return;
	}
	tprintf("{type=");
	printxval(lockfcmds, fl.l_type, "F_???");
	tprintf(", whence=");
	printxval(whence, fl.l_whence, "SEEK_???");
	tprintf(", start=%lld, len=%lld", (long long) fl.l_start, (long long) fl.l_len);
	if (getlk)
		tprintf(", pid=%lu}", (unsigned long) fl.l_pid);
	else
		tprintf("}");
}
#endif

static const char *
sprintflags(const struct xlat *xlat, int flags)
{
	static char outstr[1024];
	const char *sep;

	strcpy(outstr, "flags ");
	sep = "";
	for (; xlat->str; xlat++) {
		if ((flags & xlat->val) == xlat->val) {
			sprintf(outstr + strlen(outstr),
				"%s%s", sep, xlat->str);
			sep = "|";
			flags &= ~xlat->val;
		}
	}
	if (flags)
		sprintf(outstr + strlen(outstr),
			"%s%#x", sep, flags);
	return outstr;
}

/*
 * low bits of the open(2) flags define access mode,
 * other bits are real flags.
 */
static const char *
sprint_open_modes(mode_t flags)
{
	extern const struct xlat open_access_modes[];
	extern const struct xlat open_mode_flags[];
	static char outstr[1024];
	const char *str = xlookup(open_access_modes, flags & 3);
	const char *sep = "";
	const struct xlat *x;

	strcpy(outstr, "flags ");
	if (str)
	{
		strcat(outstr, str);
		flags &= ~3;
		if (!flags)
			return outstr;
		strcat(outstr, "|");
	}

	for (x = open_mode_flags; x->str; x++)
	{
		if ((flags & x->val) == x->val)
		{
			sprintf(outstr + strlen(outstr),
				"%s%s", sep, x->str);
			sep = "|";
			flags &= ~x->val;
		}
	}
	if (flags)
		sprintf(outstr + strlen(outstr), "%s%#x", sep, flags);
	return outstr;
}

int
sys_fcntl(struct tcb *tcp)
{
	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
		printxval(fcntlcmds, tcp->u_arg[1], "F_???");
		switch (tcp->u_arg[1]) {
		case F_SETFD:
			tprintf(", ");
			printflags(fdflags, tcp->u_arg[2], "FD_???");
			break;
		case F_SETOWN: case F_DUPFD:
			tprintf(", %ld", tcp->u_arg[2]);
			break;
		case F_SETFL:
			tprintf(", ");
			tprint_open_modes(tcp, tcp->u_arg[2]);
			break;
		case F_SETLK: case F_SETLKW:
#ifdef F_FREESP
		case F_FREESP:
#endif
			tprintf(", ");
			printflock(tcp, tcp->u_arg[2], 0);
			break;
#if _LFS64_LARGEFILE
#ifdef F_FREESP64
		case F_FREESP64:
#endif
		/* Linux glibc defines SETLK64 as SETLK,
		   even though the kernel has different values - as does Solaris. */
#if defined(F_SETLK64) && F_SETLK64+0!=F_SETLK
		case F_SETLK64:
#endif
#if defined(F_SETLKW64) && F_SETLKW64+0!=F_SETLKW
		case F_SETLKW64:
#endif
			tprintf(", ");
			printflock64(tcp, tcp->u_arg[2], 0);
			break;
#endif
 		}
	}
	else {
		switch (tcp->u_arg[1]) {
		case F_DUPFD:
		case F_SETFD: case F_SETFL:
		case F_SETLK: case F_SETLKW:
		case F_SETOWN: case F_GETOWN:
			break;
		case F_GETFD:
			if (tcp->u_rval == 0)
				return 0;
			tcp->auxstr = sprintflags(fdflags, tcp->u_rval);
			return RVAL_HEX|RVAL_STR;
		case F_GETFL:
			tcp->auxstr = sprint_open_modes(tcp->u_rval);
			return RVAL_HEX|RVAL_STR;
		case F_GETLK:
			tprintf(", ");
			printflock(tcp, tcp->u_arg[2], 1);
			break;
#if _LFS64_LARGEFILE
#if defined(F_GETLK64) && F_GETLK64+0!=F_GETLK
		case F_GETLK64:
#endif
			tprintf(", ");
			printflock64(tcp, tcp->u_arg[2], 1);
			break;
#endif
 		default:
			tprintf(", %#lx", tcp->u_arg[2]);
			break;
		}
	}
	return 0;
}

#ifdef LOCK_SH

int
sys_flock(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
		printflags(flockcmds, tcp->u_arg[1], "LOCK_???");
	}
	return 0;
}
#endif /* LOCK_SH */

int
sys_close(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld", tcp->u_arg[0]);
	}
	return 0;
}

int
sys_dup(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld", tcp->u_arg[0]);
	}
	return 0;
}

int
sys_dup2(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, %ld", tcp->u_arg[0], tcp->u_arg[1]);
	}
	return 0;
}

#if defined(ALPHA) || defined(FREEBSD) || defined(SUNOS4)
int
sys_getdtablesize(tcp)
struct tcb *tcp;
{
	return 0;
}
#endif /* ALPHA || FREEBSD || SUNOS4 */

static int
decode_select(struct tcb *tcp, long *args, enum bitness_t bitness)
{
	int i, j, nfds;
	unsigned int fdsize = ((((args[0] + 7) / 8) + sizeof(long) - 1)
			       & -sizeof(long));
	fd_set *fds;
	static char outstr[1024];
	char *sep;
	long arg;

	if (entering(tcp)) {
		fds = (fd_set *) malloc(fdsize);
		if (fds == NULL)
			fprintf(stderr, "out of memory\n");
		nfds = args[0];
		tprintf("%d", nfds);
		for (i = 0; i < 3; i++) {
			arg = args[i+1];
			if (arg == 0) {
				tprintf(", NULL");
				continue;
			}
			if (fds == NULL || !verbose(tcp)) {
				tprintf(", %#lx", arg);
				continue;
			}
			if (umoven(tcp, arg, fdsize, (char *) fds) < 0) {
				tprintf(", [?]");
				continue;
			}
			tprintf(", [");
			for (j = 0, sep = ""; j < nfds; j++) {
				if (FD_ISSET(j, fds)) {
					tprintf("%s%u", sep, j);
					sep = " ";
				}
			}
			tprintf("]");
		}
		free(fds);
		tprintf(", ");
		printtv_bitness(tcp, args[4], bitness);
	}
	else
	{
		unsigned int cumlen = 0;
		char *sep = "";

		if (syserror(tcp))
			return 0;

		if ((nfds = tcp->u_rval) == 0) {
			tcp->auxstr = "Timeout";
			return RVAL_STR;
		}

		fds = (fd_set *) malloc(fdsize);
		if (fds == NULL)
			fprintf(stderr, "out of memory\n");

		outstr[0] = '\0';
		for (i = 0; i < 3; i++) {
			int first = 1;
			char str[20];

			tcp->auxstr = outstr;
			arg = args[i+1];
			if (fds == NULL || !arg ||
			    umoven(tcp, arg, fdsize, (char *) fds) < 0)
				continue;
			for (j = 0; j < args[0]; j++) {
				if (FD_ISSET(j, fds)) {
					if (first) {
						sprintf(str, "%s%s [%u", sep,
							i == 0 ? "in" :
							i == 1 ? "out" :
							"except", j);
						first = 0;
						sep = ", ";
					}
					else
						sprintf(str, " %u", j);
					cumlen += strlen(str);
					if (cumlen < sizeof(outstr))
						strcat(outstr, str);
					nfds--;
				}
			}
			if (cumlen)
				strcat(outstr, "]");
			if (nfds == 0)
				break;
		}
		free(fds);
#ifdef LINUX
		/* This contains no useful information on SunOS.  */
		if (args[4]) {
			char str[128];

			sprintf(str, "%sleft ", sep);
			sprinttv(tcp, args[4], bitness, str + strlen(str));
			if ((cumlen += strlen(str)) < sizeof(outstr))
				strcat(outstr, str);
		}
#endif /* LINUX */
		return RVAL_STR;
	}
	return 0;
}

#ifdef LINUX

int
sys_oldselect(tcp)
struct tcb *tcp;
{
	long args[5];

	if (umoven(tcp, tcp->u_arg[0], sizeof args, (char *) args) < 0) {
		tprintf("[...]");
		return 0;
	}
	return decode_select(tcp, args, BITNESS_CURRENT);
}

#ifdef ALPHA
int
sys_osf_select(tcp)
struct tcb *tcp;
{
	long *args = tcp->u_arg;
	return decode_select(tcp, args, BITNESS_32);
}
#endif

static struct xlat epollctls[] = {
#ifdef EPOLL_CTL_ADD
	{ EPOLL_CTL_ADD,	"EPOLL_CTL_ADD"	},
#endif
#ifdef EPOLL_CTL_MOD
	{ EPOLL_CTL_MOD,	"EPOLL_CTL_MOD"	},
#endif
#ifdef EPOLL_CTL_DEL
	{ EPOLL_CTL_DEL,	"EPOLL_CTL_DEL"	},
#endif
	{ 0,			NULL		}
};

static struct xlat epollevents[] = {
#ifdef EPOLLIN
	{ EPOLLIN,	"EPOLLIN"	},
#endif
#ifdef EPOLLPRI
	{ EPOLLPRI,	"EPOLLPRI"	},
#endif
#ifdef EPOLLOUT
	{ EPOLLOUT,	"EPOLLOUT"	},
#endif
#ifdef EPOLLRDNORM
	{ EPOLLRDNORM,	"EPOLLRDNORM"	},
#endif
#ifdef EPOLLRDBAND
	{ EPOLLRDBAND,	"EPOLLRDBAND"	},
#endif
#ifdef EPOLLWRNORM
	{ EPOLLWRNORM,	"EPOLLWRNORM"	},
#endif
#ifdef EPOLLWRBAND
	{ EPOLLWRBAND,	"EPOLLWRBAND"	},
#endif
#ifdef EPOLLMSG
	{ EPOLLMSG,	"EPOLLMSG"	},
#endif
#ifdef EPOLLERR
	{ EPOLLERR,	"EPOLLERR"	},
#endif
#ifdef EPOLLHUP
	{ EPOLLHUP,	"EPOLLHUP"	},
#endif
#ifdef EPOLLONESHOT
	{ EPOLLONESHOT,	"EPOLLONESHOT"	},
#endif
#ifdef EPOLLET
	{ EPOLLET,	"EPOLLET"	},
#endif
	{ 0,		NULL		}
};

int
sys_epoll_create(tcp)
struct tcb *tcp;
{
	if (entering(tcp))
		tprintf("%ld", tcp->u_arg[0]);
	return 0;
}

#ifdef HAVE_SYS_EPOLL_H
static void
print_epoll_event(ev)
struct epoll_event *ev;
{
	tprintf("{");
	printflags(epollevents, ev->events, "EPOLL???");
	/* We cannot know what format the program uses, so print u32 and u64
	   which will cover every value.  */
	tprintf(", {u32=%" PRIu32 ", u64=%" PRIu64 "}}",
		ev->data.u32, ev->data.u64);
}
#endif

int
sys_epoll_ctl(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
                printxval(epollctls, tcp->u_arg[1], "EPOLL_CTL_???");
		tprintf(", %ld, ", tcp->u_arg[2]);
		if (tcp->u_arg[3] == 0)
			tprintf("NULL");
		else {
#ifdef HAVE_SYS_EPOLL_H
			struct epoll_event ev;
			if (umove(tcp, tcp->u_arg[3], &ev) == 0)
				print_epoll_event(&ev);
			else
#endif
				tprintf("{...}");
		}
	}
	return 0;
}

int
sys_epoll_wait(tcp)
struct tcb *tcp;
{
	if (entering(tcp))
		tprintf("%ld, ", tcp->u_arg[0]);
	else {
		if (syserror(tcp))
			tprintf("%lx", tcp->u_arg[1]);
		else if (tcp->u_rval == 0)
			tprintf("{}");
		else {
#ifdef HAVE_SYS_EPOLL_H
			struct epoll_event ev, *start, *cur, *end;
			int failed = 0;

			tprintf("{");
			start = (struct epoll_event *) tcp->u_arg[1];
			end = start + tcp->u_rval;
			for (cur = start; cur < end; ++cur) {
				if (cur > start)
					tprintf(", ");
				if (umove(tcp, (long) cur, &ev) == 0)
					print_epoll_event(&ev);
				else {
					tprintf("?");
					failed = 1;
					break;
				}
			}
			tprintf("}");
			if (failed)
				tprintf(" %#lx", (long) start);
#else
			tprintf("{...}");
#endif
		}
		tprintf(", %ld, %ld", tcp->u_arg[2], tcp->u_arg[3]);
	}
	return 0;
}

int
sys_io_setup(tcp)
struct tcb *tcp;
{
	if (entering(tcp))
		tprintf("%ld, ", tcp->u_arg[0]);
	else {
		if (syserror(tcp))
			tprintf("0x%0lx", tcp->u_arg[1]);
		else {
			unsigned long user_id;
			if (umove(tcp, tcp->u_arg[1], &user_id) == 0)
				tprintf("{%lu}", user_id);
			else
				tprintf("{...}");
		}
	}
	return 0;
}

int
sys_io_destroy(tcp)
struct tcb *tcp;
{
	if (entering(tcp))
		tprintf("%lu", tcp->u_arg[0]);
	return 0;
}

int
sys_io_submit(tcp)
struct tcb *tcp;
{
	long nr;
	if (entering(tcp)) {
		tprintf("%lu, %ld, ", tcp->u_arg[0], tcp->u_arg[1]);
		nr = tcp->u_arg[1];
		/* and if nr is negative? */
		if (nr == 0)
			tprintf("{}");
		else {
#ifdef HAVE_LIBAIO_H
			long i;
			struct iocb *iocbp, **iocbs = (void *)tcp->u_arg[2];

			for (i = 0; i < nr; i++, iocbs++) {
				struct iocb iocb;
				if (i == 0)
					tprintf("{");
				else
					tprintf(", ");

				if (umove(tcp, (unsigned long)iocbs, &iocbp) ||
				    umove(tcp, (unsigned long)iocbp, &iocb)) {
					tprintf("{...}");
					continue;
				}
				tprintf("{%p, %u, %hu, %hu, %d}",
					iocb.data, iocb.key,
					iocb.aio_lio_opcode,
					iocb.aio_reqprio, iocb.aio_fildes);
			}
			if (i)
				tprintf("}");
#else
			tprintf("{...}");
#endif
		}
	}
	return 0;
}

int
sys_io_cancel(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
#ifdef HAVE_LIBAIO_H
		struct iocb iocb;
#endif
		tprintf("%lu, ", tcp->u_arg[0]);
#ifdef HAVE_LIBAIO_H
		if (umove(tcp, tcp->u_arg[1], &iocb) == 0) {
			tprintf("{%p, %u, %hu, %hu, %d}, ",
				iocb.data, iocb.key,
				iocb.aio_lio_opcode,
				iocb.aio_reqprio, iocb.aio_fildes);
		} else
#endif
			tprintf("{...}, ");
	} else {
		if (tcp->u_rval < 0)
			tprintf("{...}");
		else {
#ifdef HAVE_LIBAIO_H
			struct io_event event;
			if (umove(tcp, tcp->u_arg[2], &event) == 0)
				tprintf("{%p, %p, %ld, %ld}",
					event.data, event.obj,
					event.res, event.res2);
			else 
#endif
				tprintf("{...}");
		}
	}
	return 0;
}

int
sys_io_getevents(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, %ld, %ld, ", tcp->u_arg[0], tcp->u_arg[1],
			tcp->u_arg[2]);
	} else {
		if (tcp->u_rval == 0) {
			tprintf("{}");
		} else  {
#ifdef HAVE_LIBAIO_H
			struct io_event *events = (void *)tcp->u_arg[3];
			long i, nr = tcp->u_rval;

			for (i = 0; i < nr; i++, events++) {
				struct io_event event;

				if (i == 0)
					tprintf("{");
				else
					tprintf(", ");

				if (umove(tcp, (unsigned long)events, &event) != 0) {
					tprintf("{...}");
					continue;
				}
				tprintf("{%p, %p, %ld, %ld}", event.data,
					event.obj, event.res, event.res2);
			}
			tprintf("}, ");
#else
				tprintf("{...}");
#endif
		}

		if (tcp->u_arg[4] == 0)
			tprintf("NULL");
		else {
			struct timespec to;
			if (umove(tcp, tcp->u_arg[4], &to) == 0)
				tprintf("{%lu, %lu}", to.tv_sec, to.tv_nsec);
			else
				tprintf("{...}");
		}
	}
	return 0;
}
#endif /* LINUX */

int
sys_select(tcp)
struct tcb *tcp;
{
	return decode_select(tcp, tcp->u_arg, BITNESS_CURRENT);
}

#ifdef LINUX
int
sys_pselect6(struct tcb *tcp)
{
	int rc = decode_select(tcp, tcp->u_arg, BITNESS_CURRENT);
	if (exiting(tcp)) {
		struct {
			void *ss;
			unsigned long len;
		} data;
		if (umove(tcp, tcp->u_arg[5], &data) < 0)
			tprintf(", %#lx", tcp->u_arg[5]);
		else {
			tprintf(", {");
			if (data.len < sizeof(sigset_t))
				tprintf("%#lx", (long)data.ss);
			else
				print_sigset(tcp, (long)data.ss, 0);
			tprintf(", %lu}", data.len);
		}
	}
	return rc;
}
#endif
