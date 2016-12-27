/*
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
#include <poll.h>

#include "xlat/pollflags.h"

static bool
print_pollfd(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	const struct pollfd *fds = elem_buf;

	tprints("{fd=");
	printfd(tcp, fds->fd);
	if (fds->fd >= 0) {
		tprints(", events=");
		printflags(pollflags, (unsigned short) fds->events, "POLL???");
	}
	tprints("}");

	return true;
}

static void
decode_poll_entering(struct tcb *tcp)
{
	const kernel_ulong_t addr = tcp->u_arg[0];
	const unsigned int nfds = tcp->u_arg[1];
	struct pollfd fds;

	print_array(tcp, addr, nfds, &fds, sizeof(fds),
		    umoven_or_printaddr, print_pollfd, 0);
	tprintf(", %u, ", nfds);
}

static int
decode_poll_exiting(struct tcb *const tcp, const kernel_ulong_t pts)
{
	struct pollfd fds;
	const unsigned int nfds = tcp->u_arg[1];
	const unsigned long size = sizeof(fds) * nfds;
	const kernel_ulong_t start = tcp->u_arg[0];
	const kernel_ulong_t end = start + size;
	kernel_ulong_t cur;
	const unsigned int max_printed =
		abbrev(tcp) ? max_strlen : -1U;
	unsigned int printed;

	static char outstr[1024];
	char *outptr;
#define end_outstr (outstr + sizeof(outstr))

	if (syserror(tcp))
		return 0;
	if (tcp->u_rval == 0) {
		tcp->auxstr = "Timeout";
		return RVAL_STR;
	}

	if (!verbose(tcp) || !start || !nfds ||
	    size / sizeof(fds) != nfds || end < start)
		return 0;

	outptr = outstr;

	for (printed = 0, cur = start; cur < end; cur += sizeof(fds)) {
		if (umove(tcp, cur, &fds) < 0) {
			if (outptr == outstr)
				*outptr++ = '[';
			else
				outptr = stpcpy(outptr, ", ");
			outptr += sprintf(outptr, "%#" PRI_klx, cur);
			break;
		}
		if (!fds.revents)
			continue;
		if (outptr == outstr)
			*outptr++ = '[';
		else
			outptr = stpcpy(outptr, ", ");
		if (printed >= max_printed) {
			outptr = stpcpy(outptr, "...");
			break;
		}

		static const char fmt[] = "{fd=%d, revents=";
		char fdstr[sizeof(fmt) + sizeof(int) * 3];
		sprintf(fdstr, fmt, fds.fd);

		const char *flagstr = sprintflags("", pollflags,
						  (unsigned short) fds.revents);

		if (outptr + strlen(fdstr) + strlen(flagstr) + 1 >=
		    end_outstr - (2 + 2 * sizeof(long) + sizeof(", ], ..."))) {
			outptr = stpcpy(outptr, "...");
			break;
		}
		outptr = stpcpy(outptr, fdstr);
		outptr = stpcpy(outptr, flagstr);
		*outptr++ = '}';
		++printed;
	}

	if (outptr != outstr)
		*outptr++ = ']';

	*outptr = '\0';
	if (pts) {
		const char *str = sprint_timespec(tcp, pts);

		if (outptr + sizeof(", left ") + strlen(str) < end_outstr) {
			outptr = stpcpy(outptr, outptr == outstr ? "left " : ", left ");
			outptr = stpcpy(outptr, str);
		} else {
			outptr = stpcpy(outptr, ", ...");
		}
	}

	if (outptr == outstr)
		return 0;

	tcp->auxstr = outstr;
	return RVAL_STR;
#undef end_outstr
}

SYS_FUNC(poll)
{
	if (entering(tcp)) {
		decode_poll_entering(tcp);
		int timeout = tcp->u_arg[2];

#ifdef INFTIM
		if (INFTIM == timeout)
			tprints("INFTIM");
		else
#endif
			tprintf("%d", timeout);

		return 0;
	} else {
		return decode_poll_exiting(tcp, 0);
	}
}

SYS_FUNC(ppoll)
{
	if (entering(tcp)) {
		decode_poll_entering(tcp);

		print_timespec(tcp, tcp->u_arg[2]);
		tprints(", ");
		/* NB: kernel requires arg[4] == NSIG_BYTES */
		print_sigset_addr_len(tcp, tcp->u_arg[3], tcp->u_arg[4]);
		tprintf(", %" PRI_klu, tcp->u_arg[4]);

		return 0;
	} else {
		return decode_poll_exiting(tcp, tcp->u_arg[2]);
	}
}
