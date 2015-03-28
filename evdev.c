/*
 * Copyright (c) 2015 Etienne Gemsa <etienne.gemsa@lse.epita.fr>
 * Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "defs.h"

#include <linux/ioctl.h>

#ifdef HAVE_LINUX_INPUT_H
#include <linux/input.h>
#include "xlat/evdev_abs.h"
#include "xlat/evdev_autorepeat.h"
#include "xlat/evdev_ff_status.h"
#include "xlat/evdev_ff_types.h"
#include "xlat/evdev_keycode.h"
#include "xlat/evdev_leds.h"
#include "xlat/evdev_misc.h"
#include "xlat/evdev_mtslots.h"
#include "xlat/evdev_prop.h"
#include "xlat/evdev_relative_axes.h"
#include "xlat/evdev_snd.h"
#include "xlat/evdev_switch.h"
#include "xlat/evdev_sync.h"

#ifndef SYN_MAX
# define SYN_MAX 0xf
#endif

static void
decode_envelope(struct ff_envelope *envelope)
{
	tprintf(", envelope={attack_length=%" PRIu16 ", attack_level=%" PRIu16
		", fade_length=%" PRIu16 ", fade_level=%" PRIx32 "}",
		envelope->attack_length,
		envelope->attack_level,
		envelope->fade_length,
		envelope->fade_level);
}

static int
ff_effect_ioctl(struct tcb *tcp, long arg)
{
	struct ff_effect ffe;

	if (!verbose(tcp) || umove(tcp, arg, &ffe) < 0)
		return 0;

	tprints(", {type=");
	printxval(evdev_ff_types, ffe.type, "FF_???");
	tprintf(", id=%" PRIu16 ", direction=%" PRIu16,
		ffe.id, ffe.direction);

	if (!abbrev(tcp)) {
		tprintf(", trigger={button=%" PRIu16 ", interval=%" PRIu16 "}",
			ffe.trigger.button, ffe.trigger.interval);
		tprintf(", replay={lenght=%" PRIu16 ", delay=%" PRIu16 "}",
			ffe.replay.length, ffe.replay.delay);
		switch (ffe.type) {
			case FF_CONSTANT:
				tprintf(", constant_ef={%" PRIi16,
					ffe.u.constant.level);
				decode_envelope(&ffe.u.constant.envelope);
				tprints("}");
				return 1;
			case FF_RAMP:
				tprintf(", ramp={start_level=%" PRIi16
					", end_level=%" PRIi16,
					ffe.u.ramp.start_level,
					ffe.u.ramp.end_level);
				decode_envelope(&ffe.u.ramp.envelope);
				tprints("}");
				return 1;
			case FF_PERIODIC:
				tprintf(", periodic_ef={waveform=%" PRIu16
					", period=%" PRIu16
					", magnitude=%" PRIi16
					", offset=%" PRIi16
					", phase=%" PRIu16,
					ffe.u.periodic.waveform,
					ffe.u.periodic.period,
					ffe.u.periodic.magnitude,
					ffe.u.periodic.offset,
					ffe.u.periodic.phase);
				decode_envelope(&ffe.u.periodic.envelope);
				tprintf(", custom_len=%" PRIu32
					", *custom_data=%#lx}",
					ffe.u.periodic.custom_len,
					(unsigned long)ffe.u.periodic.custom_data);
				return 1;
			case FF_RUMBLE:
				tprintf(", rumble={strong_magnitude=%" PRIu16
					", weak_magnitude=%" PRIu16 "}",
					ffe.u.rumble.strong_magnitude,
					ffe.u.rumble.weak_magnitude);
				return 1;
			case FF_SPRING:
			case FF_FRICTION:
			case FF_DAMPER:
			case FF_INERTIA:
			case FF_CUSTOM:
				break;
			default :
				break;
		}
	}

	tprints(", ...}");
	return 1;
}

static int
abs_ioctl(struct tcb *tcp, long arg)
{
	struct input_absinfo absinfo;

	if (!verbose(tcp) || umove(tcp, arg, &absinfo) < 0)
		return 0;

	tprintf(", {value=%" PRIu32 ", minimum=%" PRIu32,
		absinfo.value, absinfo.minimum);
	if (!abbrev(tcp)) {
		tprintf(", maximum=%" PRIu32 ", fuzz=%" PRIu32,
			absinfo.maximum, absinfo.fuzz);
		tprintf(", flat=%" PRIu32, absinfo.flat);
#ifdef HAVE_STRUCT_INPUT_ABSINFO_RESOLUTION
		tprintf(", resolution=%" PRIu32, absinfo.resolution);
#endif
		tprints("}");
	} else {
		tprints(", ...}");
	}
	return 1;
}

static int
keycode_ioctl(struct tcb *tcp, long arg)
{
	unsigned int keycode[2];

	if (!arg) {
		tprints(", NULL");
		return 1;
	}

	if (!verbose(tcp) || umove(tcp, arg, &keycode) < 0)
		return 0;

	tprintf(", [%u, ", keycode[0]);
	printxval(evdev_keycode, keycode[1], "KEY_???");
	tprints("]");
	return 1;
}

#ifdef EVIOCGKEYCODE_V2
static int
keycode_V2_ioctl(struct tcb *tcp, long arg)
{
	struct input_keymap_entry ike;

	if (!arg) {
		tprints(", NULL");
		return 1;
	}

	if (!verbose(tcp) || umove(tcp, arg, &ike) < 0)
		return 0;

	tprintf(", {flags=%" PRIu8 ", len=%" PRIu8, ike.flags, ike.len);
	if (!abbrev(tcp)) {
		unsigned int i;

		tprintf(", index=%" PRIu16 ", keycode=", ike.index);
		printxval(evdev_keycode, ike.keycode, "KEY_???");
		tprints(", scancode=[");
		for (i = 0; i < ARRAY_SIZE(ike.scancode); i++) {
			if (i > 0)
				tprints(", ");
			tprintf("%" PRIx8, ike.scancode[i]);
		}
		tprints("]}");
	} else {
		tprints(", ...}");
	}
	return 1;
}
#endif /* EVIOCGKEYCODE_V2 */

