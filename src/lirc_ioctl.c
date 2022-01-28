/*
 * Copyright (c) 2022 Sean Young <sean@mess.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/lirc.h>

#include "xlat/lirc_features.h"
#include "xlat/lirc_modes.h"

int
lirc_ioctl(struct tcb *const tcp, const unsigned int code,
	   const kernel_ulong_t arg)
{
	uint32_t value;

	if (_IOC_DIR(code) == _IOC_READ && entering(tcp))
		return 0;

	if (umove_or_printaddr(tcp, arg, &value))
		return RVAL_IOCTL_DECODED;

	switch (code) {
	case LIRC_GET_FEATURES:
		tprint_arg_next();
		tprint_indirect_begin();
		printflags(lirc_features, value, "LIRC_CAN_???");
		tprint_indirect_end();
		break;

	case LIRC_GET_SEND_MODE:
	case LIRC_GET_REC_MODE:
	case LIRC_SET_SEND_MODE:
	case LIRC_SET_REC_MODE:
		tprint_arg_next();
		tprint_indirect_begin();
		printxval(lirc_modes, value, "LIRC_MODE_???");
		tprint_indirect_end();
		break;

	case LIRC_GET_REC_RESOLUTION:
	case LIRC_GET_MIN_TIMEOUT:
	case LIRC_GET_MAX_TIMEOUT:
	case LIRC_GET_LENGTH:
	case LIRC_GET_REC_TIMEOUT:
	case LIRC_SET_SEND_CARRIER:
	case LIRC_SET_REC_CARRIER:
	case LIRC_SET_SEND_DUTY_CYCLE:
	case LIRC_SET_REC_TIMEOUT:
	case LIRC_SET_REC_CARRIER_RANGE:
		tprintf(", %u", value);
		break;

	case LIRC_SET_TRANSMITTER_MASK:
	case LIRC_SET_REC_TIMEOUT_REPORTS:
	case LIRC_SET_MEASURE_CARRIER_MODE:
	case LIRC_SET_WIDEBAND_RECEIVER:
		tprintf(", %#x", value);
		return RVAL_IOCTL_DECODED;
	}

	return RVAL_DECODED;
}
