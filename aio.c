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

/* --- Copied from libaio-0.3.109/src/libaio.h ---
 * Why keep a copy instead of using external libaio.h?
 * Because we want to properly decode 32-bit aio calls
 * by 64-bit strace. For that, we need more definitions than
 * libaio.h provides. (TODO).
 * Keeping our local 32-bit compat defs in sync with libaio.h
 * _without seeing libaio structs_ is hard/more bug-prone.
 * A smaller benefit is that we don't need libaio installed.
 */
#define HAVE_LIBAIO_H 1
typedef enum io_iocb_cmd {
	IO_CMD_PREAD = 0,
	IO_CMD_PWRITE = 1,

	IO_CMD_FSYNC = 2,
	IO_CMD_FDSYNC = 3,

	IO_CMD_POLL = 5, /* Never implemented in mainline, see io_prep_poll */
	IO_CMD_NOOP = 6,
	IO_CMD_PREADV = 7,
	IO_CMD_PWRITEV = 8,
} io_iocb_cmd_t;

#if defined(__i386__) /* little endian, 32 bits */
#define PADDED(x, y)	x; unsigned y
#define PADDEDptr(x, y)	x; unsigned y
#define PADDEDul(x, y)	unsigned long x; unsigned y
#elif defined(__ia64__) || defined(__x86_64__) || defined(__alpha__)
#define PADDED(x, y)	x, y
#define PADDEDptr(x, y)	x
#define PADDEDul(x, y)	unsigned long x
#elif defined(__powerpc64__) /* big endian, 64 bits */
#define PADDED(x, y)	unsigned y; x
#define PADDEDptr(x,y)	x
#define PADDEDul(x, y)	unsigned long x
#elif defined(__PPC__)  /* big endian, 32 bits */
#define PADDED(x, y)	unsigned y; x
#define PADDEDptr(x, y)	unsigned y; x
#define PADDEDul(x, y)	unsigned y; unsigned long x
#elif defined(__s390x__) /* big endian, 64 bits */
#define PADDED(x, y)	unsigned y; x
#define PADDEDptr(x,y)	x
#define PADDEDul(x, y)	unsigned long x
#elif defined(__s390__) /* big endian, 32 bits */
#define PADDED(x, y)	unsigned y; x
#define PADDEDptr(x, y) unsigned y; x
#define PADDEDul(x, y)	unsigned y; unsigned long x
#elif defined(__arm__)
#  if defined (__ARMEB__) /* big endian, 32 bits */
#define PADDED(x, y)	unsigned y; x
#define PADDEDptr(x, y)	unsigned y; x
#define PADDEDul(x, y)	unsigned y; unsigned long x
#  else                   /* little endian, 32 bits */
#define PADDED(x, y)	x; unsigned y
#define PADDEDptr(x, y)	x; unsigned y
#define PADDEDul(x, y)	unsigned long x; unsigned y
#  endif
#else
#  warning No AIO definitions for this architecture => no io_submit decoding
#  undef HAVE_LIBAIO_H
#endif

#ifdef HAVE_LIBAIO_H
struct io_iocb_poll {
	PADDED(int events, __pad1);
};	/* result code is the set of result flags or -'ve errno */

struct io_iocb_sockaddr {
	struct sockaddr *addr;
	int		len;
};	/* result code is the length of the sockaddr, or -'ve errno */

struct io_iocb_common {
	PADDEDptr(void	*buf, __pad1);
	PADDEDul(nbytes, __pad2);
	long long	offset;
	long long	__pad3;
	unsigned	flags;
	unsigned	resfd;
};	/* result code is the amount read or -'ve errno */

struct io_iocb_vector {
	const struct iovec	*vec;
	int			nr;
	long long		offset;
};	/* result code is the amount read or -'ve errno */

struct iocb {
	PADDEDptr(void *data, __pad1);	/* Return in the io completion event */
	PADDED(unsigned key, __pad2);	/* For use in identifying io requests */

	short		aio_lio_opcode;
	short		aio_reqprio;
	int		aio_fildes;

	union {
		struct io_iocb_common		c;
		struct io_iocb_vector		v;
		struct io_iocb_poll		poll;
		struct io_iocb_sockaddr	saddr;
	} u;
};

struct io_event {
	PADDEDptr(void *data, __pad1);
	PADDEDptr(struct iocb *obj,  __pad2);
	PADDEDul(res,  __pad3);
	PADDEDul(res2, __pad4);
};

#undef PADDED
#undef PADDEDptr
#undef PADDEDul

#endif /* HAVE_LIBAIO_H */

/* --- End of a chunk of libaio.h --- */
/* Not defined in libaio.h */
#ifndef IOCB_RESFD
# define IOCB_RESFD (1 << 0)
#endif

int
sys_io_setup(struct tcb *tcp)
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

int
sys_io_destroy(struct tcb *tcp)
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
	if (iocb->u.c.flags & IOCB_RESFD)
		tprintf(", resfd=%d", iocb->u.c.resfd);
	if (iocb->u.c.flags & ~IOCB_RESFD)
		tprintf(", flags=%x", iocb->u.c.flags);
}

#endif /* HAVE_LIBAIO_H */

int
sys_io_submit(struct tcb *tcp)
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
					if (iocb.aio_lio_opcode == IO_CMD_PWRITE) {
						tprints(", str:");
						printstr(tcp, (unsigned long)iocb.u.c.buf,
							 iocb.u.c.nbytes);
					} else
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
						   iocb.aio_lio_opcode == IO_CMD_PWRITEV
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
		tprintf("%lu, %ld, %#lx", tcp->u_arg[0], tcp->u_arg[1], tcp->u_arg[2]);
#endif
	}
	return 0;
}

int
sys_io_cancel(struct tcb *tcp)
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

int
sys_io_getevents(struct tcb *tcp)
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
