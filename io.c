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
#include <fcntl.h>
#include <sys/uio.h>

SYS_FUNC(read)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		if (syserror(tcp))
			printaddr(tcp->u_arg[1]);
		else
			printstr(tcp, tcp->u_arg[1], tcp->u_rval);
		tprintf(", %lu", tcp->u_arg[2]);
	}
	return 0;
}

SYS_FUNC(write)
{
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	printstr(tcp, tcp->u_arg[1], tcp->u_arg[2]);
	tprintf(", %lu", tcp->u_arg[2]);

	return RVAL_DECODED;
}

struct print_iovec_config {
	enum iov_decode decode_iov;
	unsigned long data_size;
};

static bool
print_iovec(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	const unsigned long *iov;
	unsigned long iov_buf[2], len;
	struct print_iovec_config *c = data;

        if (elem_size < sizeof(iov_buf)) {
		iov_buf[0] = ((unsigned int *) elem_buf)[0];
		iov_buf[1] = ((unsigned int *) elem_buf)[1];
		iov = iov_buf;
	} else {
		iov = elem_buf;
	}

	tprints("{iov_base=");

	len = iov[1];

	switch (c->decode_iov) {
		case IOV_DECODE_STR:
			if (len > c->data_size)
				len = c->data_size;
			if (c->data_size != (unsigned long) -1L)
				c->data_size -= len;
			printstr(tcp, iov[0], len);
			break;
		case IOV_DECODE_NETLINK:
			if (len > c->data_size)
				len = c->data_size;
			if (c->data_size != (unsigned long) -1L)
				c->data_size -= len;
			decode_netlink(tcp, iov[0], iov[1]);
			break;
		default:
			printaddr(iov[0]);
			break;
	}

	tprintf(", iov_len=%lu}", iov[1]);

	return true;
}

/*
 * data_size limits the cumulative size of printed data.
 * Example: recvmsg returing a short read.
 */
void
tprint_iov_upto(struct tcb *tcp, unsigned long len, unsigned long addr,
		enum iov_decode decode_iov, unsigned long data_size)
{
	unsigned long iov[2];
	struct print_iovec_config config =
		{ .decode_iov = decode_iov, .data_size = data_size };

	print_array(tcp, addr, len, iov, current_wordsize * 2,
		    umoven_or_printaddr, print_iovec, &config);
}

void
tprint_iov(struct tcb *tcp, unsigned long len, unsigned long addr,
	   enum iov_decode decode_iov)
{
	tprint_iov_upto(tcp, len, addr, decode_iov, (unsigned long) -1L);
}

SYS_FUNC(readv)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		tprint_iov_upto(tcp, tcp->u_arg[2], tcp->u_arg[1],
				IOV_DECODE_STR, tcp->u_rval);
		tprintf(", %lu", tcp->u_arg[2]);
	}
	return 0;
}

SYS_FUNC(writev)
{
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	tprint_iov(tcp, tcp->u_arg[2], tcp->u_arg[1], IOV_DECODE_STR);
	tprintf(", %lu", tcp->u_arg[2]);

	return RVAL_DECODED;
}

SYS_FUNC(pread)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		if (syserror(tcp))
			printaddr(tcp->u_arg[1]);
		else
			printstr(tcp, tcp->u_arg[1], tcp->u_rval);
		tprintf(", %lu, ", tcp->u_arg[2]);
		printllval(tcp, "%lld", 3);
	}
	return 0;
}

SYS_FUNC(pwrite)
{
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	printstr(tcp, tcp->u_arg[1], tcp->u_arg[2]);
	tprintf(", %lu, ", tcp->u_arg[2]);
	printllval(tcp, "%lld", 3);

	return RVAL_DECODED;
}

