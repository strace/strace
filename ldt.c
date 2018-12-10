/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993-1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2002-2004 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2010 Andreas Schwab <schwab@linux-m68k.org>
 * Copyright (c) 2014-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2014-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#ifdef HAVE_STRUCT_USER_DESC

# include <asm/ldt.h>

# include "print_fields.h"
# include "xstring.h"

void
print_user_desc(struct tcb *const tcp, const kernel_ulong_t addr,
		enum user_desc_print_filter filter)
{
	struct user_desc desc;
	unsigned *entry_number = get_tcb_priv_data(tcp);

	switch (filter) {
	case USER_DESC_ENTERING:
		if (umove_or_printaddr(tcp, addr, &desc.entry_number))
			return;

		break;

	case USER_DESC_EXITING:
		if (!addr || !verbose(tcp))
			return;
		if (syserror(tcp) || umove(tcp, addr, &desc)) {
			if (entry_number)
				tprints(", ...}");

			return;
		}

		break;

	case USER_DESC_BOTH:
		if (umove_or_printaddr(tcp, addr, &desc))
			return;

		break;
	}

	if (filter & USER_DESC_ENTERING) {
		PRINT_FIELD_ID("{", desc, entry_number);

		/*
		 * If we don't print the whole structure now, let's save it for
		 * later.
		 */
		if (filter == USER_DESC_ENTERING) {
			entry_number = xmalloc(sizeof(*entry_number));

			*entry_number = desc.entry_number;
			set_tcb_priv_data(tcp, entry_number, free);
		}
	}

	if (filter & USER_DESC_EXITING) {
		/*
		 * It should be the same in case of get_thread_area, but we can
		 * never be sure...
		 */
		if (filter == USER_DESC_EXITING) {
			if (entry_number) {
				if (*entry_number != desc.entry_number) {
					if ((int) desc.entry_number == -1)
						tprints(" => -1");
					else
						tprintf(" => %u",
							desc.entry_number);
				}
			} else {
				/*
				 * This is really strange. If we are here, it
				 * means that we failed on entering but somehow
				 * succeeded on exiting.
				 */
				PRINT_FIELD_ID(" => {", desc, entry_number);
			}
		}

		PRINT_FIELD_0X(", ", desc, base_addr);
		PRINT_FIELD_0X(", ", desc, limit);
		PRINT_FIELD_U_CAST(", ", desc, seg_32bit, unsigned int);
		PRINT_FIELD_U_CAST(", ", desc, contents, unsigned int);
		PRINT_FIELD_U_CAST(", ", desc, read_exec_only, unsigned int);
		PRINT_FIELD_U_CAST(", ", desc, limit_in_pages, unsigned int);
		PRINT_FIELD_U_CAST(", ", desc, seg_not_present, unsigned int);
		PRINT_FIELD_U_CAST(", ", desc, useable, unsigned int);

# ifdef HAVE_STRUCT_USER_DESC_LM
		/* lm is totally ignored for 32-bit processes */
		if (current_klongsize == 8)
			PRINT_FIELD_U_CAST(", ", desc, lm, unsigned int);
# endif /* HAVE_STRUCT_USER_DESC_LM */

		tprints("}");
	}
}

SYS_FUNC(modify_ldt)
{
	if (entering(tcp)) {
		tprintf("%d, ", (int) tcp->u_arg[0]);
		if (tcp->u_arg[2] != sizeof(struct user_desc))
			printaddr(tcp->u_arg[1]);
		else
			print_user_desc(tcp, tcp->u_arg[1], USER_DESC_BOTH);
		tprintf(", %" PRI_klu, tcp->u_arg[2]);

		return 0;
	}

	/*
	 * For some reason ("tht ABI for sys_modify_ldt() expects
	 * 'int'"), modify_ldt clips higher bits on x86_64.
	 */

	if (syserror(tcp) || (kernel_ulong_t) tcp->u_rval < 0xfffff000)
		return 0;

	tcp->u_error = -(unsigned int) tcp->u_rval;

	return 0;
}

SYS_FUNC(set_thread_area)
{
	if (entering(tcp)) {
		print_user_desc(tcp, tcp->u_arg[0], USER_DESC_BOTH);
	} else {
		struct user_desc desc;

		if (!verbose(tcp) || syserror(tcp) ||
		    umove(tcp, tcp->u_arg[0], &desc) < 0) {
			/* returned entry_number is not available */
		} else {
			static char outstr[32];

			xsprintf(outstr, "entry_number=%u", desc.entry_number);
			tcp->auxstr = outstr;
			return RVAL_STR;
		}
	}
	return 0;
}

SYS_FUNC(get_thread_area)
{
	print_user_desc(tcp, tcp->u_arg[0],
			entering(tcp) ? USER_DESC_ENTERING : USER_DESC_EXITING);
	return 0;
}

#endif /* HAVE_STRUCT_USER_DESC */

#if defined(M68K) || defined(MIPS)
SYS_FUNC(set_thread_area)
{
	printaddr(tcp->u_arg[0]);

	return RVAL_DECODED;

}
#endif

#if defined(M68K)
SYS_FUNC(get_thread_area)
{
	return RVAL_DECODED | RVAL_HEX;
}
#endif
