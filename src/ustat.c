/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#ifdef HAVE_USTAT_H
# include DEF_MPERS_TYPE(struct_ustat)
# include <ustat.h>
typedef struct ustat struct_ustat;
#endif /* HAVE_USTAT_H */

#include MPERS_DEFS

SYS_FUNC(ustat)
{
	if (entering(tcp)) {
		/* dev */
		print_dev_t((unsigned int) tcp->u_arg[0]);
		tprint_arg_next();
	} else {
		/* ubuf */
#ifdef HAVE_USTAT_H
		struct_ustat ust;

		if (!umove_or_printaddr(tcp, tcp->u_arg[1], &ust)) {
			tprint_struct_begin();
			PRINT_FIELD_U(ust, f_tfree);
			tprint_struct_next();
			PRINT_FIELD_U(ust, f_tinode);
			tprint_struct_end();
		}
#else /* !HAVE_USTAT_H */
		printaddr(tcp->u_arg[1]);
#endif /* HAVE_USTAT_H */
	}

	return 0;
}
