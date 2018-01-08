/*
 * Auxiliary printing function for the struct user_desc type.
 * Used by modify_ldt and xet_thread_area tests.
 *
 * Copyright (c) 2018 The strace developers.
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
