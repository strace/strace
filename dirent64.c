/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2005-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
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
			print_quoted_cstring(d->d_name, d_name_len);

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
