/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017-2018 The strace developers.
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
	if (entering(tcp))
		print_dev_t((unsigned int) tcp->u_arg[0]);
	else {
		tprints(", ");
#ifdef HAVE_USTAT_H
		struct_ustat ust;

		if (!umove_or_printaddr(tcp, tcp->u_arg[1], &ust))
			tprintf("{f_tfree=%llu, f_tinode=%llu}",
				zero_extend_signed_to_ull(ust.f_tfree),
				zero_extend_signed_to_ull(ust.f_tinode));
#else /* !HAVE_USTAT_H */
		printaddr(tcp->u_arg[1]);
#endif /* HAVE_USTAT_H */
	}

	return 0;
}