static void
print_lld_from_low_high_val(struct tcb *tcp, int arg)
{
#if SIZEOF_LONG > 4 && SIZEOF_LONG == SIZEOF_LONG_LONG
# if SUPPORTED_PERSONALITIES > 1
#  ifdef X86_64
	if (current_personality != 1)
#  else
	if (current_wordsize == sizeof(long))
#  endif
# endif
		tprintf("%ld", tcp->u_arg[arg]);
# if SUPPORTED_PERSONALITIES > 1
	else
		tprintf("%ld",
			((unsigned long) tcp->u_arg[arg + 1] << current_wordsize * 8)
			| (unsigned long) tcp->u_arg[arg]);
# endif
#elif SIZEOF_LONG > 4
# error Unsupported configuration: SIZEOF_LONG > 4 && SIZEOF_LONG_LONG > SIZEOF_LONG
#elif HAVE_STRUCT_TCB_EXT_ARG
# if SUPPORTED_PERSONALITIES > 1
	if (current_personality == 1) {
		tprintf("%lld",
			(zero_extend_signed_to_ull(tcp->u_arg[arg + 1]) << sizeof(long) * 8)
			| zero_extend_signed_to_ull(tcp->u_arg[arg]));
	} else
# endif
	{
		tprintf("%lld", tcp->ext_arg[arg]);
	}
#else /* SIZEOF_LONG_LONG > SIZEOF_LONG && !HAVE_STRUCT_TCB_EXT_ARG */
	tprintf("%lld",
		(zero_extend_signed_to_ull(tcp->u_arg[arg + 1]) << sizeof(long) * 8)
		| zero_extend_signed_to_ull(tcp->u_arg[arg]));
#endif
}

#include "xlat/rwf_flags.h"

static int
do_preadv(struct tcb *tcp, const int flags_arg)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		tprint_iov_upto(tcp, tcp->u_arg[2], tcp->u_arg[1], IOV_DECODE_STR,
				tcp->u_rval);
		tprintf(", %lu, ", tcp->u_arg[2]);
		print_lld_from_low_high_val(tcp, 3);
		if (flags_arg >= 0) {
			tprints(", ");
			printflags(rwf_flags, tcp->u_arg[flags_arg], "RWF_???");
		}
	}
	return 0;
}

SYS_FUNC(preadv)
{
	return do_preadv(tcp, -1);
}

SYS_FUNC(preadv2)
{
	return do_preadv(tcp, 5);
}

static int
do_pwritev(struct tcb *tcp, const int flags_arg)
{
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	tprint_iov(tcp, tcp->u_arg[2], tcp->u_arg[1], IOV_DECODE_STR);
	tprintf(", %lu, ", tcp->u_arg[2]);
	print_lld_from_low_high_val(tcp, 3);
	if (flags_arg >= 0) {
		tprints(", ");
		printflags(rwf_flags, tcp->u_arg[flags_arg], "RWF_???");
	}

	return RVAL_DECODED;
}

SYS_FUNC(pwritev)
{
	return do_pwritev(tcp, -1);
}

SYS_FUNC(pwritev2)
{
	return do_pwritev(tcp, 5);
}

#include "xlat/splice_flags.h"

SYS_FUNC(tee)
{
	/* int fd_in */
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	/* int fd_out */
	printfd(tcp, tcp->u_arg[1]);
	tprints(", ");
	/* size_t len */
	tprintf("%lu, ", tcp->u_arg[2]);
	/* unsigned int flags */
	printflags(splice_flags, tcp->u_arg[3], "SPLICE_F_???");

	return RVAL_DECODED;
}

SYS_FUNC(splice)
{
	/* int fd_in */
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	/* loff_t *off_in */
	printnum_int64(tcp, tcp->u_arg[1], "%" PRId64);
	tprints(", ");
	/* int fd_out */
	printfd(tcp, tcp->u_arg[2]);
	tprints(", ");
	/* loff_t *off_out */
	printnum_int64(tcp, tcp->u_arg[3], "%" PRId64);
	tprints(", ");
	/* size_t len */
	tprintf("%lu, ", tcp->u_arg[4]);
	/* unsigned int flags */
	printflags(splice_flags, tcp->u_arg[5], "SPLICE_F_???");

	return RVAL_DECODED;
}

SYS_FUNC(vmsplice)
{
	/* int fd */
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	/* const struct iovec *iov, unsigned long nr_segs */
	tprint_iov(tcp, tcp->u_arg[2], tcp->u_arg[1], IOV_DECODE_STR);
	tprintf(", %lu, ", tcp->u_arg[2]);
	/* unsigned int flags */
	printflags(splice_flags, tcp->u_arg[3], "SPLICE_F_???");

	return RVAL_DECODED;
}
