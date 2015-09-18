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
#include <linux/aio_abi.h>

SYS_FUNC(io_setup)
{
	if (entering(tcp))
		tprintf("%u, ", (unsigned int) tcp->u_arg[0]);
	else
		printnum_ulong(tcp, tcp->u_arg[1]);
	return 0;
}

SYS_FUNC(io_destroy)
{
	tprintf("%lu", tcp->u_arg[0]);

	return RVAL_DECODED;
}

enum iocb_sub {
	SUB_NONE, SUB_COMMON, SUB_VECTOR
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
		{ "preadx", SUB_NONE },
		{ "poll", SUB_NONE },
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
print_common_flags(const struct iocb *cb)
{
/* IOCB_FLAG_RESFD is available since v2.6.22-rc1~47 */
#ifdef IOCB_FLAG_RESFD
	if (cb->aio_flags & IOCB_FLAG_RESFD)
		tprintf(", resfd=%d", cb->aio_resfd);
	if (cb->aio_flags & ~IOCB_FLAG_RESFD)
		tprintf(", flags=%x", cb->aio_flags);
#endif
}

static bool
iocb_is_valid(const struct iocb *cb)
{
	return cb->aio_buf == (unsigned long) cb->aio_buf &&
	       cb->aio_nbytes == (size_t) cb->aio_nbytes &&
	       (ssize_t) cb->aio_nbytes >= 0;
}

static enum iocb_sub
print_iocb_header(const struct iocb *cb)
{
	enum iocb_sub sub;

	if (cb->aio_data)
		tprintf("data=%#" PRIx64 ", ",
			(uint64_t) cb->aio_data);

	if (cb->aio_key)
		tprintf("key=%u, ", cb->aio_key);

	sub = tprint_lio_opcode(cb->aio_lio_opcode);
	if (cb->aio_reqprio)
		tprintf(", reqprio=%hd", cb->aio_reqprio);

	tprintf(", fildes=%d", cb->aio_fildes);

	return sub;
}

static void
print_iocb(struct tcb *tcp, const struct iocb *cb)
{
	enum iocb_sub sub = print_iocb_header(cb);

	switch (sub) {
	case SUB_COMMON:
		if (cb->aio_lio_opcode == 1 && iocb_is_valid(cb)) {
			tprints(", str=");
			printstr(tcp, (unsigned long) cb->aio_buf,
				 (unsigned long) cb->aio_nbytes);
		} else {
			tprintf(", buf=%#" PRIx64, (uint64_t) cb->aio_buf);
		}
		tprintf(", nbytes=%" PRIu64 ", offset=%" PRId64,
			(uint64_t) cb->aio_nbytes, (int64_t) cb->aio_offset);
		print_common_flags(cb);
		break;
	case SUB_VECTOR:
		if (iocb_is_valid(cb)) {
			tprints(", iovec=");
			tprint_iov(tcp, cb->aio_nbytes, cb->aio_buf,
				   cb->aio_lio_opcode == 8);
		} else {
			tprintf(", buf=%#" PRIx64 ", nbytes=%" PRIu64,
				(uint64_t) cb->aio_buf,
				(uint64_t) cb->aio_nbytes);
		}
		tprintf(", offset=%" PRId64, (int64_t) cb->aio_offset);
		print_common_flags(cb);
		break;
	case SUB_NONE:
		break;
	}
}

SYS_FUNC(io_submit)
{
	long nr = tcp->u_arg[1];
	/* if nr <= 0, we end up printing just "[]" */
	tprintf("%lu, %ld, [", tcp->u_arg[0], nr);
	{
		long i;
		long iocbs = tcp->u_arg[2];

		for (i = 0; i < nr; ++i, iocbs += current_wordsize) {
			unsigned long iocbp;
			struct iocb cb;

			if (i)
				tprints(", ");

			if (umove_ulong_or_printaddr(tcp, iocbs, &iocbp)) {
				/*
				 * No point in trying to read the whole array
				 * because nr can be ridiculously large.
				 */
				break;
			}

			tprints("{");
			if (!umove_or_printaddr(tcp, iocbp, &cb))
				print_iocb(tcp, &cb);
			tprints("}");
		}
	}
	tprints("]");
	return RVAL_DECODED;
}

static int
print_io_event(struct tcb *tcp, const long addr)
{
	struct io_event event;

	if (umove_or_printaddr(tcp, addr, &event))
		return -1;
	tprintf("{data=%#" PRIx64 ", obj=%#" PRIx64
		", res=%" PRId64 ", res2=%" PRId64 "}",
		(uint64_t) event.data, (uint64_t) event.obj,
		(int64_t) event.res, (int64_t) event.res2);
	return 0;
}

SYS_FUNC(io_cancel)
{
	if (entering(tcp)) {
		tprintf("%lu, ", tcp->u_arg[0]);
		struct iocb cb;

		if (!umove_or_printaddr(tcp, tcp->u_arg[1], &cb)) {
			tprints("{");
			print_iocb_header(&cb);
			tprints("}");
		}
		tprints(", ");
	} else {
		print_io_event(tcp, tcp->u_arg[2]);
	}
	return 0;
}

SYS_FUNC(io_getevents)
{
	if (entering(tcp)) {
		tprintf("%lu, %ld, %ld, ",
			tcp->u_arg[0], tcp->u_arg[1], tcp->u_arg[2]);
	} else {
		if (tcp->u_rval == 0) {
			tprints("[]");
		} else {
			struct io_event *events = (void *)tcp->u_arg[3];
			long i, nr = tcp->u_rval;

			for (i = 0; i < nr; i++, events++) {
				if (i == 0)
					tprints("[");
				else
					tprints(", ");

				if (print_io_event(tcp, (long)events))
					break;
			}
			tprints("], ");
		}

		/*
		 * Since the timeout parameter is read by the kernel
		 * on entering syscall, it has to be decoded the same way
		 * whether the syscall has failed or not.
		 */
		temporarily_clear_syserror(tcp);
		print_timespec(tcp, tcp->u_arg[4]);
		restore_cleared_syserror(tcp);
	}
	return 0;
}
