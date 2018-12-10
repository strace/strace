/*
 * Copyright (c) 2001-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
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
