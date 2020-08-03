/*
 * Copyright (c) 2015 Etienne Gemsa <etienne.gemsa@lse.epita.fr>
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#ifdef HAVE_LINUX_INPUT_H

# include DEF_MPERS_TYPE(struct_ff_effect)

# include <linux/ioctl.h>
# include <linux/input.h>

typedef struct ff_effect struct_ff_effect;

#endif /* HAVE_LINUX_INPUT_H */

#include MPERS_DEFS

#ifdef HAVE_LINUX_INPUT_H

# include "print_fields.h"

static void
decode_envelope(void *const data)
{
	const struct ff_envelope *const envelope = data;

	PRINT_FIELD_U(", envelope={", *envelope, attack_length);
	PRINT_FIELD_U(", ", *envelope, attack_level);
	PRINT_FIELD_U(", ", *envelope, fade_length);
	PRINT_FIELD_X(", ", *envelope, fade_level);
	tprints("}");
}

static int
ff_effect_ioctl(struct tcb *const tcp, const kernel_ulong_t arg)
{
	tprints(", ");

	struct_ff_effect ffe;

	if (umove_or_printaddr(tcp, arg, &ffe))
		return RVAL_IOCTL_DECODED;

	tprints("{type=");
	print_evdev_ff_type(ffe.type);
	PRINT_FIELD_D(", ", ffe, id);
	PRINT_FIELD_U(", ", ffe, direction);

	if (abbrev(tcp)) {
		tprints(", ...}");
		return RVAL_IOCTL_DECODED;
	}

	PRINT_FIELD_U(", trigger={", ffe.trigger, button);
	PRINT_FIELD_U(", ", ffe.trigger, interval);
	PRINT_FIELD_U("}, replay={", ffe.replay, length);
	PRINT_FIELD_U(", ", ffe.replay, delay);
	tprints("}");

	switch (ffe.type) {
	case FF_CONSTANT:
		PRINT_FIELD_D(", constant={", ffe.u.constant, level);
		decode_envelope(&ffe.u.constant.envelope);
		tprints("}");
		break;
	case FF_RAMP:
		PRINT_FIELD_D(", ramp={", ffe.u.ramp, start_level);
		PRINT_FIELD_D(", ", ffe.u.ramp, end_level);
		decode_envelope(&ffe.u.ramp.envelope);
		tprints("}");
		break;
	case FF_PERIODIC:
		PRINT_FIELD_U(", periodic={", ffe.u.periodic, waveform);
		PRINT_FIELD_U(", ", ffe.u.periodic, period);
		PRINT_FIELD_D(", ", ffe.u.periodic, magnitude);
		PRINT_FIELD_D(", ", ffe.u.periodic, offset);
		PRINT_FIELD_U(", ", ffe.u.periodic, phase);
		decode_envelope(&ffe.u.periodic.envelope);
		PRINT_FIELD_U(", ", ffe.u.periodic, custom_len);
		tprints(", custom_data=");
		printaddr(ptr_to_kulong(ffe.u.periodic.custom_data));
		tprints("}");
		break;
	case FF_RUMBLE:
		PRINT_FIELD_U(", rumble={", ffe.u.rumble, strong_magnitude);
		PRINT_FIELD_U(", ", ffe.u.rumble, weak_magnitude);
		tprints("}");
		break;
	default:
		break;
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

MPERS_PRINTER_DECL(int, evdev_write_ioctl_mpers, struct tcb *const tcp,
		   const unsigned int code, const kernel_ulong_t arg)
{
	switch (code) {
	case EVIOCSFF:
		return ff_effect_ioctl(tcp, arg);
	default:
		return RVAL_DECODED;
	}
}

#endif /* HAVE_LINUX_INPUT_H */
