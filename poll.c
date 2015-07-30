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

static int
decode_poll(struct tcb *tcp, long pts)
{
	struct pollfd fds;
	unsigned nfds;
	unsigned long size, start, cur, end, abbrev_end;
	int failed = 0;

	if (entering(tcp)) {
		nfds = tcp->u_arg[1];
		size = sizeof(fds) * nfds;
		start = tcp->u_arg[0];
		end = start + size;
		if (nfds == 0 || size / sizeof(fds) != nfds || end < start) {
			tprintf("%#lx, %d, ",
				tcp->u_arg[0], nfds);
			return 0;
		}
		if (abbrev(tcp)) {
			abbrev_end = start + max_strlen * sizeof(fds);
			if (abbrev_end < start)
				abbrev_end = end;
		} else {
			abbrev_end = end;
		}
		tprints("[");
		for (cur = start; cur < end; cur += sizeof(fds)) {
			if (cur > start)
				tprints(", ");
			if (cur >= abbrev_end) {
				tprints("...");
				break;
			}
			if (umoven(tcp, cur, sizeof fds, &fds) < 0) {
				tprints("?");
				failed = 1;
				break;
			}
			if (fds.fd < 0) {
				tprintf("{fd=%d}", fds.fd);
				continue;
			}
			tprints("{fd=");
			printfd(tcp, fds.fd);
			tprints(", events=");
			printflags(pollflags, fds.events, "POLL???");
			tprints("}");
		}
		tprints("]");
		if (failed)
			tprintf(" %#lx", start);
		tprintf(", %d, ", nfds);
		return 0;
	} else {
		static char outstr[1024];
		char *outptr;
#define end_outstr (outstr + sizeof(outstr))
		const char *flagstr;

		if (syserror(tcp))
			return 0;
		if (tcp->u_rval == 0) {
			tcp->auxstr = "Timeout";
			return RVAL_STR;
		}

		nfds = tcp->u_arg[1];
		size = sizeof(fds) * nfds;
		start = tcp->u_arg[0];
		end = start + size;
		if (nfds == 0 || size / sizeof(fds) != nfds || end < start)
			return 0;
		if (abbrev(tcp)) {
			abbrev_end = start + max_strlen * sizeof(fds);
			if (abbrev_end < start)
				abbrev_end = end;
		} else {
			abbrev_end = end;
		}

		outptr = outstr;

		for (cur = start; cur < end; cur += sizeof(fds)) {
			if (umoven(tcp, cur, sizeof fds, &fds) < 0) {
				if (outptr < end_outstr - 2)
					*outptr++ = '?';
				failed = 1;
				break;
			}
			if (!fds.revents)
				continue;
			if (outptr == outstr) {
				*outptr++ = '[';
			} else {
				if (outptr < end_outstr - 3)
					outptr = stpcpy(outptr, ", ");
			}
			if (cur >= abbrev_end) {
				if (outptr < end_outstr - 4)
					outptr = stpcpy(outptr, "...");
				break;
			}
			if (outptr < end_outstr - (sizeof("{fd=%d, revents=") + sizeof(int)*3) + 1)
				outptr += sprintf(outptr, "{fd=%d, revents=", fds.fd);
			flagstr = sprintflags("", pollflags, fds.revents);
			if (outptr < end_outstr - (strlen(flagstr) + 2)) {
				outptr = stpcpy(outptr, flagstr);
				*outptr++ = '}';
			}
		}
		if (failed)
			return 0;

		if (outptr != outstr /* && outptr < end_outstr - 1 (always true)*/)
			*outptr++ = ']';

		*outptr = '\0';
		if (pts) {
			if (outptr < end_outstr - (10 + TIMESPEC_TEXT_BUFSIZE)) {
				outptr = stpcpy(outptr, outptr == outstr ? "left " : ", left ");
				sprint_timespec(outptr, tcp, pts);
			}
		}

		if (outptr == outstr)
			return 0;

		tcp->auxstr = outstr;
		return RVAL_STR;
#undef end_outstr
	}
}

SYS_FUNC(poll)
{
	int rc = decode_poll(tcp, 0);
	if (entering(tcp)) {
#ifdef INFTIM
		if (INFTIM == (int) tcp->u_arg[2])
			tprints("INFTIM");
		else
#endif
			tprintf("%d", (int) tcp->u_arg[2]);
	}
	return rc;
}

SYS_FUNC(ppoll)
{
	int rc = decode_poll(tcp, tcp->u_arg[2]);
	if (entering(tcp)) {
		print_timespec(tcp, tcp->u_arg[2]);
		tprints(", ");
		/* NB: kernel requires arg[4] == NSIG / 8 */
		print_sigset_addr_len(tcp, tcp->u_arg[3], tcp->u_arg[4]);
		tprintf(", %lu", tcp->u_arg[4]);
	}
	return rc;
}
