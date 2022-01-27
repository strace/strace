/*
 * Copyright (c) 2022 Sean Young <sean@mess.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/lirc.h>

static const struct features {
	int flag;
	const char *name;
} features[] = {
	{ LIRC_CAN_REC_SCANCODE, "LIRC_CAN_REC_SCANCODE" },
	{ LIRC_CAN_REC_MODE2, "LIRC_CAN_REC_MODE2" },
	{ LIRC_CAN_GET_REC_RESOLUTION, "LIRC_CAN_GET_REC_RESOLUTION" },
	{ LIRC_CAN_SEND_PULSE, "LIRC_CAN_SEND_PULSE" },
	{ LIRC_CAN_SET_TRANSMITTER_MASK, "LIRC_CAN_SET_TRANSMITTER_MASK" },
	{ LIRC_CAN_SET_SEND_CARRIER, "LIRC_CAN_SET_SEND_CARRIER" },
	{ LIRC_CAN_SET_SEND_DUTY_CYCLE, "LIRC_CAN_SET_SEND_DUTY_CYCLE" },
	{ LIRC_CAN_SET_REC_CARRIER, "LIRC_CAN_SET_REC_CARRIER" },
	{ LIRC_CAN_SET_REC_CARRIER_RANGE, "LIRC_CAN_SET_REC_CARRIER_RANGE" },
	{ LIRC_CAN_USE_WIDEBAND_RECEIVER, "LIRC_CAN_USE_WIDEBAND_RECEIVER" },
	{ LIRC_CAN_MEASURE_CARRIER, "LIRC_CAN_MEASURE_CARRIER" },
	{ LIRC_CAN_SET_REC_TIMEOUT, "LIRC_CAN_SET_REC_TIMEOUT" },
};

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
	case LIRC_GET_FEATURES: {
		bool needs_or = false;
		unsigned long i;

		tprintf(", features=");

		for (i = 0; i < ARRAY_SIZE(features); i++) {
			if (value & features[i].flag) {
				if (needs_or)
					tprintf("|");
				tprintf("%s", features[i].name);
				needs_or = true;
			}
		}
		break;
	}
	case LIRC_GET_SEND_MODE:
	case LIRC_GET_REC_MODE:
	case LIRC_SET_SEND_MODE:
	case LIRC_SET_REC_MODE:
		switch (value) {
		case LIRC_MODE_RAW:
			tprintf(", mode=raw");
			break;
		case LIRC_MODE_PULSE:
			tprintf(", mode=pulse");
			break;
		case LIRC_MODE_MODE2:
			tprintf(", mode=mode2");
			break;
		case LIRC_MODE_SCANCODE:
			tprintf(", mode=scancode");
			break;
		case LIRC_MODE_LIRCCODE:
			tprintf(", mode=lirccode");
			break;
		default:
			tprintf(", mode=%u", value);
			break;
		}
		break;
	default:
		tprintf(", %u", value);
		break;
	}

	return RVAL_IOCTL_DECODED;
}
