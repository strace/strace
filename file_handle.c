/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
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

#include "xlat/name_to_handle_at_flags.h"

#ifndef MAX_HANDLE_SZ
# define MAX_HANDLE_SZ 128
#endif

typedef struct {
	unsigned int handle_bytes;
	int handle_type;
} file_handle_header;

SYS_FUNC(name_to_handle_at)
{
	file_handle_header h;
	const kernel_ulong_t addr = tcp->u_arg[2];

	if (entering(tcp)) {
		/* dirfd */
		print_dirfd(tcp, tcp->u_arg[0]);

		/* pathname */
		printpath(tcp, tcp->u_arg[1]);
		tprints(", ");

		/* handle */
		if (umove_or_printaddr(tcp, addr, &h)) {
			tprints(", ");

			/* mount_id */
			printaddr(tcp->u_arg[3]);
			tprints(", ");

			/* flags */
			printflags(name_to_handle_at_flags, tcp->u_arg[4],
				   "AT_???");

			return RVAL_DECODED;
		}
		tprintf("{handle_bytes=%u", h.handle_bytes);

		set_tcb_priv_ulong(tcp, h.handle_bytes);

		return 0;
	} else {
		unsigned int i = get_tcb_priv_ulong(tcp);

		if ((!syserror(tcp) || EOVERFLOW == tcp->u_error)
		    && !umove(tcp, addr, &h)) {
			unsigned char f_handle[MAX_HANDLE_SZ];

			if (i != h.handle_bytes)
				tprintf(" => %u", h.handle_bytes);
			if (!syserror(tcp)) {
				tprintf(", handle_type=%d", h.handle_type);
				if (h.handle_bytes > MAX_HANDLE_SZ)
					h.handle_bytes = MAX_HANDLE_SZ;
				if (!umoven(tcp, addr + sizeof(h), h.handle_bytes,
					    f_handle)) {
					tprints(", f_handle=0x");
					for (i = 0; i < h.handle_bytes; ++i)
						tprintf("%02x", f_handle[i]);
				}
			}
		}
		tprints("}, ");

		/* mount_id */
		printnum_int(tcp, tcp->u_arg[3], "%d");
		tprints(", ");

		/* flags */
		printflags(name_to_handle_at_flags, tcp->u_arg[4], "AT_???");
	}
	return 0;
}

SYS_FUNC(open_by_handle_at)
{
	file_handle_header h;
	const kernel_ulong_t addr = tcp->u_arg[1];

	/* mount_fd */
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");

	/* handle */
	if (!umove_or_printaddr(tcp, addr, &h)) {
		unsigned char f_handle[MAX_HANDLE_SZ];

		tprintf("{handle_bytes=%u, handle_type=%d",
			h.handle_bytes, h.handle_type);
		if (h.handle_bytes > MAX_HANDLE_SZ)
			h.handle_bytes = MAX_HANDLE_SZ;
		if (!umoven(tcp, addr + sizeof(h), h.handle_bytes, &f_handle)) {
			unsigned int i;

			tprints(", f_handle=0x");
			for (i = 0; i < h.handle_bytes; ++i)
				tprintf("%02x", f_handle[i]);
		}
		tprints("}");
	}
	tprints(", ");

	/* flags */
	tprint_open_modes(tcp->u_arg[2]);

	return RVAL_DECODED;
}
