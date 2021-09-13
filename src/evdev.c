/*
 * Copyright (c) 2015 Etienne Gemsa <etienne.gemsa@lse.epita.fr>
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/ioctl.h>
#include <linux/input.h>
#include "xlat/evdev_abs.h"
#include "xlat/evdev_ev.h"
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

static int
abs_ioctl(struct tcb *const tcp, const unsigned int code,
	  const kernel_ulong_t arg)
{
	static const size_t orig_sz = offsetofend(struct input_absinfo, flat);
	static const size_t res_sz = offsetofend(struct input_absinfo,
						 resolution);

	struct input_absinfo absinfo;
	size_t sz = _IOC_SIZE(code);
	size_t read_sz = MIN(sz, sizeof(absinfo));

	if (sz < orig_sz) {
		printaddr(arg);
		return RVAL_IOCTL_DECODED;
	}

	if (umoven_or_printaddr(tcp, arg, read_sz, &absinfo))
		return RVAL_IOCTL_DECODED;

	tprint_struct_begin();
	PRINT_FIELD_U(absinfo, value);
	tprint_struct_next();
	PRINT_FIELD_U(absinfo, minimum);

	if (!abbrev(tcp)) {
		tprint_struct_next();
		PRINT_FIELD_U(absinfo, maximum);
		tprint_struct_next();
		PRINT_FIELD_U(absinfo, fuzz);
		tprint_struct_next();
		PRINT_FIELD_U(absinfo, flat);
		if (sz > orig_sz) {
			if (sz >= res_sz) {
				tprint_struct_next();
				PRINT_FIELD_U(absinfo, resolution);
			}
			if (sz != res_sz) {
				tprint_struct_next();
				tprint_more_data_follows();
			}
		}
	} else {
		tprint_struct_next();
		tprint_more_data_follows();
	}

	tprint_struct_end();

	return RVAL_IOCTL_DECODED;
}

static int
keycode_ioctl(struct tcb *const tcp, const kernel_ulong_t arg)
{
	unsigned int keycode[2];

	if (!umove_or_printaddr(tcp, arg, &keycode)) {
		tprint_array_begin();
		PRINT_VAL_U(keycode[0]);
		tprint_array_next();

		printxval(evdev_keycode, keycode[1], "KEY_???");
		tprint_array_end();
	}

	return RVAL_IOCTL_DECODED;
}

static int
keycode_V2_ioctl(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct input_keymap_entry ike;

	if (umove_or_printaddr(tcp, arg, &ike))
		return RVAL_IOCTL_DECODED;

	tprint_struct_begin();
	PRINT_FIELD_U(ike, flags);
	tprint_struct_next();
	PRINT_FIELD_U(ike, len);

	if (!abbrev(tcp)) {
		tprint_struct_next();
		PRINT_FIELD_U(ike, index);
		tprint_struct_next();
		PRINT_FIELD_XVAL(ike, keycode, evdev_keycode, "KEY_???");
		tprint_struct_next();
		PRINT_FIELD_X_ARRAY(ike, scancode);
	} else {
		tprint_struct_next();
		tprint_more_data_follows();
	}

	tprint_struct_end();

	return RVAL_IOCTL_DECODED;
}

static int
getid_ioctl(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct input_id id;

	if (!umove_or_printaddr(tcp, arg, &id)) {
		tprint_struct_begin();
		PRINT_FIELD_U(id, bustype);
		tprint_struct_next();
		PRINT_FIELD_U(id, vendor);
		tprint_struct_next();
		PRINT_FIELD_U(id, product);
		tprint_struct_next();
		PRINT_FIELD_U(id, version);
		tprint_struct_end();
	}

	return RVAL_IOCTL_DECODED;
}

static int
decode_bitset(struct tcb *const tcp, const kernel_ulong_t arg,
	      const struct xlat *decode_nr, const unsigned int max_nr,
	      const char *const dflt)
{
	unsigned int size;
	unsigned int size_bits;

	if ((kernel_ulong_t) tcp->u_rval > max_nr / 8)
		size_bits = max_nr;
	else
		size_bits = tcp->u_rval * 8;

	size = ROUNDUP(ROUNDUP_DIV(size_bits, 8), current_wordsize);

	if (syserror(tcp) || !size) {
		printaddr(arg);

		return RVAL_IOCTL_DECODED;
	}

	char decoded_arg[size];

	if (umove_or_printaddr(tcp, arg, &decoded_arg))
		return RVAL_IOCTL_DECODED;

	if (xlat_verbose(xlat_verbosity) != XLAT_STYLE_RAW) {
		tprint_bitset_begin();

		int bit_displayed = 0;
		int i = next_set_bit(decoded_arg, 0, size_bits);
		if (i >= 0) {
			printxval(decode_nr, i, dflt);

			while ((i = next_set_bit(decoded_arg, i + 1,
						 size_bits)) > 0) {
				if (abbrev(tcp) && bit_displayed >= 3) {
					tprint_bitset_next();
					tprint_more_data_follows();
					break;
				}
				tprint_bitset_next();
				printxval(decode_nr, i, dflt);
				bit_displayed++;
			}
		}

		tprint_bitset_end();
	}

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
		tprint_comment_begin();

	if (xlat_verbose(xlat_verbosity) != XLAT_STYLE_ABBREV) {
		print_local_array_ex(tcp, decoded_arg, size / current_wordsize,
				     current_wordsize, print_xint_array_member,
				     NULL, 0, NULL, NULL);
	}

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
		tprint_comment_end();

	return RVAL_IOCTL_DECODED;
}

static int
mtslots_ioctl(struct tcb *const tcp, const unsigned int code,
	      const kernel_ulong_t arg)
{
	const size_t size = _IOC_SIZE(code) / sizeof(int);
	if (!size) {
		printaddr(arg);
		return RVAL_IOCTL_DECODED;
	}

	struct {
		unsigned int code;
		int values[0];
	} mt;
	if (umove_or_printaddr(tcp, arg, &mt))
		return RVAL_IOCTL_DECODED;

	tprint_struct_begin();

	PRINT_FIELD_XVAL(mt, code, evdev_mtslots, "ABS_MT_???");
	tprint_struct_next();

	tprints_field_name("values");
	int val;
	print_array(tcp, arg + sizeof(val), size - 1, &val, sizeof(val),
		    tfetch_mem, print_int_array_member, NULL);

	tprint_struct_end();

	return RVAL_IOCTL_DECODED;
}

static int
repeat_ioctl(struct tcb *const tcp, const kernel_ulong_t arg)
{
	printpair_int(tcp, arg, "%u");
	return RVAL_IOCTL_DECODED;
}

static int
bit_ioctl(struct tcb *const tcp, const unsigned int ev_nr,
	  const kernel_ulong_t arg)
{
	switch (ev_nr) {
	case 0:
		return decode_bitset(tcp, arg, evdev_ev,
				     EV_MAX, "EV_???");
	case EV_KEY:
		return decode_bitset(tcp, arg, evdev_keycode,
				     KEY_MAX, "KEY_???");
	case EV_REL:
		return decode_bitset(tcp, arg, evdev_relative_axes,
				     REL_MAX, "REL_???");
	case EV_ABS:
		return decode_bitset(tcp, arg, evdev_abs,
				     ABS_MAX, "ABS_???");
	case EV_MSC:
		return decode_bitset(tcp, arg, evdev_misc,
				     MSC_MAX, "MSC_???");
	case EV_SW:
		return decode_bitset(tcp, arg, evdev_switch,
				     SW_MAX, "SW_???");
	case EV_LED:
		return decode_bitset(tcp, arg, evdev_leds,
				     LED_MAX, "LED_???");
	case EV_SND:
		return decode_bitset(tcp, arg, evdev_snd,
				     SND_MAX, "SND_???");
	case EV_REP:
		return decode_bitset(tcp, arg, evdev_autorepeat,
				     REP_MAX, "REP_???");
	case EV_FF:
		return decode_bitset(tcp, arg, evdev_ff_types,
				     FF_MAX, "FF_???");
	case EV_PWR:
		printnum_int(tcp, arg, "%d");
		return RVAL_IOCTL_DECODED;
	case EV_FF_STATUS:
		return decode_bitset(tcp, arg, evdev_ff_status,
				     FF_STATUS_MAX, "FF_STATUS_???");
	default:
		printaddr(arg);
		return RVAL_IOCTL_DECODED;
	}
}

static int
evdev_read_ioctl(struct tcb *const tcp, const unsigned int code,
		 const kernel_ulong_t arg)
{
	/* fixed-number fixed-length commands */
	switch (code) {
	case EVIOCGVERSION:
		printnum_int(tcp, arg, "%#x");
		return RVAL_IOCTL_DECODED;
	case EVIOCGEFFECTS:
		printnum_int(tcp, arg, "%u");
		return RVAL_IOCTL_DECODED;
	case EVIOCGID:
		return getid_ioctl(tcp, arg);
	case EVIOCGREP:
		return repeat_ioctl(tcp, arg);
	case EVIOCGKEYCODE:
		return keycode_ioctl(tcp, arg);
	case EVIOCGKEYCODE_V2:
		return keycode_V2_ioctl(tcp, arg);
	}

	/* fixed-number variable-length commands */
	switch (_IOC_NR(code)) {
	case _IOC_NR(EVIOCGMTSLOTS(0)):
		return mtslots_ioctl(tcp, code, arg);
	case _IOC_NR(EVIOCGNAME(0)):
	case _IOC_NR(EVIOCGPHYS(0)):
	case _IOC_NR(EVIOCGUNIQ(0)):
		if (syserror(tcp))
			printaddr(arg);
		else
			printstrn(tcp, arg, tcp->u_rval);
		return RVAL_IOCTL_DECODED;
	case _IOC_NR(EVIOCGPROP(0)):
		return decode_bitset(tcp, arg, evdev_prop,
				     INPUT_PROP_MAX, "PROP_???");
	case _IOC_NR(EVIOCGSND(0)):
		return decode_bitset(tcp, arg, evdev_snd,
				     SND_MAX, "SND_???");
	case _IOC_NR(EVIOCGSW(0)):
		return decode_bitset(tcp, arg, evdev_switch,
				     SW_MAX, "SW_???");
	case _IOC_NR(EVIOCGKEY(0)):
		return decode_bitset(tcp, arg, evdev_keycode,
				     KEY_MAX, "KEY_???");
	case _IOC_NR(EVIOCGLED(0)):
		return decode_bitset(tcp, arg, evdev_leds,
				     LED_MAX, "LED_???");
	}

	/* multi-number fixed-length commands */
	if ((_IOC_NR(code) & ~ABS_MAX) == _IOC_NR(EVIOCGABS(0)))
		return abs_ioctl(tcp, code, arg);

	/* multi-number variable-length commands */
	if ((_IOC_NR(code) & ~EV_MAX) == _IOC_NR(EVIOCGBIT(0, 0)))
		return bit_ioctl(tcp, _IOC_NR(code) & EV_MAX, arg);

	printaddr(arg);
	return RVAL_IOCTL_DECODED;
}

