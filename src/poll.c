/*
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999-2024 The strace developers.
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

	tprint_struct_begin();
	PRINT_FIELD_FD(*fds, fd, tcp);
	if (fds->fd >= 0) {
		tprint_struct_next();
		PRINT_FIELD_FLAGS(*fds, events, pollflags, "POLL???");
	}
	tprint_struct_end();

	return true;
}

static void
decode_poll_entering(struct tcb *tcp)
{
	const kernel_ulong_t addr = tcp->u_arg[0];
	const unsigned int nfds = tcp->u_arg[1];
	struct pollfd fds;

	/* fds */
	print_array(tcp, addr, nfds, &fds, sizeof(fds),
		    tfetch_mem, print_pollfd, 0);
	tprint_arg_next();

	/* nfds */
	PRINT_VAL_U(nfds);
	tprint_arg_next();
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
	unsigned int printed = 0;

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

	for (kernel_ulong_t cur = start; cur < end; cur += sizeof(fds)) {
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
		/* printed starts with 0, hence printed + 1 */
		if (sequence_truncation_needed(tcp, printed + 1)) {
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

		/* timeout */
		PRINT_VAL_D((int) tcp->u_arg[2]);

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

		/* tmo_p */
		print_ts(tcp, tcp->u_arg[2]);
		tprint_arg_next();

		/* sigmask */
		/* NB: kernel requires arg[4] == NSIG_BYTES */
		print_sigset_addr_len(tcp, tcp->u_arg[3], tcp->u_arg[4]);
		tprint_arg_next();

		/* sigsetsize */
		PRINT_VAL_U(tcp->u_arg[4]);

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
