/*
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <poll.h>
#include "xstring.h"

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
		    tfetch_mem, print_pollfd, 0);
	tprintf(", %u, ", nfds);
}

static int
decode_poll_exiting(struct tcb *const tcp, const sprint_obj_by_addr_fn sprint_ts,
		    const kernel_ulong_t pts)
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
			outptr = xappendstr(outstr, outptr, "%#" PRI_klx, cur);
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
		xsprintf(fdstr, fmt, fds.fd);

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
		const char *str = sprint_ts(tcp, pts);

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

#if HAVE_ARCH_TIME32_SYSCALLS || HAVE_ARCH_OLD_TIME64_SYSCALLS
static int
do_poll(struct tcb *const tcp, const sprint_obj_by_addr_fn sprint_ts)
{
	if (entering(tcp)) {
		decode_poll_entering(tcp);
		tprintf("%d", (int) tcp->u_arg[2]);

		return 0;
	} else {
		return decode_poll_exiting(tcp, sprint_ts, 0);
	}
}
#endif /* HAVE_ARCH_TIME32_SYSCALLS || HAVE_ARCH_OLD_TIME64_SYSCALLS */

#if HAVE_ARCH_TIME32_SYSCALLS
SYS_FUNC(poll_time32)
{
	return do_poll(tcp, sprint_timespec32);
}
#endif

#if HAVE_ARCH_OLD_TIME64_SYSCALLS
SYS_FUNC(poll_time64)
{
	return do_poll(tcp, sprint_timespec64);
}
#endif

static int
do_ppoll(struct tcb *const tcp, const print_obj_by_addr_fn print_ts,
	 const sprint_obj_by_addr_fn sprint_ts)
{
	if (entering(tcp)) {
		decode_poll_entering(tcp);

		print_ts(tcp, tcp->u_arg[2]);
		tprints(", ");
		/* NB: kernel requires arg[4] == NSIG_BYTES */
		print_sigset_addr_len(tcp, tcp->u_arg[3], tcp->u_arg[4]);
		tprintf(", %" PRI_klu, tcp->u_arg[4]);

		return 0;
	} else {
		return decode_poll_exiting(tcp, sprint_ts, tcp->u_arg[2]);
	}
}

#if HAVE_ARCH_TIME32_SYSCALLS
SYS_FUNC(ppoll_time32)
{
	return do_ppoll(tcp, print_timespec32, sprint_timespec32);
}
#endif

SYS_FUNC(ppoll_time64)
{
	return do_ppoll(tcp, print_timespec64, sprint_timespec64);
}