static int
evdev_write_ioctl(struct tcb *const tcp, const unsigned int code,
		  const kernel_ulong_t arg)
{
	/* fixed-number fixed-length commands */
	switch (code) {
	case EVIOCSREP:
		return repeat_ioctl(tcp, arg);
	case EVIOCSKEYCODE:
		return keycode_ioctl(tcp, arg);
	case EVIOCSKEYCODE_V2:
		return keycode_V2_ioctl(tcp, arg);
	case EVIOCRMFF:
		PRINT_VAL_D((int) arg);
		return RVAL_IOCTL_DECODED;
	case EVIOCGRAB:
	case EVIOCREVOKE:
		PRINT_VAL_U(arg);
		return RVAL_IOCTL_DECODED;
	case EVIOCSCLOCKID:
		printnum_int(tcp, arg, "%u");
		return RVAL_IOCTL_DECODED;
	}

	int rc = evdev_write_ioctl_mpers(tcp, code, arg);

	if (rc != RVAL_DECODED)
		return rc;

	/* multi-number fixed-length commands */
	if ((_IOC_NR(code) & ~ABS_MAX) == _IOC_NR(EVIOCSABS(0)))
		return abs_ioctl(tcp, code, arg);

	printaddr(arg);
	return RVAL_IOCTL_DECODED;
}

void
print_evdev_ff_type(const kernel_ulong_t val)
{
	printxval(evdev_ff_types, val, "FF_???");
}

int
evdev_ioctl(struct tcb *const tcp,
	    const unsigned int code, const kernel_ulong_t arg)
{
	switch (_IOC_DIR(code)) {
	case _IOC_READ:
		if (entering(tcp))
			return 0;
		tprint_arg_next();
		return evdev_read_ioctl(tcp, code, arg);
	case _IOC_WRITE:
		tprint_arg_next();
		return evdev_write_ioctl(tcp, code, arg) | RVAL_DECODED;
	default:
		return RVAL_DECODED;
	}
}
