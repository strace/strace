/*
 * Copyright (c) 2015 Etienne Gemsa <etienne.gemsa@lse.epita.fr>
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include DEF_MPERS_TYPE(struct_ff_effect)

#include <linux/ioctl.h>
#include <linux/input.h>

typedef struct ff_effect struct_ff_effect;

#include MPERS_DEFS

static void
print_ff_envelope(const MPERS_PTR_ARG(struct ff_envelope *) const arg)
{
	const struct ff_envelope *const p = arg;
	tprint_struct_begin();
	PRINT_FIELD_U(*p, attack_length);
	tprint_struct_next();
	PRINT_FIELD_U(*p, attack_level);
	tprint_struct_next();
	PRINT_FIELD_U(*p, fade_length);
	tprint_struct_next();
	PRINT_FIELD_X(*p, fade_level);
	tprint_struct_end();
}

#define DECL_print_ff(name_)	\
	print_ff_ ## name_(const typeof_field(struct_ff_effect, name_) *const p)

static void
DECL_print_ff(trigger)
{
	tprint_struct_begin();
	PRINT_FIELD_U(*p, button);
	tprint_struct_next();
	PRINT_FIELD_U(*p, interval);
	tprint_struct_end();
}

static void
DECL_print_ff(replay)
{
	tprint_struct_begin();
	PRINT_FIELD_U(*p, length);
	tprint_struct_next();
	PRINT_FIELD_U(*p, delay);
	tprint_struct_end();
}

#define PRINT_FIELD_FF_EFFECT(where_, field_)			\
	do {							\
		tprints_field_name(#field_);			\
		print_ff_ ## field_(&((where_).field_));	\
	} while (0)

#define DECL_print_ff_effect(name_)	\
	print_ff_ ## name_ ## _effect(const typeof_field(struct_ff_effect, u.name_) *const p)

static void
DECL_print_ff_effect(constant)
{
	tprint_struct_begin();
	PRINT_FIELD_D(*p, level);
	tprint_struct_next();
	PRINT_FIELD_OBJ_PTR(*p, envelope, print_ff_envelope);
	tprint_struct_end();
}

static void
DECL_print_ff_effect(ramp)
{
	tprint_struct_begin();
	PRINT_FIELD_D(*p, start_level);
	tprint_struct_next();
	PRINT_FIELD_D(*p, end_level);
	tprint_struct_next();
	PRINT_FIELD_OBJ_PTR(*p, envelope, print_ff_envelope);
	tprint_struct_end();
}

static void
DECL_print_ff_effect(periodic)
{
	tprint_struct_begin();
	PRINT_FIELD_U(*p, waveform);
	tprint_struct_next();
	PRINT_FIELD_U(*p, period);
	tprint_struct_next();
	PRINT_FIELD_D(*p, magnitude);
	tprint_struct_next();
	PRINT_FIELD_D(*p, offset);
	tprint_struct_next();
	PRINT_FIELD_U(*p, phase);
	tprint_struct_next();
	PRINT_FIELD_OBJ_PTR(*p, envelope, print_ff_envelope);
	tprint_struct_next();
	PRINT_FIELD_U(*p, custom_len);
	tprint_struct_next();
	PRINT_FIELD_PTR(*p, custom_data);
	tprint_struct_end();
}

static void
DECL_print_ff_effect(rumble)
{
	tprint_struct_begin();
	PRINT_FIELD_U(*p, strong_magnitude);
	tprint_struct_next();
	PRINT_FIELD_U(*p, weak_magnitude);
	tprint_struct_end();
}

#define PRINT_FIELD_FF_TYPE_EFFECT(where_, field_)			\
	do {								\
		tprints_field_name(#field_);				\
		print_ff_ ## field_ ## _effect(&((where_).field_));	\
	} while (0)

static int
ff_effect_ioctl(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_ff_effect ffe;

	if (umove_or_printaddr(tcp, arg, &ffe))
		return RVAL_IOCTL_DECODED;

	tprint_struct_begin();
	PRINT_FIELD_OBJ_VAL(ffe, type, print_evdev_ff_type);
	tprint_struct_next();
	PRINT_FIELD_D(ffe, id);
	tprint_struct_next();
	PRINT_FIELD_U(ffe, direction);
	tprint_struct_next();

	if (abbrev(tcp)) {
		tprint_more_data_follows();
		tprint_struct_end();
		return RVAL_IOCTL_DECODED;
	}

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

	tprint_struct_end();

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
