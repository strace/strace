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

#include DEF_MPERS_TYPE(kernel_dirent)

#include MPERS_DEFS

#define D_NAME_LEN_MAX 256

static void
print_old_dirent(struct tcb *const tcp, const kernel_ulong_t addr)
{
	kernel_dirent d;

	if (umove_or_printaddr(tcp, addr, &d))
		return;

	tprintf("{d_ino=%llu, d_off=%llu, d_reclen=%u, d_name=",
		zero_extend_signed_to_ull(d.d_ino),
		zero_extend_signed_to_ull(d.d_off), d.d_reclen);
	if (d.d_reclen > D_NAME_LEN_MAX)
		d.d_reclen = D_NAME_LEN_MAX;
	printpathn(tcp, addr + offsetof(kernel_dirent, d_name), d.d_reclen);
	tprints("}");
}

SYS_FUNC(readdir)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		if (tcp->u_rval == 0)
			printaddr(tcp->u_arg[1]);
		else
			print_old_dirent(tcp, tcp->u_arg[1]);
		/* Not much point in printing this out, it is always 1. */
		if (tcp->u_arg[2] != 1)
			tprintf(", %" PRI_klu, tcp->u_arg[2]);
	}
	return 0;
}

SYS_FUNC(getdents)
{
	unsigned int i, len, dents = 0;
	unsigned char *buf;

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
	else if (tcp->u_rval < (int) sizeof(kernel_dirent))
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
	for (i = 0; len && i <= len - sizeof(kernel_dirent); ) {
		kernel_dirent *d = (kernel_dirent *) &buf[i];

		if (!abbrev(tcp)) {
			int oob = d->d_reclen < sizeof(kernel_dirent) ||
				  i + d->d_reclen - 1 >= len;
			int d_name_len = oob ? len - i : d->d_reclen;
			d_name_len -= offsetof(kernel_dirent, d_name) + 1;
			if (d_name_len > D_NAME_LEN_MAX)
				d_name_len = D_NAME_LEN_MAX;

			tprintf("%s{d_ino=%llu, d_off=%llu, d_reclen=%u"
				", d_name=", i ? ", " : "",
				zero_extend_signed_to_ull(d->d_ino),
				zero_extend_signed_to_ull(d->d_off),
				d->d_reclen);

			if (print_quoted_string(d->d_name, d_name_len,
					        QUOTE_0_TERMINATED) > 0) {
				tprints("...");
			}

			tprints(", d_type=");
			if (oob)
				tprints("?");
			else
				printxval(dirent_types, buf[i + d->d_reclen - 1], "DT_???");
			tprints("}");
		}
		dents++;
		if (d->d_reclen < sizeof(kernel_dirent)) {
			tprints_comment("d_reclen < sizeof(struct dirent)");
			break;
		}
		i += d->d_reclen;
	}
	if (!abbrev(tcp))
		tprints("]");
	else
		tprintf_comment("%u entries", dents);
	tprintf(", %u", count);
	free(buf);
	return 0;
}
