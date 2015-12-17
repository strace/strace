/*
 * Copyright (c) 2004-2007 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2004 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2005-2015 Dmitry V. Levin <ldv@altlinux.org>
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
#ifdef HAVE_SYS_EPOLL_H
# include <sys/epoll.h>
#endif

SYS_FUNC(epoll_create)
{
	tprintf("%d", (int) tcp->u_arg[0]);

	return RVAL_DECODED | RVAL_FD;
}

#include "xlat/epollflags.h"

SYS_FUNC(epoll_create1)
{
	printflags(epollflags, tcp->u_arg[0], "EPOLL_???");

	return RVAL_DECODED | RVAL_FD;
}

#ifdef HAVE_SYS_EPOLL_H
# include "xlat/epollevents.h"

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

#include "xlat/epollctls.h"

SYS_FUNC(epoll_ctl)
{
	struct epoll_event ev;

	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	printxval(epollctls, tcp->u_arg[1], "EPOLL_CTL_???");
	tprints(", ");
	printfd(tcp, tcp->u_arg[2]);
	tprints(", ");
#ifdef HAVE_SYS_EPOLL_H
	if (EPOLL_CTL_DEL == tcp->u_arg[1])
		printaddr(tcp->u_arg[3]);
	else if (!umove_or_printaddr(tcp, tcp->u_arg[3], &ev))
		print_epoll_event(&ev);
#else
	printaddr(tcp->u_arg[3]);
#endif

	return RVAL_DECODED;
}

static void
print_epoll_event_array(struct tcb *tcp, const long addr, const long len)
{
#ifdef HAVE_SYS_EPOLL_H
	struct epoll_event ev, *start, *cur, *end;

	if (!len) {
		tprints("[]");
		return;
	}

	if (umove_or_printaddr(tcp, addr, &ev))
		return;

	tprints("[");
	print_epoll_event(&ev);

	start = (struct epoll_event *) addr;
	end = start + len;
	for (cur = start + 1; cur < end; ++cur) {
		tprints(", ");
		if (umove_or_printaddr(tcp, (long) cur, &ev))
			break;
		print_epoll_event(&ev);
	}
	tprints("]");
#else
	printaddr(addr);
#endif
}

static void
epoll_wait_common(struct tcb *tcp)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		print_epoll_event_array(tcp, tcp->u_arg[1], tcp->u_rval);
		tprintf(", %d, %d", (int) tcp->u_arg[2], (int) tcp->u_arg[3]);
	}
}

SYS_FUNC(epoll_wait)
{
	epoll_wait_common(tcp);
	return 0;
}

SYS_FUNC(epoll_pwait)
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
