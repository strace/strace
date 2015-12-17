/*
 * Copyright (c) 2002-2005 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2004 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2005-2015 Dmitry V. Levin <ldv@altlinux.org>
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

#ifdef HAVE_SYS_XATTR_H
# include <sys/xattr.h>
#endif

#include "xlat/xattrflags.h"

static void
print_xattr_val(struct tcb *tcp,
		unsigned long addr,
		unsigned long insize,
		unsigned long size)
{
	char *buf = NULL;
	unsigned int len;

	tprints(", ");

	if (insize == 0)
		goto done;

	len = size;
	if (size != (unsigned long) len)
		goto done;

	if (!len) {
		tprintf("\"\", %ld", insize);
		return;
	}

	if (!verbose(tcp) || (exiting(tcp) && syserror(tcp)))
		goto done;

	buf = malloc(len);
	if (!buf)
		goto done;

	if (umoven(tcp, addr, len, buf) < 0) {
		free(buf);
		buf = NULL;
		goto done;
	}

	/* Don't print terminating NUL if there is one. */
	if (buf[len - 1] == '\0')
		--len;

done:
	if (buf) {
		print_quoted_string(buf, len, 0);
		free(buf);
	} else {
		printaddr(addr);
	}
	tprintf(", %ld", insize);
}

SYS_FUNC(setxattr)
{
	printpath(tcp, tcp->u_arg[0]);
	tprints(", ");
	printstr(tcp, tcp->u_arg[1], -1);
	print_xattr_val(tcp, tcp->u_arg[2], tcp->u_arg[3], tcp->u_arg[3]);
	tprints(", ");
	printflags(xattrflags, tcp->u_arg[4], "XATTR_???");
	return RVAL_DECODED;
}

SYS_FUNC(fsetxattr)
{
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	printstr(tcp, tcp->u_arg[1], -1);
	print_xattr_val(tcp, tcp->u_arg[2], tcp->u_arg[3], tcp->u_arg[3]);
	tprints(", ");
	printflags(xattrflags, tcp->u_arg[4], "XATTR_???");
	return RVAL_DECODED;
}

SYS_FUNC(getxattr)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprints(", ");
		printstr(tcp, tcp->u_arg[1], -1);
	} else {
		print_xattr_val(tcp, tcp->u_arg[2], tcp->u_arg[3], tcp->u_rval);
	}
	return 0;
}

SYS_FUNC(fgetxattr)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		printstr(tcp, tcp->u_arg[1], -1);
	} else {
		print_xattr_val(tcp, tcp->u_arg[2], tcp->u_arg[3], tcp->u_rval);
	}
	return 0;
}

static void
print_xattr_list(struct tcb *tcp, unsigned long addr, unsigned long size)
{
	if (syserror(tcp)) {
		printaddr(addr);
	} else {
		unsigned long len =
			(size < (unsigned long) tcp->u_rval) ?
				size : (unsigned long) tcp->u_rval;
		printstr(tcp, addr, len);
	}
	tprintf(", %lu", size);
}

SYS_FUNC(listxattr)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		print_xattr_list(tcp, tcp->u_arg[1], tcp->u_arg[2]);
	}
	return 0;
}

SYS_FUNC(flistxattr)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		print_xattr_list(tcp, tcp->u_arg[1], tcp->u_arg[2]);
	}
	return 0;
}

SYS_FUNC(removexattr)
{
	printpath(tcp, tcp->u_arg[0]);
	tprints(", ");
	printstr(tcp, tcp->u_arg[1], -1);
	return RVAL_DECODED;
}

SYS_FUNC(fremovexattr)
{
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	printstr(tcp, tcp->u_arg[1], -1);
	return RVAL_DECODED;
}
