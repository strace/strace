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
#include <fcntl.h>
#include <sys/uio.h>

SYS_FUNC(read)
{
	if (entering(tcp)) {
		/* fd */
		printfd(tcp, tcp->u_arg[0]);
		tprint_arg_next();
	} else {
		/* buf */
		if (syserror(tcp))
			printaddr(tcp->u_arg[1]);
		else
			printstrn(tcp, tcp->u_arg[1], tcp->u_rval);
		tprint_arg_next();

		/* count */
		PRINT_VAL_U(tcp->u_arg[2]);
	}
	return 0;
}

SYS_FUNC(write)
{
	/* fd */
	printfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* buf */
	printstrn(tcp, tcp->u_arg[1], tcp->u_arg[2]);
	tprint_arg_next();

	/* count */
	PRINT_VAL_U(tcp->u_arg[2]);

	return RVAL_DECODED;
}

void
iov_decode_addr(struct tcb *tcp, kernel_ulong_t addr, kernel_ulong_t size,
	       void *opaque_data)
{
	printaddr(addr);
}

void
iov_decode_str(struct tcb *tcp, kernel_ulong_t addr, kernel_ulong_t size,
	       void *opaque_data)
{
	printstrn(tcp, addr, size);
}

struct print_iovec_config {
	kernel_ulong_t data_size;
	print_obj_by_addr_size_fn print_func;
	void *opaque_data;
};

static bool
print_iovec_klong(struct tcb *const tcp,
		  const kernel_ulong_t iov_base,
		  const kernel_ulong_t iov_len,
		  struct print_iovec_config *const c)
{
	kernel_ulong_t len = iov_len;

	if (len > c->data_size)
		len = c->data_size;
	if (c->data_size != (kernel_ulong_t) -1)
		c->data_size -= len;

	tprint_struct_begin();
	tprints_field_name("iov_base");
	c->print_func(tcp, iov_base, len, c->opaque_data);
	tprint_struct_next();

	tprints_field_name("iov_len");
	PRINT_VAL_U(iov_len);
	tprint_struct_end();

	return true;
}

#if ANY_WORDSIZE_LESS_THAN_KERNEL_LONG
static bool
print_iovec_elem_int(struct tcb *tcp, void *elem_buf, size_t elem_size,
		     void *data)
{
	const unsigned int *const iov = elem_buf;
	return print_iovec_klong(tcp, iov[0], iov[1], data);
}
#endif

#if ANY_WORDSIZE_EQUALS_TO_KERNEL_LONG
static bool
print_iovec_elem_klong(struct tcb *tcp, void *elem_buf, size_t elem_size,
		       void *data)
{
	const kernel_ulong_t *const iov = elem_buf;
	return print_iovec_klong(tcp, iov[0], iov[1], data);
}
#endif

/*
 * data_size limits the cumulative size of printed data.
 * Example: recvmsg returning a short read.
 */
void
tprint_iov_upto(struct tcb *const tcp, const kernel_ulong_t len,
		const kernel_ulong_t addr,
		const kernel_ulong_t data_size,
		print_obj_by_addr_size_fn print_func,
		void *opaque_data)
{
	kernel_ulong_t iov[2];
	struct print_iovec_config config = {
		.data_size = data_size,
		.print_func = print_func,
		.opaque_data = opaque_data
	};
	const print_fn print_elem_func =
#if !ANY_WORDSIZE_EQUALS_TO_KERNEL_LONG
			print_iovec_elem_int;
#elif !ANY_WORDSIZE_LESS_THAN_KERNEL_LONG
			print_iovec_elem_klong;
#else
		current_wordsize < sizeof(kernel_ulong_t) ?
			print_iovec_elem_int :
			print_iovec_elem_klong;
#endif

	print_array(tcp, addr, len, iov, current_wordsize * 2,
		    tfetch_mem_ignore_syserror, print_elem_func, &config);
}

SYS_FUNC(readv)
{
	if (entering(tcp)) {
		/* fd */
		printfd(tcp, tcp->u_arg[0]);
		tprint_arg_next();
	} else {
		/* iov */
		tprint_iov_upto(tcp, tcp->u_arg[2], tcp->u_arg[1], tcp->u_rval,
				syserror(tcp) ? iov_decode_addr
					      : iov_decode_str,
				NULL);
		tprint_arg_next();

		/* iovcnt */
		PRINT_VAL_U(tcp->u_arg[2]);
	}
	return 0;
}

SYS_FUNC(writev)
{
	/* fd */
	printfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* iov */
	tprint_iov(tcp, tcp->u_arg[2], tcp->u_arg[1], iov_decode_str);
	tprint_arg_next();

	/* iovcnt */
	PRINT_VAL_U(tcp->u_arg[2]);

	return RVAL_DECODED;
}

SYS_FUNC(pread)
{
	if (entering(tcp)) {
		/* fd */
		printfd(tcp, tcp->u_arg[0]);
		tprint_arg_next();
	} else {
		/* buf */
		if (syserror(tcp))
			printaddr(tcp->u_arg[1]);
		else
			printstrn(tcp, tcp->u_arg[1], tcp->u_rval);
		tprint_arg_next();

		/* count */
		PRINT_VAL_U(tcp->u_arg[2]);
		tprint_arg_next();

		/* offset */
		print_arg_lld(tcp, 3);
	}
	return 0;
}

