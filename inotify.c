/*
 * Copyright (c) 2006 Bernhard Kaindl <bk@suse.de>
 * Copyright (c) 2006-2015 Dmitry V. Levin <ldv@altlinux.org>
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

#include "print_fields.h"

#include "xlat/inotify_flags.h"
#include "xlat/inotify_init_flags.h"

bool
decode_inotify_read(struct tcb *tcp, int fd, const char *fdpath,
		    enum fileops op, kernel_ulong_t addr,
		    kernel_ulong_t addrlen)
{
	struct iev_hdr {
		int32_t	 wd;
		uint32_t mask;
		uint32_t cookie;
		uint32_t len;
	} iev_hdr;
	kernel_ulong_t pos = 0;

	if (addrlen < sizeof(iev_hdr))
		return false;

	tprints("[");

	do {
		if (pos)
			tprints(", ");

		if (umove(tcp, addr + pos, &iev_hdr)) {
			printaddr_comment(addr + pos);
			break;
		}

		PRINT_FIELD_D("{", iev_hdr, wd);
		PRINT_FIELD_FLAGS(", ", iev_hdr, mask, inotify_flags, "IN_???");
		PRINT_FIELD_U(", ", iev_hdr, cookie);
		PRINT_FIELD_U(", ", iev_hdr, len);

		pos += sizeof(iev_hdr);

		if (iev_hdr.len) {
			tprints(", name=");
			printstrn(tcp, addr + pos, iev_hdr.len);

			pos += iev_hdr.len;
		}

		tprints("}");
	} while (pos <= addrlen - sizeof(iev_hdr));

	if (pos < addrlen) {
		if (pos)
			tprints(", ");

		printstrn(tcp, addr + pos, addrlen - pos);
	}

	tprints("]");

	return true;
}

SYS_FUNC(inotify_add_watch)
{
	/* file descriptor */
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	/* pathname */
	printpath(tcp, tcp->u_arg[1]);
	tprints(", ");
	/* mask */
	printflags(inotify_flags, tcp->u_arg[2], "IN_???");

	return RVAL_DECODED;
}

SYS_FUNC(inotify_rm_watch)
{
	/* file descriptor */
	printfd(tcp, tcp->u_arg[0]);
	/* watch descriptor */
	tprintf(", %d", (int) tcp->u_arg[1]);

	return RVAL_DECODED;
}

SYS_FUNC(inotify_init)
{
	return RVAL_DECODED | RVAL_FD;
}

SYS_FUNC(inotify_init1)
{
	printflags(inotify_init_flags, tcp->u_arg[0], "IN_???");

	return RVAL_DECODED | RVAL_FD;
}
