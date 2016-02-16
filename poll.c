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

static void
print_pollfd(struct tcb *tcp, const struct pollfd *fds)
{
	tprints("{fd=");
	printfd(tcp, fds->fd);
	if (fds->fd >= 0) {
		tprints(", events=");
		printflags(pollflags, fds->events, "POLL???");
	}
	tprints("}");
}

static int
decode_poll_entering(struct tcb *tcp)
{
	struct pollfd fds;
	const unsigned int nfds = tcp->u_arg[1];
	const unsigned long size = sizeof(fds) * nfds;
	const unsigned long start = tcp->u_arg[0];
	const unsigned long end = start + size;
	const unsigned long max_printed =
		abbrev(tcp) ? max_strlen : (unsigned int) -1;

	if (!verbose(tcp) || !start || !nfds ||
	    size / sizeof(fds) != nfds || end < start) {
		printaddr(start);
		tprintf(", %u, ", nfds);
		return 0;
	}

	if (umove(tcp, start, &fds) < 0) {
		printaddr(start);
		tprintf(", %u, ", nfds);
		return 0;
	}

	tprints("[");
	if (max_printed) {
		unsigned long printed = 1;
		unsigned long cur = start + sizeof(fds);

		print_pollfd(tcp, &fds);
		for (; cur < end; ++printed, cur += sizeof(fds)) {
			tprints(", ");
			if (printed >= max_printed) {
				tprints("...");
				break;
			}
			if (umove_or_printaddr(tcp, cur, &fds))
				break;
			print_pollfd(tcp, &fds);

		}
	} else {
		tprints("...");
	}
	tprintf("], %u, ", nfds);

	return 0;
}

static int
decode_poll_exiting(struct tcb *tcp, const long pts)
{
	struct pollfd fds;
	const unsigned int nfds = tcp->u_arg[1];
	const unsigned long size = sizeof(fds) * nfds;
	const unsigned long start = tcp->u_arg[0];
	const unsigned long end = start + size;
	const unsigned long max_printed =
		abbrev(tcp) ? max_strlen : (unsigned int) -1;
	unsigned long printed, cur;

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
			outptr += sprintf(outptr, "%#lx", cur);
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

		const char *flagstr = sprintflags("", pollflags, fds.revents);

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
		int rc = decode_poll_entering(tcp);

#ifdef INFTIM
		if (INFTIM == (int) tcp->u_arg[2])
			tprints("INFTIM");
		else
#endif
			tprintf("%d", (int) tcp->u_arg[2]);

		return rc;
	} else {
		return decode_poll_exiting(tcp, 0);
	}
}

SYS_FUNC(ppoll)
{
	if (entering(tcp)) {
		int rc = decode_poll_entering(tcp);

		print_timespec(tcp, tcp->u_arg[2]);
		tprints(", ");
		/* NB: kernel requires arg[4] == NSIG / 8 */
		print_sigset_addr_len(tcp, tcp->u_arg[3], tcp->u_arg[4]);
		tprintf(", %lu", tcp->u_arg[4]);

		return rc;
	} else {
		return decode_poll_exiting(tcp, tcp->u_arg[2]);
	}
}
