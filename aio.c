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
#ifdef HAVE_LIBAIO_H
# include <libaio.h>
#endif

/* Not defined in libaio.h */
#ifndef IOCB_RESFD
# define IOCB_RESFD (1 << 0)
#endif

SYS_FUNC(io_setup)
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
				tprints("{...}");
		}
	}
	return 0;
}

SYS_FUNC(io_destroy)
{
	if (entering(tcp))
		tprintf("%lu", tcp->u_arg[0]);
	return 0;
}

#ifdef HAVE_LIBAIO_H

enum iocb_sub {
	SUB_NONE, SUB_COMMON, SUB_POLL, SUB_VECTOR
};

static enum iocb_sub
tprint_lio_opcode(unsigned cmd)
{
	static const struct {
		const char *name;
		enum iocb_sub sub;
	} cmds[] = {
		{ "pread", SUB_COMMON },
		{ "pwrite", SUB_COMMON },
		{ "fsync", SUB_NONE },
		{ "fdsync", SUB_NONE },
		{ "op4", SUB_NONE },
		{ "poll", SUB_POLL },
		{ "noop", SUB_NONE },
		{ "preadv", SUB_VECTOR },
		{ "pwritev", SUB_VECTOR },
	};

	if (cmd < ARRAY_SIZE(cmds)) {
		tprints(cmds[cmd].name);
		return cmds[cmd].sub;
	}
	tprintf("%u /* SUB_??? */", cmd);
	return SUB_NONE;
}

static void
print_common_flags(struct iocb *iocb)
{
#if HAVE_STRUCT_IOCB_U_C_FLAGS
	if (iocb->u.c.flags & IOCB_RESFD)
		tprintf(", resfd=%d", iocb->u.c.resfd);
	if (iocb->u.c.flags & ~IOCB_RESFD)
		tprintf(", flags=%x", iocb->u.c.flags);
#else
# warning "libaio.h is too old => limited io_submit decoding"
#endif
}

#endif /* HAVE_LIBAIO_H */

SYS_FUNC(io_submit)
{
	if (entering(tcp)) {
#ifdef HAVE_LIBAIO_H
		long nr = tcp->u_arg[1];
		/* if nr <= 0, we end up printing just "{}" */
		tprintf("%lu, %ld, {", tcp->u_arg[0], tcp->u_arg[1]);
		{
			long i;
			struct iocb **iocbs = (void *)tcp->u_arg[2];
//FIXME: decoding of 32-bit call by 64-bit strace

			for (i = 0; i < nr; i++, iocbs++) {
				enum iocb_sub sub;
				struct iocb *iocbp;
				struct iocb iocb;
				if (i)
					tprints(", ");

				if (umove(tcp, (unsigned long)iocbs, &iocbp)) {
					tprintf("%#lx", (unsigned long)iocbs);
					/* No point in trying to read iocbs+1 etc */
					/* (nr can be ridiculously large): */
					break;
				}
				if (umove(tcp, (unsigned long)iocbp, &iocb)) {
					tprintf("{%#lx}", (unsigned long)iocbp);
					continue;
				}
				tprints("{");
				if (iocb.data)
					tprintf("data:%p, ", iocb.data);
				if (iocb.key)
					tprintf("key:%u, ", iocb.key);
				sub = tprint_lio_opcode(iocb.aio_lio_opcode);
				if (iocb.aio_reqprio)
					tprintf(", reqprio:%d", iocb.aio_reqprio);
				tprintf(", filedes:%d", iocb.aio_fildes);
				switch (sub) {
				case SUB_COMMON:
#if HAVE_DECL_IO_CMD_PWRITE
					if (iocb.aio_lio_opcode == IO_CMD_PWRITE) {
						tprints(", str:");
						printstr(tcp, (unsigned long)iocb.u.c.buf,
							 iocb.u.c.nbytes);
					} else
#endif
						tprintf(", buf:%p", iocb.u.c.buf);
					tprintf(", nbytes:%lu, offset:%lld",
						iocb.u.c.nbytes,
						iocb.u.c.offset);
					print_common_flags(&iocb);
					break;
				case SUB_VECTOR:
					tprintf(", %lld", iocb.u.v.offset);
					print_common_flags(&iocb);
					tprints(", ");
					tprint_iov(tcp, iocb.u.v.nr,
						   (unsigned long)iocb.u.v.vec,
#if HAVE_DECL_IO_CMD_PWRITEV
						   iocb.aio_lio_opcode == IO_CMD_PWRITEV
#else
						   0
#endif
						  );
					break;
				case SUB_POLL:
					tprintf(", %x", iocb.u.poll.events);
					break;
				case SUB_NONE:
				        break;
				}
				tprints("}");
			}
		}
		tprints("}");
#else
# warning "libaio.h is not available => no io_submit decoding"
		tprintf("%lu, %ld, %#lx", tcp->u_arg[0], tcp->u_arg[1], tcp->u_arg[2]);
#endif
	}
	return 0;
}

SYS_FUNC(io_cancel)
{
	if (entering(tcp)) {
#ifdef HAVE_LIBAIO_H
		struct iocb iocb;
#endif
		tprintf("%lu, ", tcp->u_arg[0]);
#ifdef HAVE_LIBAIO_H
		if (umove(tcp, tcp->u_arg[1], &iocb) == 0) {
			tprintf("{%p, %u, %u, %u, %d}, ",
				iocb.data, iocb.key,
				(unsigned)iocb.aio_lio_opcode,
				(unsigned)iocb.aio_reqprio, iocb.aio_fildes);
		} else
#endif
			tprints("{...}, ");
	} else {
		if (tcp->u_rval < 0)
			tprints("{...}");
		else {
#ifdef HAVE_LIBAIO_H
			struct io_event event;
			if (umove(tcp, tcp->u_arg[2], &event) == 0)
				tprintf("{%p, %p, %ld, %ld}",
					event.data, event.obj,
					event.res, event.res2);
			else
#endif
				tprints("{...}");
		}
	}
	return 0;
}

SYS_FUNC(io_getevents)
{
	if (entering(tcp)) {
		tprintf("%ld, %ld, %ld, ", tcp->u_arg[0], tcp->u_arg[1],
			tcp->u_arg[2]);
	} else {
		if (tcp->u_rval == 0) {
			tprints("{}");
		} else {
#ifdef HAVE_LIBAIO_H
			struct io_event *events = (void *)tcp->u_arg[3];
			long i, nr = tcp->u_rval;

			for (i = 0; i < nr; i++, events++) {
				struct io_event event;

				if (i == 0)
					tprints("{");
				else
					tprints(", ");

				if (umove(tcp, (unsigned long)events, &event) != 0) {
					tprints("{...}");
					continue;
				}
				tprintf("{%p, %p, %ld, %ld}", event.data,
					event.obj, event.res, event.res2);
			}
			tprints("}, ");
#else
			tprints("{...}");
#endif
		}

		print_timespec(tcp, tcp->u_arg[4]);
	}
	return 0;
}
