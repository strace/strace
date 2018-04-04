/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993-1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2012 Denys Vlasenko <vda.linux@googlemail.com>
 * Copyright (c) 2012-2017 The strace developers.
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
#include <sys/stat.h>

#include "xlat/modetypes.h"

void
print_symbolic_mode_t(const unsigned int mode)
{
	const char *ifmt = "";

	if (mode & S_IFMT)
		ifmt = xlookup(modetypes, mode & S_IFMT);

	if (!ifmt || xlat_verbose(xlat_verbosity) != XLAT_STYLE_ABBREV)
		tprintf("%#03o", mode);

	if (!ifmt || xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW)
		return;

	(xlat_verbose(xlat_verbosity) == XLAT_STYLE_ABBREV
		? tprintf : tprintf_comment)("%s%s%s%s%s%#03o",
			ifmt, ifmt[0] ? "|" : "",
			(mode & S_ISUID) ? "S_ISUID|" : "",
			(mode & S_ISGID) ? "S_ISGID|" : "",
			(mode & S_ISVTX) ? "S_ISVTX|" : "",
			mode & ~(S_IFMT|S_ISUID|S_ISGID|S_ISVTX));
}

void
print_numeric_umode_t(const unsigned short mode)
{
	tprintf("%#03ho", mode);
}

void
print_numeric_long_umask(const unsigned long mode)
{
	tprintf("%#03lo", mode);
}
