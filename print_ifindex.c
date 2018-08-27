/*
 * Copyright (c) 2001-2018 The strace developers.
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
#include <net/if.h>

#ifdef HAVE_IF_INDEXTONAME
# include "xstring.h"

# define INI_PFX "if_nametoindex(\""
# define INI_SFX "\")"
# define IFNAME_QUOTED_SZ (sizeof(IFNAMSIZ) * 4 + 3)

const char *
get_ifname(const unsigned int ifindex)
{
	static char name_quoted_buf[IFNAME_QUOTED_SZ];
	char name_buf[IFNAMSIZ];

	if (!if_indextoname(ifindex, name_buf))
		return NULL;

	if (string_quote(name_buf, name_quoted_buf, sizeof(name_buf),
			 QUOTE_0_TERMINATED | QUOTE_OMIT_LEADING_TRAILING_QUOTES,
			 NULL))
		return NULL;

	return name_quoted_buf;
}

static const char *
sprint_ifname(const unsigned int ifindex)
{
	static char res[IFNAME_QUOTED_SZ + sizeof(INI_PFX INI_SFX)];

	const char *name_quoted = get_ifname(ifindex);

	if (!name_quoted)
		return NULL;

	xsprintf(res, INI_PFX "%s" INI_SFX, name_quoted);

	return res;
}

#else /* !HAVE_IF_INDEXTONAME */

const char *get_ifname(const unsigned int ifindex) { return NULL; }

#endif /* HAVE_IF_INDEXTONAME */

void
print_ifindex(const unsigned int ifindex)
{
#ifdef HAVE_IF_INDEXTONAME
	print_xlat_ex(ifindex, sprint_ifname(ifindex), XLAT_STYLE_FMT_U);
#else
	tprintf("%u", ifindex);
#endif
}
