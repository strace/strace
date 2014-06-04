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
 */

#include "defs.h"
#include <fcntl.h>
#include <sys/file.h>
#ifdef HAVE_SYS_EPOLL_H
# include <sys/epoll.h>
#endif
#ifdef HAVE_LINUX_PERF_EVENT_H
# include  <linux/perf_event.h>
#endif

#include "xlat/fcntlcmds.h"
#include "xlat/fdflags.h"
#include "xlat/flockcmds.h"
#include "xlat/lockfcmds.h"
#include "xlat/notifyflags.h"
#include "xlat/perf_event_open_flags.h"

/*
 * Assume that F_SETLK64, F_SETLKW64, and F_GETLK64 are either defined
 * or not defined altogether.
 */
#if defined(F_SETLK64) && F_SETLK64 + 0 != F_SETLK
# define USE_PRINTFLOCK64 1
#else
# define USE_PRINTFLOCK64 0
#endif

#if USE_PRINTFLOCK64

# ifndef HAVE_STRUCT_FLOCK64
struct flock64 {
	short int l_type, l_whence;
	int64_t l_start, l_len;
	int l_pid;
};
# endif

static void
printflock64(struct tcb *tcp, long addr, int getlk)
{
	struct flock64 fl;

	if (umove(tcp, addr, &fl) < 0) {
		tprints("{...}");
		return;
	}
	tprints("{type=");
	printxval(lockfcmds, fl.l_type, "F_???");
	tprints(", whence=");
	printxval(whence_codes, fl.l_whence, "SEEK_???");
	tprintf(", start=%lld, len=%lld", (long long) fl.l_start, (long long) fl.l_len);
	if (getlk)
		tprintf(", pid=%lu}", (unsigned long) fl.l_pid);
	else
		tprints("}");
}
#endif /* USE_PRINTFLOCK64 */

static void
printflock(struct tcb *tcp, long addr, int getlk)
{
	struct flock fl;
	int r;

#if SUPPORTED_PERSONALITIES > 1
	if (
# if SIZEOF_OFF_T > SIZEOF_LONG
	    current_personality > 0 &&
#endif
	    current_wordsize != sizeof(fl.l_start)) {
		if (current_wordsize == 4) {
			/* 32-bit x86 app on x86_64 and similar cases */
			struct {
				short int l_type;
				short int l_whence;
				int32_t l_start; /* off_t */
				int32_t l_len; /* off_t */
				int32_t l_pid; /* pid_t */
			} fl32;
			r = umove(tcp, addr, &fl32);
			if (r >= 0) {
				fl.l_type = fl32.l_type;
				fl.l_whence = fl32.l_whence;
				fl.l_start = fl32.l_start;
				fl.l_len = fl32.l_len;
				fl.l_pid = fl32.l_pid;
			}
		} else {
			/* let people know we have a problem here */
			tprintf("<decode error: unsupported wordsize %d>",
				current_wordsize);
			return;
		}
	} else
#endif
	{
		r = umove(tcp, addr, &fl);
	}
	if (r < 0) {
		tprints("{...}");
		return;
	}
	tprints("{type=");
	printxval(lockfcmds, fl.l_type, "F_???");
	tprints(", whence=");
	printxval(whence_codes, fl.l_whence, "SEEK_???");
#if SIZEOF_OFF_T > SIZEOF_LONG
	tprintf(", start=%lld, len=%lld", fl.l_start, fl.l_len);
#else
	tprintf(", start=%ld, len=%ld", fl.l_start, fl.l_len);
#endif
	if (getlk)
		tprintf(", pid=%lu}", (unsigned long) fl.l_pid);
	else
		tprints("}");
}

