/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
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
#include <dirent.h>

#include "xlat/dirent_types.h"

#define D_NAME_LEN_MAX 256

SYS_FUNC(getdents64)
{
	/* the minimum size of a valid dirent64 structure */
	const unsigned int d_name_offset = offsetof(struct dirent64, d_name);

	unsigned int i, len, dents = 0;
	char *buf;

	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		return 0;
	}

	const unsigned int count = tcp->u_arg[2];

	if (syserror(tcp) || !verbose(tcp)) {
		tprints(", ");
		printaddr(tcp->u_arg[1]);
		tprintf(", %u", count);
		return 0;
	}

	/* Beware of insanely large or too small values in tcp->u_rval */
	if (tcp->u_rval > 1024*1024)
		len = 1024*1024;
	else if (tcp->u_rval < (int) d_name_offset)
		len = 0;
	else
		len = tcp->u_rval;

	if (len) {
		buf = malloc(len);
		if (!buf || umoven(tcp, tcp->u_arg[1], len, buf) < 0) {
			tprints(", ");
			printaddr(tcp->u_arg[1]);
			tprintf(", %u", count);
			free(buf);
			return 0;
		}
	} else {
		buf = NULL;
	}

	tprints(",");
	if (!abbrev(tcp))
		tprints(" [");
	for (i = 0; len && i <= len - d_name_offset; ) {
		struct dirent64 *d = (struct dirent64 *) &buf[i];
		if (!abbrev(tcp)) {
			int d_name_len;
			if (d->d_reclen >= d_name_offset
			    && i + d->d_reclen <= len) {
				d_name_len = d->d_reclen - d_name_offset;
			} else {
				d_name_len = len - i - d_name_offset;
			}
			if (d_name_len > D_NAME_LEN_MAX)
				d_name_len = D_NAME_LEN_MAX;

			tprintf("%s{d_ino=%" PRIu64 ", d_off=%" PRId64
				", d_reclen=%u, d_type=",
				i ? ", " : "",
				d->d_ino,
				d->d_off,
				d->d_reclen);
			printxval(dirent_types, d->d_type, "DT_???");

			tprints(", d_name=");
			if (print_quoted_string(d->d_name, d_name_len,
					        QUOTE_0_TERMINATED) > 0) {
				tprints("...");
			}

			tprints("}");
		}
		if (d->d_reclen < d_name_offset) {
			tprints_comment("d_reclen < offsetof(struct dirent64, d_name)");
			break;
		}
		i += d->d_reclen;
		dents++;
	}
	if (!abbrev(tcp))
		tprints("]");
	else
		tprintf_comment("%u entries", dents);
	tprintf(", %u", count);
	free(buf);
	return 0;
}