static int
getid_ioctl(struct tcb *tcp, long arg)
{
	struct input_id id;

	if (!verbose(tcp) || umove(tcp, arg, &id) < 0)
		return 0;

	tprintf(", {ID_BUS=%" PRIu16 ", ID_VENDOR=%" PRIu16,
		id.bustype, id.vendor);
	if (!abbrev(tcp)) {
		tprintf(", ID_PRODUCT=%" PRIu16 ", ID_VERSION=%" PRIu16 "}",
			id.product, id.version);
	} else {
		tprints(", ...}");
	}
	return 1;
}

static int
decode_bitset(struct tcb *tcp, long arg, const struct xlat decode_nr[],
	      const unsigned int max_nr, const char *dflt)
{
	if (!verbose(tcp))
		return 0;

	unsigned int size;
	if ((unsigned long) tcp->u_rval > max_nr)
		size = max_nr;
	else
		size = tcp->u_rval;
	char decoded_arg[size];

	if (umoven(tcp, arg, size, decoded_arg) < 0)
		return 0;

	tprints(", [");

	int bit_displayed = 0;
	int i = next_set_bit(decoded_arg, 0, size);
	if (i < 0) {
		tprints(" 0 ");
	} else {
		printxval(decode_nr, i, dflt);

		while ((i = next_set_bit(decoded_arg, i + 1, size)) > 0) {
			if (abbrev(tcp) && bit_displayed >= 3) {
				tprints(", ...");
				break;
			}
			tprints(", ");
			printxval(decode_nr, i, dflt);
			bit_displayed++;
		}
	}

	tprints("]");

	return 1;
}

#ifdef EVIOCGMTSLOTS
static int
mtslots_ioctl(struct tcb *tcp, const unsigned int code, long arg)
{
	const size_t size = _IOC_SIZE(code) / sizeof(int32_t);
	if (!size)
		return 0;

	int32_t buffer[size];

	if (!verbose(tcp) || umove(tcp, arg, &buffer) < 0)
		return 0;

	tprints(", {code=");
	printxval(evdev_mtslots, buffer[0], "ABS_MT_???");

	unsigned int i;
	tprints(", values=[");

	for (i = 1; i < ARRAY_SIZE(buffer); i++)
		tprintf("%s%d", i > 1 ? ", " : "", buffer[i]);

	tprints("]}");
	return 1;
}
#endif /* EVIOCGMTSLOTS */

#ifdef EVIOCGREP
static int
repeat_ioctl(struct tcb *tcp, long arg)
{
	unsigned int val[2];

	if (!verbose(tcp) || umove(tcp, arg, &val) < 0)
		return 0;

	tprintf(", [%" PRIu32 " %" PRIu32 "]", val[0], val[1]);
	return 1;
}
#endif /* EVIOCGREP */

