/*
 * Decode struct sg_req_info.
 *
 * Copyright (c) 2017-2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#ifdef HAVE_SCSI_SG_H

# include DEF_MPERS_TYPE(struct_sg_req_info)

# include <scsi/sg.h>

typedef struct sg_req_info struct_sg_req_info;

#endif /* HAVE_SCSI_SG_H */

#include MPERS_DEFS

#ifdef HAVE_SCSI_SG_H

MPERS_PRINTER_DECL(int, decode_sg_req_info,
		   struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_sg_req_info info;

	if (entering(tcp))
		return 0;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &info)) {
		tprintf("{req_state=%hhd"
			", orphan=%hhd"
			", sg_io_owned=%hhd"
			", problem=%hhd"
			", pack_id=%d"
			", usr_ptr=",
			info.req_state,
			info.orphan,
			info.sg_io_owned,
			info.problem,
			info.pack_id);
		printaddr(ptr_to_kulong(info.usr_ptr));
		tprintf(", duration=%u}", info.duration);
	}

	return RVAL_IOCTL_DECODED;
}

#endif /* HAVE_SCSI_SG_H */
