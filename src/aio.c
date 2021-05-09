/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/aio_abi.h>

#include "xlat/aio_cmds.h"

#ifdef HAVE_STRUCT_IOCB_AIO_FLAGS
# include "xlat/aio_iocb_flags.h"
#endif

SYS_FUNC(io_setup)
{
	if (entering(tcp)) {
		/* nr_events */
		unsigned int nr_events = tcp->u_arg[0];
		PRINT_VAL_U(nr_events);
		tprint_arg_next();
	} else {
		/* ctx_idp */
		printnum_ptr(tcp, tcp->u_arg[1]);
	}
	return 0;
}

SYS_FUNC(io_destroy)
{
	/* ctx_id */
	printaddr(tcp->u_arg[0]);

	return RVAL_DECODED;
}

enum iocb_sub {
	SUB_NONE, SUB_COMMON, SUB_VECTOR, SUB_POLL
};

static enum iocb_sub
tprint_lio_opcode(unsigned int cmd)
{
	static const enum iocb_sub subs[] = {
		[IOCB_CMD_PREAD]	= SUB_COMMON,
		[IOCB_CMD_PWRITE]	= SUB_COMMON,
		[IOCB_CMD_FSYNC]	= SUB_NONE,
		[IOCB_CMD_FDSYNC]	= SUB_NONE,
		[IOCB_CMD_PREADX]	= SUB_NONE,
		[IOCB_CMD_POLL]		= SUB_POLL,
		[IOCB_CMD_NOOP]		= SUB_NONE,
		[IOCB_CMD_PREADV]	= SUB_VECTOR,
		[IOCB_CMD_PWRITEV]	= SUB_VECTOR,
	};

	printxval_ex(aio_cmds, cmd, "IOCB_CMD_???", XLAT_STYLE_FMT_U);

	return cmd < ARRAY_SIZE(subs) ? subs[cmd] : SUB_NONE;
}

static void
print_common_flags(struct tcb *tcp, const struct iocb *cb)
{
/* aio_flags and aio_resfd fields are available since v2.6.22-rc1~47 */
#ifdef HAVE_STRUCT_IOCB_AIO_FLAGS
	if (cb->aio_flags) {
		tprint_struct_next();
		PRINT_FIELD_FLAGS(*cb, aio_flags, aio_iocb_flags,
				  "IOCB_FLAG_???");
	}

	if (cb->aio_flags & IOCB_FLAG_RESFD) {
		tprint_struct_next();
		PRINT_FIELD_FD(*cb, aio_resfd, tcp);
	} else if (cb->aio_resfd) {
		tprint_struct_next();
		PRINT_FIELD_X(*cb, aio_resfd);
	}
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

	PRINT_FIELD_X(*cb, aio_data);

	if (cb->aio_key) {
		tprint_struct_next();
		PRINT_FIELD_U(*cb, aio_key);
	}

#ifndef HAVE_STRUCT_IOCB_AIO_RW_FLAGS
# define aio_rw_flags aio_reserved1
#endif
	if (cb->aio_rw_flags) {
		tprint_struct_next();
		PRINT_FIELD_FLAGS(*cb, aio_rw_flags, rwf_flags, "RWF_???");
	}

	tprint_struct_next();
	tprints_field_name("aio_lio_opcode");
	sub = tprint_lio_opcode(cb->aio_lio_opcode);

	if (cb->aio_flags & IOCB_FLAG_IOPRIO) {
		tprint_struct_next();
		PRINT_FIELD_OBJ_U(*cb, aio_reqprio, print_ioprio);
	} else if (cb->aio_reqprio) {
		tprint_struct_next();
		PRINT_FIELD_D(*cb, aio_reqprio);
	}

	tprint_struct_next();
	PRINT_FIELD_FD(*cb, aio_fildes, tcp);

	return sub;
}

static void
print_iocb(struct tcb *tcp, const struct iocb *cb)
{
	tprint_struct_begin();

	enum iocb_sub sub = print_iocb_header(tcp, cb);

	switch (sub) {
	case SUB_COMMON:
		if (cb->aio_lio_opcode == 1 && iocb_is_valid(cb)) {
			tprint_struct_next();
			PRINT_FIELD_OBJ_TCB_VAL(*cb, aio_buf, tcp,
				printstrn, cb->aio_nbytes);
		} else {
			tprint_struct_next();
			PRINT_FIELD_X(*cb, aio_buf);
		}
		tprint_struct_next();
		PRINT_FIELD_U(*cb, aio_nbytes);
		tprint_struct_next();
		PRINT_FIELD_D(*cb, aio_offset);
		print_common_flags(tcp, cb);
		break;
	case SUB_VECTOR:
		if (iocb_is_valid(cb)) {
			tprint_struct_next();
			tprints_field_name("aio_buf");
			tprint_iov(tcp, cb->aio_nbytes, cb->aio_buf,
				   cb->aio_lio_opcode == 8
				   ? iov_decode_str
				   : iov_decode_addr);
		} else {
			tprint_struct_next();
			PRINT_FIELD_X(*cb, aio_buf);
			tprint_struct_next();
			PRINT_FIELD_U(*cb, aio_nbytes);
		}
		tprint_struct_next();
		PRINT_FIELD_D(*cb, aio_offset);
		print_common_flags(tcp, cb);
		break;
	case SUB_POLL:
		tprint_struct_next();
		PRINT_FIELD_FLAGS(*cb, aio_buf, pollflags, "POLL???");
		print_common_flags(tcp, cb);
		break;
	case SUB_NONE:
		break;
	}

	tprint_struct_end();
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

	if (!umove_or_printaddr(tcp, addr, &cb))
		print_iocb(tcp, &cb);

	return true;
}

