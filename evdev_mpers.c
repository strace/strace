/*
 * Copyright (c) 2015 Etienne Gemsa <etienne.gemsa@lse.epita.fr>
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@strace.io>
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
print_ff_envelope(const MPERS_PTR_ARG(struct ff_envelope *) const arg)
{
	const struct ff_envelope *const p = arg;
	PRINT_FIELD_U("{", *p, attack_length);
	PRINT_FIELD_U(", ", *p, attack_level);
	PRINT_FIELD_U(", ", *p, fade_length);
	PRINT_FIELD_X(", ", *p, fade_level);
	tprints("}");
}

#define DECL_print_ff(name_)	\
	print_ff_ ## name_(const typeof_field(struct_ff_effect, name_) *const p)

static void
DECL_print_ff(trigger)
{
	PRINT_FIELD_U("{", *p, button);
	PRINT_FIELD_U(", ", *p, interval);
	tprints("}");
}

static void
DECL_print_ff(replay)
{
	PRINT_FIELD_U("{", *p, length);
	PRINT_FIELD_U(", ", *p, delay);
	tprints("}");
}

# define PRINT_FIELD_FF_EFFECT(where_, field_)			\
	do {							\
		tprints_field_name(#field_);			\
		print_ff_ ## field_(&((where_).field_));	\
	} while (0)

#define DECL_print_ff_effect(name_)	\
	print_ff_ ## name_ ## _effect(const typeof_field(struct_ff_effect, u.name_) *const p)

static void
DECL_print_ff_effect(constant)
{
	PRINT_FIELD_D("{", *p, level);
	PRINT_FIELD_OBJ_PTR(", ", *p, envelope, print_ff_envelope);
	tprints("}");
}

static void
DECL_print_ff_effect(ramp)
{
	PRINT_FIELD_D("{", *p, start_level);
	PRINT_FIELD_D(", ", *p, end_level);
	PRINT_FIELD_OBJ_PTR(", ", *p, envelope, print_ff_envelope);
	tprints("}");
}

static void
DECL_print_ff_effect(periodic)
{
	PRINT_FIELD_U("{", *p, waveform);
	PRINT_FIELD_U(", ", *p, period);
	PRINT_FIELD_D(", ", *p, magnitude);
	PRINT_FIELD_D(", ", *p, offset);
	PRINT_FIELD_U(", ", *p, phase);
	PRINT_FIELD_OBJ_PTR(", ", *p, envelope, print_ff_envelope);
	PRINT_FIELD_U(", ", *p, custom_len);
	PRINT_FIELD_PTR(", ", *p, custom_data);
	tprints("}");
}

static void
DECL_print_ff_effect(rumble)
{
	PRINT_FIELD_U("{", *p, strong_magnitude);
	PRINT_FIELD_U(", ", *p, weak_magnitude);
	tprints("}");
}

# define PRINT_FIELD_FF_TYPE_EFFECT(where_, field_)			\
	do {								\
		tprints_field_name(#field_);				\
		print_ff_ ## field_ ## _effect(&((where_).field_));	\
	} while (0)

static int
ff_effect_ioctl(struct tcb *const tcp, const kernel_ulong_t arg)
{
	tprints(", ");

	struct_ff_effect ffe;

	if (umove_or_printaddr(tcp, arg, &ffe))
		return RVAL_IOCTL_DECODED;

	PRINT_FIELD_OBJ_VAL("{", ffe, type, print_evdev_ff_type);
	PRINT_FIELD_D(", ", ffe, id);
	PRINT_FIELD_U(", ", ffe, direction);

	if (abbrev(tcp)) {
		tprints(", ...}");
		return RVAL_IOCTL_DECODED;
	}

	tprint_struct_next();
	PRINT_FIELD_FF_EFFECT(ffe, trigger);
	tprint_struct_next();
	PRINT_FIELD_FF_EFFECT(ffe, replay);

	switch (ffe.type) {
	case FF_CONSTANT:
		tprint_struct_next();
		PRINT_FIELD_FF_TYPE_EFFECT(ffe.u, constant);
		break;
	case FF_RAMP:
		tprint_struct_next();
		PRINT_FIELD_FF_TYPE_EFFECT(ffe.u, ramp);
		break;
	case FF_PERIODIC:
		tprint_struct_next();
		PRINT_FIELD_FF_TYPE_EFFECT(ffe.u, periodic);
		break;
	case FF_RUMBLE:
		tprint_struct_next();
		PRINT_FIELD_FF_TYPE_EFFECT(ffe.u, rumble);
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
