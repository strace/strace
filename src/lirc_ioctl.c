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
	unsigned int value;

	switch (code) {
	case LIRC_GET_FEATURES:
	case LIRC_GET_SEND_MODE:
	case LIRC_GET_REC_MODE:
	case LIRC_GET_REC_RESOLUTION:
	case LIRC_GET_MIN_TIMEOUT:
	case LIRC_GET_MAX_TIMEOUT:
	case LIRC_GET_LENGTH:
	case LIRC_SET_SEND_MODE:
	case LIRC_SET_REC_MODE:
	case LIRC_SET_SEND_CARRIER:
	case LIRC_SET_REC_CARRIER:
	case LIRC_SET_SEND_DUTY_CYCLE:
	case LIRC_SET_TRANSMITTER_MASK:
	case LIRC_SET_REC_TIMEOUT:
	case LIRC_SET_REC_TIMEOUT_REPORTS:
	case LIRC_SET_MEASURE_CARRIER_MODE:
	case LIRC_SET_REC_CARRIER_RANGE:
	case LIRC_SET_WIDEBAND_RECEIVER:
	case LIRC_GET_REC_TIMEOUT:
		break;
	default:
		return RVAL_DECODED;
	}

	if (entering(tcp)) {
		tprint_arg_next();
		if (_IOC_DIR(code) == _IOC_READ)
			return 0;
	}

	if (umove_or_printaddr(tcp, arg, &value))
		return RVAL_IOCTL_DECODED;

	tprint_indirect_begin();

	switch (code) {
	case LIRC_GET_FEATURES:
		printflags(lirc_features, value, "LIRC_CAN_???");
		break;
	case LIRC_GET_SEND_MODE:
	case LIRC_GET_REC_MODE:
	case LIRC_SET_SEND_MODE:
	case LIRC_SET_REC_MODE:
		printxval(lirc_modes, value, "LIRC_MODE_???");
		break;
	case LIRC_SET_TRANSMITTER_MASK:
		PRINT_VAL_X(value);
		break;
	default:
		PRINT_VAL_U(value);
		break;
	}

	tprint_indirect_end();

	return RVAL_IOCTL_DECODED;
}