SYS_FUNC(pwrite)
{
	/* fd */
	printfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* buf */
	printstrn(tcp, tcp->u_arg[1], tcp->u_arg[2]);
	tprint_arg_next();

	/* count */
	PRINT_VAL_U(tcp->u_arg[2]);
	tprint_arg_next();

	/* offset */
	print_arg_lld(tcp, 3);

	return RVAL_DECODED;
}

static void
print_lld_from_low_high_val(struct tcb *tcp, int arg)
{
#if SIZEOF_KERNEL_LONG_T > 4
# ifndef current_klongsize
	if (current_klongsize < SIZEOF_KERNEL_LONG_T)
		PRINT_VAL_D((tcp->u_arg[arg + 1] << 32) | tcp->u_arg[arg]);
	else
# endif /* !current_klongsize */
		PRINT_VAL_D(tcp->u_arg[arg]);
#else /* SIZEOF_KERNEL_LONG_T == 4 */
	PRINT_VAL_D(((unsigned long long) tcp->u_arg[arg + 1] << 32) |
		    ((unsigned long long) tcp->u_arg[arg]));
#endif
}

#include "xlat/rwf_flags.h"

static int
do_preadv(struct tcb *tcp, const int flags_arg)
{
	if (entering(tcp)) {
		/* fd */
		printfd(tcp, tcp->u_arg[0]);
		tprint_arg_next();
	} else {
		kernel_ulong_t len =
			truncate_kulong_to_current_wordsize(tcp->u_arg[2]);

		/* iov */
		tprint_iov_upto(tcp, len, tcp->u_arg[1], tcp->u_rval,
				syserror(tcp) ? iov_decode_addr
					      : iov_decode_str,
				NULL);
		tprint_arg_next();

		/* iovcnt */
		PRINT_VAL_U(len);
		tprint_arg_next();

		/* offset */
		print_lld_from_low_high_val(tcp, 3);

		if (flags_arg >= 0) {
			tprint_arg_next();

			/* flags */
			printflags(rwf_flags, tcp->u_arg[flags_arg], "RWF_???");
		}
	}
	return 0;
}

SYS_FUNC(preadv)
{
	return do_preadv(tcp, -1);
}

static int
do_pwritev(struct tcb *tcp, const int flags_arg)
{
	kernel_ulong_t len =
		truncate_kulong_to_current_wordsize(tcp->u_arg[2]);

	/* fd */
	printfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* iov */
	tprint_iov(tcp, len, tcp->u_arg[1], iov_decode_str);
	tprint_arg_next();

	/* iovcnt */
	PRINT_VAL_U(len);
	tprint_arg_next();

	/* offset */
	print_lld_from_low_high_val(tcp, 3);

	if (flags_arg >= 0) {
		tprint_arg_next();

		/* flags */
		printflags(rwf_flags, tcp->u_arg[flags_arg], "RWF_???");
	}

	return RVAL_DECODED;
}

SYS_FUNC(pwritev)
{
	return do_pwritev(tcp, -1);
}

/*
 * x32 is the only architecture where preadv2 takes 5 arguments
 * instead of 6, see preadv64v2 in kernel sources.
 * Likewise, x32 is the only architecture where pwritev2 takes 5 arguments
 * instead of 6, see pwritev64v2 in kernel sources.
 */

#if defined X86_64
# define PREADV2_PWRITEV2_FLAGS_ARG_NO (current_personality == 2 ? 4 : 5)
#elif defined X32
# define PREADV2_PWRITEV2_FLAGS_ARG_NO (current_personality == 0 ? 4 : 5)
#else
# define PREADV2_PWRITEV2_FLAGS_ARG_NO 5
#endif

SYS_FUNC(preadv2)
{
	return do_preadv(tcp, PREADV2_PWRITEV2_FLAGS_ARG_NO);
}

SYS_FUNC(pwritev2)
{
	return do_pwritev(tcp, PREADV2_PWRITEV2_FLAGS_ARG_NO);
}

#include "xlat/splice_flags.h"

SYS_FUNC(tee)
{
	/* int fd_in */
	printfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* int fd_out */
	printfd(tcp, tcp->u_arg[1]);
	tprint_arg_next();

	/* size_t len */
	PRINT_VAL_U(tcp->u_arg[2]);
	tprint_arg_next();

	/* unsigned int flags */
	printflags(splice_flags, tcp->u_arg[3], "SPLICE_F_???");

	return RVAL_DECODED;
}

SYS_FUNC(splice)
{
	/* int fd_in */
	printfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* loff_t *off_in */
	printnum_int64(tcp, tcp->u_arg[1], "%" PRId64);
	tprint_arg_next();

	/* int fd_out */
	printfd(tcp, tcp->u_arg[2]);
	tprint_arg_next();

	/* loff_t *off_out */
	printnum_int64(tcp, tcp->u_arg[3], "%" PRId64);
	tprint_arg_next();

	/* size_t len */
	PRINT_VAL_U(tcp->u_arg[4]);
	tprint_arg_next();

	/* unsigned int flags */
	printflags(splice_flags, tcp->u_arg[5], "SPLICE_F_???");

	return RVAL_DECODED;
}

SYS_FUNC(vmsplice)
{
	/* int fd */
	printfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* const struct iovec *iov */
	tprint_iov(tcp, tcp->u_arg[2], tcp->u_arg[1], iov_decode_str);
	tprint_arg_next();

	/* unsigned long nr_segs */
	PRINT_VAL_U(tcp->u_arg[2]);
	tprint_arg_next();

	/* unsigned int flags */
	printflags(splice_flags, tcp->u_arg[3], "SPLICE_F_???");

	return RVAL_DECODED;
}
