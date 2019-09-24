/*
 * Copyright (c) 2007 Vladimir Nadvornik <nadvornik@suse.cz>
 * Copyright (c) 2007-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2007-2019 The strace developers.
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
			tprintf("[%u]", iid);
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

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &id)) {
		tprintf("{host_no=%d"
			", channel=%d"
			", scsi_id=%#x"
			", lun=%d"
			", scsi_type=%#x"
			", h_cmd_per_lun=%hd"
			", d_queue_depth=%hd}",
			id.host_no,
			id.channel,
			id.scsi_id,
			id.lun,
			id.scsi_type,
			id.h_cmd_per_lun,
			id.d_queue_depth);
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

			tprints(", ");
			if (umove_or_printaddr(tcp, arg, &iid)) {
				break;
			} else {
				return decode_sg_io(tcp, iid, arg);
			}
		} else {
			uint32_t *piid = get_tcb_priv_data(tcp);
			if (piid)
				decode_sg_io(tcp, *piid, arg);
			tprints("}");
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
		tprints(", ");
		if (!umove_or_printaddr(tcp, arg, &val)) {
			tprints("[");
			if (val & SG_SCSI_RESET_NO_ESCALATE) {
				printxval(sg_scsi_reset,
					  SG_SCSI_RESET_NO_ESCALATE, 0);
				tprints("|");
			}
			printxval(sg_scsi_reset,
				  val & ~SG_SCSI_RESET_NO_ESCALATE,
				  "SG_SCSI_RESET_???");
			tprints("]");

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
		tprints(", ");
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
		tprints(", ");
		printnum_int(tcp, arg, "%d");
		break;

	/* takes an integer by value */
	case SG_SET_TRANSFORM:
		tprintf(", %#x", (unsigned int) arg);
		break;

	/* no arguments */
	case SG_GET_TIMEOUT:
		break;

	default:
		return RVAL_DECODED;
	}

	return RVAL_IOCTL_DECODED;
}
