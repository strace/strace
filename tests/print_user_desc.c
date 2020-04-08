/*
 * Auxiliary printing function for the struct user_desc type.
 * Used by modify_ldt and xet_thread_area tests.
 *
 * Copyright (c) 2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#ifdef HAVE_STRUCT_USER_DESC

# include <asm/ldt.h>

/**
 * Print user_desc structure.
 *
 * @param us        Pointer to struct user_desc to print.
 * @param entry_str If not NULL, the string is printed as a value of
 *                  entry_number field.
 */
static void
print_user_desc(struct user_desc *us, const char *entry_str)
{
	if (entry_str)
		printf("{entry_number=%s", entry_str);
	else
		printf("{entry_number=%u", us->entry_number);

	printf(", base_addr=%#08x"
	       ", limit=%#08x"
	       ", seg_32bit=%u"
	       ", contents=%u"
	       ", read_exec_only=%u"
	       ", limit_in_pages=%u"
	       ", seg_not_present=%u"
	       ", useable=%u"
# ifdef __x86_64__
	       ", lm=%u"
# endif
	       "}",
	       us->base_addr,
	       us->limit,
	       us->seg_32bit,
	       us->contents,
	       us->read_exec_only,
	       us->limit_in_pages,
	       us->seg_not_present,
	       us->useable
# ifdef __x86_64__
	       , us->lm
# endif
	       );
}

#endif /* HAVE_STRUCT_USER_DESC */