static int
evdev_read_ioctl(struct tcb *tcp, const unsigned int code, long arg)
{
	if (entering(tcp))
		return 1;

	if (syserror(tcp))
		return 0;

	if ((_IOC_NR(code) & ~EV_MAX) == _IOC_NR(EVIOCGBIT(0, 0))) {
		switch (_IOC_NR(code) - 0x20) {
			case EV_SYN:
				return decode_bitset(tcp, arg, evdev_sync,
						SYN_MAX, "SYN_???");
			case EV_KEY:
				return decode_bitset(tcp, arg, evdev_keycode,
						KEY_MAX, "KEY_???");
			case EV_REL:
				return decode_bitset(tcp, arg, evdev_relative_axes,
						REL_MAX, "REL_???");
			case EV_ABS:
				return decode_bitset(tcp, arg,
						evdev_abs, ABS_MAX, "ABS_???");
			case EV_MSC:
				return decode_bitset(tcp, arg,
						evdev_misc, MSC_MAX, "MSC_???");
#ifdef EV_SW
			case EV_SW:
				return decode_bitset(tcp, arg,
						evdev_switch, SW_MAX, "SW_???");
#endif
			case EV_LED:
				return decode_bitset(tcp, arg,
						evdev_leds, LED_MAX, "LED_???");
			case EV_SND:
				return decode_bitset(tcp, arg,
						evdev_snd, SND_MAX, "SND_???");
			case EV_REP:
				return decode_bitset(tcp, arg, evdev_autorepeat,
						REP_MAX, "REP_???");
			case EV_FF:
				return decode_bitset(tcp, arg, evdev_ff_types,
						FF_MAX, "FF_???");
			case EV_PWR:
				printnum_int(tcp, arg, "%d");
				return 1;
			case EV_FF_STATUS:
				return decode_bitset(tcp, arg, evdev_ff_status,
						FF_STATUS_MAX, "FF_STATUS_???");
			default:
				return 0;
		}
	}

	if ((_IOC_NR(code) & ~ABS_MAX) == _IOC_NR(EVIOCGABS(0)))
		return abs_ioctl(tcp, arg);

	switch (code) {
		case EVIOCGVERSION:
			tprints(", ");
			printnum_int(tcp, arg, "%" PRIx32);
			return 1;
		case EVIOCGEFFECTS:
			tprints(", ");
			printnum_int(tcp, arg, "%" PRIu32);
			return 1;
		case EVIOCGID:
			return getid_ioctl(tcp, arg);
#ifdef EVIOCGREP
		case EVIOCGREP:
			return repeat_ioctl(tcp, arg);;
#endif
		case EVIOCGKEYCODE:
			return keycode_ioctl(tcp, arg);
#ifdef EVIOCGKEYCODE_V2
		case EVIOCGKEYCODE_V2:
			return keycode_V2_ioctl(tcp, arg);
#endif
	}

	switch (_IOC_NR(code)) {
#ifdef EVIOCGMTSLOTS
		case _IOC_NR(EVIOCGMTSLOTS(0)):
			return mtslots_ioctl(tcp, code, arg);
#endif
		case _IOC_NR(EVIOCGNAME(0)):
		case _IOC_NR(EVIOCGPHYS(0)):
		case _IOC_NR(EVIOCGUNIQ(0)):
			tprints(", ");
			printstr(tcp, arg, tcp->u_rval - 1);
			return 1;
#ifdef EVIOCGPROP
		case _IOC_NR(EVIOCGPROP(0)):
			return decode_bitset(tcp, arg,
					evdev_prop, INPUT_PROP_MAX, "PROP_???");
#endif
		case _IOC_NR(EVIOCGSND(0)):
			return decode_bitset(tcp, arg,
					evdev_snd, SND_MAX, "SND_???");
#ifdef EVIOCGSW
		case _IOC_NR(EVIOCGSW(0)):
			return decode_bitset(tcp, arg,
					evdev_switch, SW_MAX, "SW_???");
#endif
		case _IOC_NR(EVIOCGKEY(0)):
			return decode_bitset(tcp, arg,
					evdev_keycode, KEY_MAX, "KEY_???");
		case _IOC_NR(EVIOCGLED(0)):
			return decode_bitset(tcp, arg,
					evdev_leds, LED_MAX, "LED_???");
		default:
			return 0;
	}
}

static int
evdev_write_ioctl(struct tcb *tcp, const unsigned int code, long arg)
{
	if (exiting(tcp))
		return 1;

	if ((_IOC_NR(code) & ~ABS_MAX) == _IOC_NR(EVIOCSABS(0)))
		return abs_ioctl(tcp, arg);

	switch (code) {
#ifdef EVIOCSREP
		case EVIOCSREP:
			return repeat_ioctl(tcp, arg);
#endif
		case EVIOCSKEYCODE:
			return keycode_ioctl(tcp, arg);
#ifdef EVIOCSKEYCODE_V2
		case EVIOCSKEYCODE_V2:
			return keycode_V2_ioctl(tcp, arg);
#endif
		case EVIOCSFF:
			return ff_effect_ioctl(tcp, arg);
		case EVIOCRMFF:
#ifdef EVIOCSCLOCKID
		case EVIOCSCLOCKID:
#endif
		case EVIOCGRAB:
#ifdef EVIOCREVOKE
		case EVIOCREVOKE:
#endif
			tprints(", ");
			printnum_int(tcp, arg, "%u");
			return 1;
		default:
			return 0;
	}
}

int
evdev_ioctl(struct tcb *tcp, const unsigned int code, long arg)
{
	switch(_IOC_DIR(code)) {
		case _IOC_READ:
			return evdev_read_ioctl(tcp, code, arg);
		case _IOC_WRITE:
			if (!evdev_write_ioctl(tcp, code, arg))
				tprintf(", %lx", arg);
			return 1;
		default:
			return 0;
	}
}

#endif /* HAVE_LINUX_INPUT_H */
