/*
 * Copyright (c) 2007 Vladimir Nadvornik <nadvornik@suse.cz>
 * Copyright (c) 2007-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2007-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#ifdef HAVE_SCSI_SG_H
# include <scsi/sg.h>
#endif

#define XLAT_MACROS_ONLY
# include "xlat/scsi_sg_commands.h"
#undef XLAT_MACROS_ONLY
#include "xlat/sg_scsi_reset.h"

static int
decode_sg_io(struct tcb *const tcp, const uint32_t iid,
	     const kernel_ulong_t arg)
{
	switch (iid) {
		case 'S':
			return decode_sg_io_v3(tcp, arg);
		case 'Q':
			return decode_sg_io_v4(tcp, arg);
		default:
			tprint_indirect_begin();
			PRINT_VAL_U(iid);
			tprint_indirect_end();
			return RVAL_IOCTL_DECODED;
	}

}

#ifdef HAVE_SCSI_SG_H

static int
decode_sg_scsi_id(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct sg_scsi_id id;

	if (entering(tcp))
		return 0;

	tprint_arg_next();
	if (!umove_or_printaddr(tcp, arg, &id)) {
		tprint_struct_begin();
		PRINT_FIELD_D(id, host_no);
		tprint_struct_next();
		PRINT_FIELD_D(id, channel);
		tprint_struct_next();
		PRINT_FIELD_X(id, scsi_id);
		tprint_struct_next();
		PRINT_FIELD_D(id, lun);
		tprint_struct_next();
		PRINT_FIELD_X(id, scsi_type);
		tprint_struct_next();
		PRINT_FIELD_D(id, h_cmd_per_lun);
		tprint_struct_next();
		PRINT_FIELD_D(id, d_queue_depth);
		tprint_struct_end();
	}
	return RVAL_IOCTL_DECODED;
}

#endif /* HAVE_SCSI_SG_H */

int
scsi_ioctl(struct tcb *const tcp, const unsigned int code,
	   const kernel_ulong_t arg)
{
	switch (code) {
	case SG_IO:
		if (entering(tcp)) {
			uint32_t iid;

			tprint_arg_next();
			if (umove_or_printaddr(tcp, arg, &iid)) {
				break;
			} else {
				return decode_sg_io(tcp, iid, arg);
			}
		} else {
			uint32_t *piid = get_tcb_priv_data(tcp);
			if (piid)
				decode_sg_io(tcp, *piid, arg);
			tprint_struct_end();
			break;
		}

#ifdef HAVE_SCSI_SG_H
	/* returns struct sg_scsi_id */
	case SG_GET_SCSI_ID:
		return decode_sg_scsi_id(tcp, arg);
	/* returns struct sg_req_info */
	case SG_GET_REQUEST_TABLE:
		return decode_sg_req_info(tcp, arg);
#endif /* HAVE_SCSI_SG_H */

	/* takes a value by pointer */
	case SG_SCSI_RESET: {
		unsigned int val;
		tprint_arg_next();
		if (!umove_or_printaddr(tcp, arg, &val)) {
			tprint_indirect_begin();
			tprint_flags_begin();
			if (val & SG_SCSI_RESET_NO_ESCALATE) {
				printxval(sg_scsi_reset,
					  SG_SCSI_RESET_NO_ESCALATE, 0);
				tprint_flags_or();
			}
			printxval(sg_scsi_reset,
				  val & ~SG_SCSI_RESET_NO_ESCALATE,
				  "SG_SCSI_RESET_???");
			tprint_flags_end();
			tprint_indirect_end();

		}
		break;
	}

	/* takes a signed int by pointer */
	case SG_NEXT_CMD_LEN:
	case SG_SET_COMMAND_Q:
	case SG_SET_DEBUG:
	case SG_SET_FORCE_LOW_DMA:
	case SG_SET_FORCE_PACK_ID:
	case SG_SET_KEEP_ORPHAN:
	case SG_SET_RESERVED_SIZE:
	case SG_SET_TIMEOUT:
		tprint_arg_next();
		printnum_int(tcp, arg, "%d");
		break;

	/* returns a signed int by pointer */
	case SG_EMULATED_HOST:
	case SG_GET_ACCESS_COUNT:
	case SG_GET_COMMAND_Q:
	case SG_GET_KEEP_ORPHAN:
	case SG_GET_LOW_DMA:
	case SG_GET_NUM_WAITING:
	case SG_GET_PACK_ID:
	case SG_GET_RESERVED_SIZE:
	case SG_GET_SG_TABLESIZE:
	case SG_GET_TRANSFORM:
	case SG_GET_VERSION_NUM:
		if (entering(tcp))
			return 0;
		tprint_arg_next();
		printnum_int(tcp, arg, "%d");
		break;

	/* takes an integer by value */
	case SG_SET_TRANSFORM:
		tprint_arg_next();
		PRINT_VAL_X((unsigned int) arg);
		break;

	/* no arguments */
	case SG_GET_TIMEOUT:
		break;

	default:
		return RVAL_DECODED;
	}

	return RVAL_IOCTL_DECODED;
}