SYS_FUNC(io_submit)
{
	const kernel_ulong_t addr = tcp->u_arg[2];
	kernel_ulong_t iocbp;

	/* ctx_id */
	printaddr(tcp->u_arg[0]);
	tprint_arg_next();

	/* nr */
	const kernel_long_t nr =
		truncate_klong_to_current_wordsize(tcp->u_arg[1]);
	PRINT_VAL_D(nr);
	tprint_arg_next();

	/* iocbpp */
	if (nr < 0)
		printaddr(addr);
	else
		print_array(tcp, addr, nr, &iocbp, current_wordsize,
			    tfetch_mem, print_iocbp, 0);

	return RVAL_DECODED;
}

static bool
print_io_event(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	struct io_event *event = elem_buf;

	tprint_struct_begin();
	PRINT_FIELD_X(*event, data);
	tprint_struct_next();
	PRINT_FIELD_X(*event, obj);
	tprint_struct_next();
	PRINT_FIELD_D(*event, res);
	tprint_struct_next();
	PRINT_FIELD_D(*event, res2);
	tprint_struct_end();

	return true;
}

SYS_FUNC(io_cancel)
{
	if (entering(tcp)) {
		/* ctx_id */
		printaddr(tcp->u_arg[0]);
		tprint_arg_next();

		/* iocb */
		struct iocb cb;
		if (!umove_or_printaddr(tcp, tcp->u_arg[1], &cb)) {
			tprint_struct_begin();
			print_iocb_header(tcp, &cb);
			tprint_struct_end();
		}
		tprint_arg_next();
	} else {
		/* result */
		struct io_event event;
		if (!umove_or_printaddr(tcp, tcp->u_arg[2], &event))
			print_io_event(tcp, &event, sizeof(event), 0);
	}
	return 0;
}

static int
print_io_getevents(struct tcb *const tcp, const print_obj_by_addr_fn print_ts,
		   const bool has_sig)
{
	if (entering(tcp)) {
		kernel_long_t nr;

		/* ctx_id */
		printaddr(tcp->u_arg[0]);
		tprint_arg_next();

		/* min_nr */
		nr = truncate_klong_to_current_wordsize(tcp->u_arg[1]);
		PRINT_VAL_D(nr);
		tprint_arg_next();

		/* nr */
		nr = truncate_klong_to_current_wordsize(tcp->u_arg[2]);
		PRINT_VAL_D(nr);
		tprint_arg_next();
	} else {
		/* events */
		struct io_event buf;
		print_array(tcp, tcp->u_arg[3], tcp->u_rval, &buf, sizeof(buf),
			    tfetch_mem, print_io_event, 0);
		tprint_arg_next();

		/*
		 * Since the timeout and sig parameters are read by the kernel
		 * on entering syscall, it has to be decoded the same way
		 * whether the syscall has failed or not.
		 */
		temporarily_clear_syserror(tcp);

		/* timeout */
		print_ts(tcp, tcp->u_arg[4]);

		if (has_sig) {
			tprint_arg_next();

			/* sig */
			print_kernel_sigset(tcp, tcp->u_arg[5]);
		}

		restore_cleared_syserror(tcp);
	}
	return 0;
}

#if HAVE_ARCH_TIME32_SYSCALLS
SYS_FUNC(io_getevents_time32)
{
	return print_io_getevents(tcp, print_timespec32, false);
}
#endif

#if HAVE_ARCH_OLD_TIME64_SYSCALLS
SYS_FUNC(io_getevents_time64)
{
	return print_io_getevents(tcp, print_timespec64, false);
}
#endif

#if HAVE_ARCH_TIME32_SYSCALLS
SYS_FUNC(io_pgetevents_time32)
{
	return print_io_getevents(tcp, print_timespec32, true);
}
#endif

SYS_FUNC(io_pgetevents_time64)
{
	return print_io_getevents(tcp, print_timespec64, true);
}