int
sys_fcntl(struct tcb *tcp)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		printxval(fcntlcmds, tcp->u_arg[1], "F_???");
		switch (tcp->u_arg[1]) {
		case F_SETFD:
			tprints(", ");
			printflags(fdflags, tcp->u_arg[2], "FD_???");
			break;
		case F_SETOWN: case F_DUPFD:
#ifdef F_DUPFD_CLOEXEC
		case F_DUPFD_CLOEXEC:
#endif
			tprintf(", %ld", tcp->u_arg[2]);
			break;
		case F_SETFL:
			tprints(", ");
			tprint_open_modes(tcp->u_arg[2]);
			break;
		case F_SETLK: case F_SETLKW:
			tprints(", ");
			printflock(tcp, tcp->u_arg[2], 0);
			break;
#if USE_PRINTFLOCK64
		case F_SETLK64: case F_SETLKW64:
			tprints(", ");
			printflock64(tcp, tcp->u_arg[2], 0);
			break;
#endif /* USE_PRINTFLOCK64 */
#ifdef F_NOTIFY
		case F_NOTIFY:
			tprints(", ");
			printflags(notifyflags, tcp->u_arg[2], "DN_???");
			break;
#endif
#ifdef F_SETLEASE
		case F_SETLEASE:
			tprints(", ");
			printxval(lockfcmds, tcp->u_arg[2], "F_???");
			break;
#endif
		}
	}
	else {
		switch (tcp->u_arg[1]) {
		case F_DUPFD:
#ifdef F_DUPFD_CLOEXEC
		case F_DUPFD_CLOEXEC:
#endif
		case F_SETFD: case F_SETFL:
		case F_SETLK: case F_SETLKW:
		case F_SETOWN: case F_GETOWN:
#ifdef F_NOTIFY
		case F_NOTIFY:
#endif
#ifdef F_SETLEASE
		case F_SETLEASE:
#endif
			break;
		case F_GETFD:
			if (syserror(tcp) || tcp->u_rval == 0)
				return 0;
			tcp->auxstr = sprintflags("flags ", fdflags, tcp->u_rval);
			return RVAL_HEX|RVAL_STR;
		case F_GETFL:
			if (syserror(tcp))
				return 0;
			tcp->auxstr = sprint_open_modes(tcp->u_rval);
			return RVAL_HEX|RVAL_STR;
		case F_GETLK:
			tprints(", ");
			printflock(tcp, tcp->u_arg[2], 1);
			break;
#if USE_PRINTFLOCK64
		case F_GETLK64:
			tprints(", ");
			printflock64(tcp, tcp->u_arg[2], 1);
			break;
#endif
#ifdef F_GETLEASE
		case F_GETLEASE:
			if (syserror(tcp))
				return 0;
			tcp->auxstr = xlookup(lockfcmds, tcp->u_rval);
			return RVAL_HEX|RVAL_STR;
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
sys_flock(struct tcb *tcp)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		printflags(flockcmds, tcp->u_arg[1], "LOCK_???");
	}
	return 0;
}
#endif /* LOCK_SH */

int
sys_close(struct tcb *tcp)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
	}
	return 0;
}

int
sys_dup(struct tcb *tcp)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
	}
	return RVAL_FD;
}

static int
do_dup2(struct tcb *tcp, int flags_arg)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		printfd(tcp, tcp->u_arg[1]);
		if (flags_arg >= 0) {
			tprints(", ");
			printflags(open_mode_flags, tcp->u_arg[flags_arg], "O_???");
		}
	}
	return RVAL_FD;
}

int
sys_dup2(struct tcb *tcp)
{
	return do_dup2(tcp, -1);
}

int
sys_dup3(struct tcb *tcp)
{
	return do_dup2(tcp, 2);
}

#if defined(ALPHA)
int
sys_getdtablesize(struct tcb *tcp)
{
	return 0;
}
#endif

