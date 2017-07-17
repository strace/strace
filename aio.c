/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999-2017 The strace developers.
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
#include "print_fields.h"
#include <linux/aio_abi.h>

SYS_FUNC(io_setup)
{
	if (entering(tcp))
		tprintf("%u, ", (unsigned int) tcp->u_arg[0]);
	else
		printnum_ptr(tcp, tcp->u_arg[1]);
	return 0;
}

SYS_FUNC(io_destroy)
{
	printaddr(tcp->u_arg[0]);

	return RVAL_DECODED;
}

enum iocb_sub {
	SUB_NONE, SUB_COMMON, SUB_VECTOR
};

static enum iocb_sub
tprint_lio_opcode(unsigned int cmd)
{
	static const struct {
		const char *name;
		enum iocb_sub sub;
	} cmds[] = {
		{ "IOCB_CMD_PREAD", SUB_COMMON },
		{ "IOCB_CMD_PWRITE", SUB_COMMON },
		{ "IOCB_CMD_FSYNC", SUB_NONE },
		{ "IOCB_CMD_FDSYNC", SUB_NONE },
		{ "IOCB_CMD_PREADX", SUB_NONE },
		{ "IOCB_CMD_POLL", SUB_NONE },
		{ "IOCB_CMD_NOOP", SUB_NONE },
		{ "IOCB_CMD_PREADV", SUB_VECTOR },
		{ "IOCB_CMD_PWRITEV", SUB_VECTOR },
	};

	if (cmd < ARRAY_SIZE(cmds)) {
		tprints(cmds[cmd].name);
		return cmds[cmd].sub;
	}
	tprintf("%u", cmd);
	tprints_comment("IOCB_CMD_???");
	return SUB_NONE;
}

static void
print_common_flags(struct tcb *tcp, const struct iocb *cb)
{
/* IOCB_FLAG_RESFD is available since v2.6.22-rc1~47 */
#ifdef IOCB_FLAG_RESFD
	if (cb->aio_flags & IOCB_FLAG_RESFD)
		PRINT_FIELD_FD(", ", *cb, aio_resfd, tcp);

	if (cb->aio_flags & ~IOCB_FLAG_RESFD)
		PRINT_FIELD_X(", ", *cb, aio_flags);
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
print_iocb_header(struct tcb *tcp, const struct iocb *cb)
{
	enum iocb_sub sub;

	if (cb->aio_data){
		PRINT_FIELD_X("", *cb, aio_data);
		tprints(", ");
	}

	if (cb->aio_key) {
		PRINT_FIELD_U("", *cb, aio_key);
		tprints(", ");
	}

	tprints("aio_lio_opcode=");
	sub = tprint_lio_opcode(cb->aio_lio_opcode);
	if (cb->aio_reqprio)
		PRINT_FIELD_D(", ", *cb, aio_reqprio);

	PRINT_FIELD_FD(", ", *cb, aio_fildes, tcp);

	return sub;
}

static void
print_iocb(struct tcb *tcp, const struct iocb *cb)
{
	enum iocb_sub sub = print_iocb_header(tcp, cb);

	switch (sub) {
	case SUB_COMMON:
		if (cb->aio_lio_opcode == 1 && iocb_is_valid(cb)) {
			PRINT_FIELD_STRN(", ", *cb, aio_buf,
					 cb->aio_nbytes, tcp);
		} else {
			PRINT_FIELD_X(", ", *cb, aio_buf);
		}
		PRINT_FIELD_U(", ", *cb, aio_nbytes);
		PRINT_FIELD_D(", ", *cb, aio_offset);
		print_common_flags(tcp, cb);
		break;
	case SUB_VECTOR:
		if (iocb_is_valid(cb)) {
			tprints(", aio_buf=");
			tprint_iov(tcp, cb->aio_nbytes, cb->aio_buf,
				   cb->aio_lio_opcode == 8
				   ? IOV_DECODE_STR
				   : IOV_DECODE_ADDR);
		} else {
			PRINT_FIELD_X(", ", *cb, aio_buf);
			PRINT_FIELD_U(", ", *cb, aio_nbytes);
		}
		PRINT_FIELD_D(", ", *cb, aio_offset);
		print_common_flags(tcp, cb);
		break;
	case SUB_NONE:
		break;
	}
}

static bool
print_iocbp(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	kernel_ulong_t addr;
	struct iocb cb;

	if (elem_size < sizeof(kernel_ulong_t)) {
		addr = *(unsigned int *) elem_buf;
	} else {
		addr = *(kernel_ulong_t *) elem_buf;
	}

	tprints("{");
	if (!umove_or_printaddr(tcp, addr, &cb))
		print_iocb(tcp, &cb);
	tprints("}");

	return true;
}

SYS_FUNC(io_submit)
{
	const kernel_long_t nr =
		truncate_klong_to_current_wordsize(tcp->u_arg[1]);
	const kernel_ulong_t addr = tcp->u_arg[2];
	kernel_ulong_t iocbp;

	printaddr(tcp->u_arg[0]);
	tprintf(", %" PRI_kld ", ", nr);

	if (nr < 0)
		printaddr(addr);
	else
		print_array(tcp, addr, nr, &iocbp, current_wordsize,
			    umoven_or_printaddr, print_iocbp, 0);

	return RVAL_DECODED;
}

static bool
print_io_event(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	struct io_event *event = elem_buf;

	PRINT_FIELD_X("{", *event, data);
	PRINT_FIELD_X(", ", *event, obj);
	PRINT_FIELD_D(", ", *event, res);
	PRINT_FIELD_D(", ", *event, res2);
	tprints("}");

	return true;
}

SYS_FUNC(io_cancel)
{
	if (entering(tcp)) {
		printaddr(tcp->u_arg[0]);
		tprints(", ");

		struct iocb cb;

		if (!umove_or_printaddr(tcp, tcp->u_arg[1], &cb)) {
			tprints("{");
			print_iocb_header(tcp, &cb);
			tprints("}");
		}
		tprints(", ");
	} else {
		struct io_event event;

		if (!umove_or_printaddr(tcp, tcp->u_arg[2], &event))
			print_io_event(tcp, &event, sizeof(event), 0);
	}
	return 0;
}

SYS_FUNC(io_getevents)
{
	if (entering(tcp)) {
		printaddr(tcp->u_arg[0]);
		tprintf(", %" PRI_kld ", %" PRI_kld ", ",
			truncate_klong_to_current_wordsize(tcp->u_arg[1]),
			truncate_klong_to_current_wordsize(tcp->u_arg[2]));
	} else {
		struct io_event buf;
		print_array(tcp, tcp->u_arg[3], tcp->u_rval, &buf, sizeof(buf),
			    umoven_or_printaddr, print_io_event, 0);
		tprints(", ");
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
