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

/*
 * data_size limits the cumulative size of printed data.
 * Example: recvmsg returing a short read.
 */
void
tprint_iov_upto(struct tcb *tcp, unsigned long len, unsigned long addr, int decode_iov, unsigned long data_size)
{
	unsigned long iov[2];
	unsigned long size, cur, end, abbrev_end;
	const unsigned long sizeof_iov = current_wordsize * 2;

	if (!len) {
		tprints("[]");
		return;
	}
	size = len * sizeof_iov;
	end = addr + size;
	if (!verbose(tcp) || (exiting(tcp) && syserror(tcp)) ||
	    !addr || size / sizeof_iov != len || end < addr) {
		printaddr(addr);
		return;
	}
	if (abbrev(tcp)) {
		abbrev_end = addr + max_strlen * sizeof_iov;
		if (abbrev_end < addr)
			abbrev_end = end;
	} else {
		abbrev_end = end;
	}
	tprints("[");
	for (cur = addr; cur < end; cur += sizeof_iov) {
		if (cur > addr)
			tprints(", ");
		if (cur >= abbrev_end) {
			tprints("...");
			break;
		}
		if (umove_ulong_array_or_printaddr(tcp, cur, iov,
						   ARRAY_SIZE(iov)))
			break;
		tprints("{");
		if (decode_iov) {
			unsigned long len = iov[1];
			if (len > data_size)
				len = data_size;
			data_size -= len;
			printstr(tcp, iov[0], len);
		} else
			printaddr(iov[0]);
		tprintf(", %lu}", iov[1]);
	}
	tprints("]");
}

void
tprint_iov(struct tcb *tcp, unsigned long len, unsigned long addr, int decode_iov)
{
	tprint_iov_upto(tcp, len, addr, decode_iov, (unsigned long) -1L);
}

SYS_FUNC(readv)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		tprint_iov(tcp, tcp->u_arg[2], tcp->u_arg[1], 1);
		tprintf(", %lu", tcp->u_arg[2]);
	}
	return 0;
}

SYS_FUNC(writev)
{
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	tprint_iov(tcp, tcp->u_arg[2], tcp->u_arg[1], 1);
	tprintf(", %lu", tcp->u_arg[2]);

	return RVAL_DECODED;
}

/* The SH4 ABI does allow long longs in odd-numbered registers, but
   does not allow them to be split between registers and memory - and
   there are only four argument registers for normal functions.  As a
   result pread takes an extra padding argument before the offset.  This
   was changed late in the 2.4 series (around 2.4.20).  */
#if defined(SH)
#define PREAD_OFFSET_ARG 4
#else
#define PREAD_OFFSET_ARG 3
#endif

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
		printllval(tcp, "%llu", PREAD_OFFSET_ARG);
	}
	return 0;
}

SYS_FUNC(pwrite)
{
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	printstr(tcp, tcp->u_arg[1], tcp->u_arg[2]);
	tprintf(", %lu, ", tcp->u_arg[2]);
	printllval(tcp, "%llu", PREAD_OFFSET_ARG);

	return RVAL_DECODED;
}

static void
print_llu_from_low_high_val(struct tcb *tcp, int arg)
{
#if SIZEOF_LONG == SIZEOF_LONG_LONG
# if SUPPORTED_PERSONALITIES > 1
#  ifdef X86_64
	if (current_personality != 1)
#  else
	if (current_wordsize == sizeof(long))
#  endif
# endif
		tprintf("%lu", (unsigned long) tcp->u_arg[arg]);
# if SUPPORTED_PERSONALITIES > 1
	else
		tprintf("%lu",
			((unsigned long) tcp->u_arg[arg + 1] << current_wordsize * 8)
			| (unsigned long) tcp->u_arg[arg]);
# endif
#else
# ifdef X32
	if (current_personality == 0)
		tprintf("%llu", (unsigned long long) tcp->ext_arg[arg]);
	else
# endif
	tprintf("%llu",
		((unsigned long long) (unsigned long) tcp->u_arg[arg + 1] << sizeof(long) * 8)
		| (unsigned long long) (unsigned long) tcp->u_arg[arg]);
#endif
}

SYS_FUNC(preadv)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		tprint_iov(tcp, tcp->u_arg[2], tcp->u_arg[1], 1);
		tprintf(", %lu, ", tcp->u_arg[2]);
		print_llu_from_low_high_val(tcp, 3);
	}
	return 0;
}

SYS_FUNC(pwritev)
{
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	tprint_iov(tcp, tcp->u_arg[2], tcp->u_arg[1], 1);
	tprintf(", %lu, ", tcp->u_arg[2]);
	print_llu_from_low_high_val(tcp, 3);

	return RVAL_DECODED;
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
	printnum_int64(tcp, tcp->u_arg[1], "%" PRIu64);
	tprints(", ");
	/* int fd_out */
	printfd(tcp, tcp->u_arg[2]);
	tprints(", ");
	/* loff_t *off_out */
	printnum_int64(tcp, tcp->u_arg[3], "%" PRIu64);
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
	tprint_iov(tcp, tcp->u_arg[2], tcp->u_arg[1], 1);
	tprintf(", %lu, ", tcp->u_arg[2]);
	/* unsigned int flags */
	printflags(splice_flags, tcp->u_arg[3], "SPLICE_F_???");

	return RVAL_DECODED;
}