static int
decode_select(struct tcb *tcp, long *args, enum bitness_t bitness)
{
	int i, j;
	int nfds, fdsize;
	fd_set *fds;
	const char *sep;
	long arg;

	/* Kernel truncates arg[0] to int, we do the same. */
	nfds = (int) args[0];

	/* Kernel rejects negative nfds, so we don't parse it either. */
	if (nfds < 0) {
		nfds = 0;
		fds = NULL;
	}
	/* Beware of select(2^31-1, NULL, NULL, NULL) and similar... */
	if (nfds > 1024*1024)
		nfds = 1024*1024;

	/*
	 * We had bugs a-la "while (j < args[0])" and "umoven(args[0])" below.
	 * Instead of args[0], use nfds for fd count, fdsize for array lengths.
	 */
	fdsize = (((nfds + 7) / 8) + current_wordsize-1) & -current_wordsize;

	if (entering(tcp)) {
		tprintf("%d", (int) args[0]);

		if (fdsize > 0) {
			fds = malloc(fdsize);
			if (!fds)
				die_out_of_memory();
		}
		for (i = 0; i < 3; i++) {
			arg = args[i+1];
			if (arg == 0) {
				tprints(", NULL");
				continue;
			}
			if (!verbose(tcp) || !fds) {
				tprintf(", %#lx", arg);
				continue;
			}
			if (umoven(tcp, arg, fdsize, (char *) fds) < 0) {
				tprints(", [?]");
				continue;
			}
			tprints(", [");
			for (j = 0, sep = "";; j++) {
				j = next_set_bit(fds, j, nfds);
				if (j < 0)
					break;
				tprints(sep);
				printfd(tcp, j);
				sep = " ";
			}
			tprints("]");
		}
		free(fds);
		tprints(", ");
		printtv_bitness(tcp, args[4], bitness, 0);
	}
	else {
		static char outstr[1024];
		char *outptr;
#define end_outstr (outstr + sizeof(outstr))
		int ready_fds;

		if (syserror(tcp))
			return 0;

		ready_fds = tcp->u_rval;
		if (ready_fds == 0) {
			tcp->auxstr = "Timeout";
			return RVAL_STR;
		}

		fds = malloc(fdsize);
		if (!fds)
			die_out_of_memory();

		outptr = outstr;
		sep = "";
		for (i = 0; i < 3 && ready_fds > 0; i++) {
			int first = 1;

			arg = args[i+1];
			if (!arg || umoven(tcp, arg, fdsize, (char *) fds) < 0)
				continue;
			for (j = 0;; j++) {
				j = next_set_bit(fds, j, nfds);
				if (j < 0)
					break;
				/* +2 chars needed at the end: ']',NUL */
				if (outptr < end_outstr - (sizeof(", except [") + sizeof(int)*3 + 2)) {
					if (first) {
						outptr += sprintf(outptr, "%s%s [%u",
							sep,
							i == 0 ? "in" : i == 1 ? "out" : "except",
							j
						);
						first = 0;
						sep = ", ";
					}
					else {
						outptr += sprintf(outptr, " %u", j);
					}
				}
				if (--ready_fds == 0)
					break;
			}
			if (outptr != outstr)
				*outptr++ = ']';
		}
		free(fds);
		/* This contains no useful information on SunOS.  */
		if (args[4]) {
			if (outptr < end_outstr - (10 + TIMEVAL_TEXT_BUFSIZE)) {
				outptr += sprintf(outptr, "%sleft ", sep);
				outptr = sprinttv(outptr, tcp, args[4], bitness, /*special:*/ 0);
			}
		}
		*outptr = '\0';
		tcp->auxstr = outstr;
		return RVAL_STR;
#undef end_outstr
	}
	return 0;
}

int
sys_oldselect(struct tcb *tcp)
{
	long args[5];

	if (umoven(tcp, tcp->u_arg[0], sizeof args, (char *) args) < 0) {
		tprints("[...]");
		return 0;
	}
	return decode_select(tcp, args, BITNESS_CURRENT);
}

#ifdef ALPHA
int
sys_osf_select(struct tcb *tcp)
{
	long *args = tcp->u_arg;
	return decode_select(tcp, args, BITNESS_32);
}
#endif

#include "xlat/epollctls.h"
#include "xlat/epollevents.h"
#include "xlat/epollflags.h"

/* Not aliased to printargs_ld: we want it to have a distinct address */
int
sys_epoll_create(struct tcb *tcp)
{
	return printargs_ld(tcp);
}

int
sys_epoll_create1(struct tcb *tcp)
{
	if (entering(tcp))
		printflags(epollflags, tcp->u_arg[0], "EPOLL_???");
	return 0;
}

#ifdef HAVE_SYS_EPOLL_H
static void
print_epoll_event(struct epoll_event *ev)
{
	tprints("{");
	printflags(epollevents, ev->events, "EPOLL???");
	/* We cannot know what format the program uses, so print u32 and u64
	   which will cover every value.  */
	tprintf(", {u32=%" PRIu32 ", u64=%" PRIu64 "}}",
		ev->data.u32, ev->data.u64);
}
#endif

