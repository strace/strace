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
 *
 *	$Id$
 */

#include "defs.h"

#include <fcntl.h>
#if HAVE_SYS_UIO_H
#include <sys/uio.h>
#endif

#ifdef HAVE_LONG_LONG_OFF_T
/*
 * Hacks for systems that have a long long off_t
 */

#define sys_pread64	sys_pread
#define sys_pwrite64	sys_pwrite
#endif

int
sys_read(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
	} else {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else
			printstr(tcp, tcp->u_arg[1], tcp->u_rval);
		tprintf(", %lu", tcp->u_arg[2]);
	}
	return 0;
}

int
sys_write(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
		printstr(tcp, tcp->u_arg[1], tcp->u_arg[2]);
		tprintf(", %lu", tcp->u_arg[2]);
	}
	return 0;
}

#if HAVE_SYS_UIO_H
void
tprint_iov(tcp, len, addr)
struct tcb * tcp;
int len;
long addr;
{
	struct iovec *iov;
	int i;


	if (!len) {
		tprintf("[]");
		return;
	}
	  
	if ((iov = (struct iovec *) malloc(len * sizeof *iov)) == NULL) {
		fprintf(stderr, "No memory");
		return;
	}
	if (umoven(tcp, addr,
		   len * sizeof *iov, (char *) iov) < 0) {
		tprintf("%#lx", tcp->u_arg[1]);
	} else {
		tprintf("[");
		for (i = 0; i < len; i++) {
			if (i)
				tprintf(", ");
			tprintf("{");
			printstr(tcp, (long) iov[i].iov_base,
				iov[i].iov_len);
			tprintf(", %lu}", (unsigned long)iov[i].iov_len);
		}
		tprintf("]");
	}
	free((char *) iov);
}

int
sys_readv(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
	} else {
		if (syserror(tcp)) {
			tprintf("%#lx, %lu",
					tcp->u_arg[1], tcp->u_arg[2]);
			return 0;
		}
		tprint_iov(tcp, tcp->u_arg[2], tcp->u_arg[1]);
		tprintf(", %lu", tcp->u_arg[2]);
	}
	return 0;
}

int
sys_writev(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
		tprint_iov(tcp, tcp->u_arg[2], tcp->u_arg[1]);
		tprintf(", %lu", tcp->u_arg[2]);
	}
	return 0;
}
#endif

#if defined(SVR4)

int
sys_pread(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
	} else {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else
			printstr(tcp, tcp->u_arg[1], tcp->u_rval);
#if UNIXWARE
		/* off_t is signed int */
		tprintf(", %lu, %ld", tcp->u_arg[2], tcp->u_arg[3]);
#else
		tprintf(", %lu, %llu", tcp->u_arg[2],
				(((unsigned long long) tcp->u_arg[4]) << 32
				 | tcp->u_arg[3]));
#endif
	}
	return 0;
}

int
sys_pwrite(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
		printstr(tcp, tcp->u_arg[1], tcp->u_arg[2]);
#if UNIXWARE
		/* off_t is signed int */
		tprintf(", %lu, %ld", tcp->u_arg[2], tcp->u_arg[3]);
#else
		tprintf(", %lu, %llu", tcp->u_arg[2],
				(((unsigned long long) tcp->u_arg[4]) << 32
				 | tcp->u_arg[3]));
#endif
	}
	return 0;
}
#endif /* SVR4 */

#ifdef FREEBSD
#include <sys/types.h>
#include <sys/socket.h>

int
sys_sendfile(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, %ld, %llu, %lu", tcp->u_arg[0], tcp->u_arg[1],
			(((unsigned long long) tcp->u_arg[3]) << 32 |
			 tcp->u_arg[2]), tcp->u_arg[4]);
	} else {
		off_t offset;

		if (!tcp->u_arg[5])
			tprintf(", NULL");
		else {
			struct sf_hdtr hdtr;

			if (umove(tcp, tcp->u_arg[5], &hdtr) < 0)
				tprintf(", %#lx", tcp->u_arg[5]);
			else {
				tprintf(", { ");
				tprint_iov(tcp, hdtr.hdr_cnt, hdtr.headers);
				tprintf(", %u, ", hdtr.hdr_cnt);
				tprint_iov(tcp, hdtr.trl_cnt, hdtr.trailers);
				tprintf(", %u }", hdtr.hdr_cnt);
			}
		}
		if (!tcp->u_arg[6])
			tprintf(", NULL");
		else if (umove(tcp, tcp->u_arg[6], &offset) < 0)
			tprintf(", %#lx", tcp->u_arg[6]);
		else
			tprintf(", [%llu]", offset);
		tprintf(", %lu", tcp->u_arg[7]);
	}
	return 0;
}
#endif /* FREEBSD */

#ifdef LINUX
int
sys_pread(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
	} else {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else
			printstr(tcp, tcp->u_arg[1], tcp->u_rval);
		tprintf(", %lu, %llu", tcp->u_arg[2],
			*(unsigned long long *)&tcp->u_arg[3]);
	}
	return 0;
}

int
sys_pwrite(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
		printstr(tcp, tcp->u_arg[1], tcp->u_arg[2]);
		tprintf(", %lu, %llu", tcp->u_arg[2],
			*(unsigned long long *)&tcp->u_arg[3]);
	}
	return 0;
}

int
sys_sendfile(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		off_t offset;

		tprintf("%ld, %ld, ", tcp->u_arg[0], tcp->u_arg[1]);
		if (!tcp->u_arg[2])
			tprintf("NULL");
		else if (umove(tcp, tcp->u_arg[2], &offset) < 0)
			tprintf("%#lx", tcp->u_arg[2]);
		else
			tprintf("[%lu]", offset);
		tprintf(", %lu", tcp->u_arg[3]);
	}
	return 0;
}

#endif /* LINUX */

#if _LFS64_LARGEFILE || HAVE_LONG_LONG_OFF_T
int
sys_pread64(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
	} else {
		ALIGN64 (tcp, 3);
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else
			printstr(tcp, tcp->u_arg[1], tcp->u_rval);
		tprintf(", %lu, %#llx", tcp->u_arg[2],
			LONG_LONG(tcp->u_arg[3], tcp->u_arg[4]));
	}
	return 0;
}

int
sys_pwrite64(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		ALIGN64 (tcp, 3);
		tprintf("%ld, ", tcp->u_arg[0]);
		printstr(tcp, tcp->u_arg[1], tcp->u_arg[2]);
		tprintf(", %lu, %#llx", tcp->u_arg[2],
			LONG_LONG(tcp->u_arg[3], tcp->u_arg[4]));
	}
	return 0;
}
#endif
 
int
sys_ioctl(tcp)
struct tcb *tcp;
{
	char *symbol;

	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
		symbol = ioctl_lookup(tcp->u_arg[1]);
		if (symbol)
			tprintf("%s", symbol);
		else
			tprintf("%#lx", tcp->u_arg[1]);
		ioctl_decode(tcp, tcp->u_arg[1], tcp->u_arg[2]);
	}
	else {
		if (ioctl_decode(tcp, tcp->u_arg[1], tcp->u_arg[2]) == 0)
			tprintf(", %#lx", tcp->u_arg[2]);
	}
	return 0;
}
