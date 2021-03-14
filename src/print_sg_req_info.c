/*
 * Decode struct sg_req_info.
 *
 * Copyright (c) 2017-2021 Dmitry V. Levin <ldv@strace.io>
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

	tprint_arg_next();
	if (!umove_or_printaddr(tcp, arg, &info)) {
		tprint_struct_begin();
		PRINT_FIELD_D(info, req_state);
		tprint_struct_next();
		PRINT_FIELD_D(info, orphan);
		tprint_struct_next();
		PRINT_FIELD_D(info, sg_io_owned);
		tprint_struct_next();
		PRINT_FIELD_D(info, problem);
		tprint_struct_next();
		PRINT_FIELD_D(info, pack_id);
		tprint_struct_next();
		PRINT_FIELD_PTR(info, usr_ptr);
		tprint_struct_next();
		PRINT_FIELD_U(info, duration);
		tprint_struct_end();
	}

	return RVAL_IOCTL_DECODED;
}

#endif /* HAVE_SCSI_SG_H */