int
sys_epoll_ctl(struct tcb *tcp)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		printxval(epollctls, tcp->u_arg[1], "EPOLL_CTL_???");
		tprints(", ");
		printfd(tcp, tcp->u_arg[2]);
		tprints(", ");
		if (tcp->u_arg[3] == 0)
			tprints("NULL");
		else {
#ifdef HAVE_SYS_EPOLL_H
			struct epoll_event ev;
			if (
#ifdef EPOLL_CTL_DEL
			    (tcp->u_arg[1] != EPOLL_CTL_DEL) &&
#endif
			    umove(tcp, tcp->u_arg[3], &ev) == 0)
				print_epoll_event(&ev);
			else
#endif
				tprintf("%lx", tcp->u_arg[3]);
		}
	}
	return 0;
}

static void
epoll_wait_common(struct tcb *tcp)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		if (syserror(tcp))
			tprintf("%lx", tcp->u_arg[1]);
		else if (tcp->u_rval == 0)
			tprints("{}");
		else {
#ifdef HAVE_SYS_EPOLL_H
			struct epoll_event ev, *start, *cur, *end;
			int failed = 0;

			tprints("{");
			start = (struct epoll_event *) tcp->u_arg[1];
			end = start + tcp->u_rval;
			for (cur = start; cur < end; ++cur) {
				if (cur > start)
					tprints(", ");
				if (umove(tcp, (long) cur, &ev) == 0)
					print_epoll_event(&ev);
				else {
					tprints("?");
					failed = 1;
					break;
				}
			}
			tprints("}");
			if (failed)
				tprintf(" %#lx", (long) start);
#else
			tprints("{...}");
#endif
		}
		tprintf(", %d, %d", (int) tcp->u_arg[2], (int) tcp->u_arg[3]);
	}
}

int
sys_epoll_wait(struct tcb *tcp)
{
	epoll_wait_common(tcp);
	return 0;
}

int
sys_epoll_pwait(struct tcb *tcp)
{
	epoll_wait_common(tcp);
	if (exiting(tcp)) {
		tprints(", ");
		/* NB: kernel requires arg[5] == NSIG / 8 */
		print_sigset_addr_len(tcp, tcp->u_arg[4], tcp->u_arg[5]);
		tprintf(", %lu", tcp->u_arg[5]);
	}
	return 0;
}

int
sys_select(struct tcb *tcp)
{
	return decode_select(tcp, tcp->u_arg, BITNESS_CURRENT);
}

int
sys_pselect6(struct tcb *tcp)
{
	int rc = decode_select(tcp, tcp->u_arg, BITNESS_CURRENT);
	if (entering(tcp)) {
		long r;
		struct {
			unsigned long ptr;
			unsigned long len;
		} data;
#if SUPPORTED_PERSONALITIES > 1 && SIZEOF_LONG > 4
		if (current_wordsize == 4) {
			struct {
				uint32_t ptr;
				uint32_t len;
			} data32;
			r = umove(tcp, tcp->u_arg[5], &data32);
			data.ptr = data32.ptr;
			data.len = data32.len;
		} else
#endif
			r = umove(tcp, tcp->u_arg[5], &data);
		if (r < 0)
			tprintf(", %#lx", tcp->u_arg[5]);
		else {
			tprints(", {");
			/* NB: kernel requires data.len == NSIG / 8 */
			print_sigset_addr_len(tcp, data.ptr, data.len);
			tprintf(", %lu}", data.len);
		}
	}
	return rc;
}

static int
do_eventfd(struct tcb *tcp, int flags_arg)
{
	if (entering(tcp)) {
		tprintf("%lu", tcp->u_arg[0]);
		if (flags_arg >= 0) {
			tprints(", ");
			printflags(open_mode_flags, tcp->u_arg[flags_arg], "O_???");
		}
	}
	return 0;
}

int
sys_eventfd(struct tcb *tcp)
{
	return do_eventfd(tcp, -1);
}

int
sys_eventfd2(struct tcb *tcp)
{
	return do_eventfd(tcp, 1);
}

int
sys_perf_event_open(struct tcb *tcp)
{
	if (entering(tcp)) {
		tprintf("%#lx, %d, %d, %d, ",
			tcp->u_arg[0],
			(int) tcp->u_arg[1],
			(int) tcp->u_arg[2],
			(int) tcp->u_arg[3]);
		printflags(perf_event_open_flags, tcp->u_arg[4],
			   "PERF_FLAG_???");
	}
	return 0;
}
