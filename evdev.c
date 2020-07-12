/*
 * Copyright (c) 2015 Etienne Gemsa <etienne.gemsa@lse.epita.fr>
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include "xlat/evdev_abs.h"
#include "xlat/evdev_ev.h"

#ifdef HAVE_LINUX_INPUT_H

# include "print_fields.h"
# include <linux/ioctl.h>
# include "types/evdev.h"

# include "xlat/evdev_autorepeat.h"
# include "xlat/evdev_ff_status.h"
# include "xlat/evdev_ff_types.h"
# include "xlat/evdev_keycode.h"
# include "xlat/evdev_leds.h"
# include "xlat/evdev_misc.h"
# include "xlat/evdev_mtslots.h"
# include "xlat/evdev_prop.h"
# include "xlat/evdev_relative_axes.h"
# include "xlat/evdev_snd.h"
# include "xlat/evdev_switch.h"

/** Added by Linux commit v2.6.38-rc1~247^2~1^2~2^2~5 */
# ifndef INPUT_PROP_MAX
#  define INPUT_PROP_MAX 0x1f
# endif
# ifndef SYN_MAX
#  define SYN_MAX 0xf
# endif

/*
 * Has to be included after struct_* type definitions, since _IO* macros
 * used in fallback definitions require them for sizeof().
 */
# define XLAT_MACROS_ONLY
#  include "xlat/evdev_ioctl_cmds.h"
# undef XLAT_MACROS_ONLY

# ifndef EVIOCGPROP
#  define EVIOCGPROP(len)	_IOR('E', 0x09, len)
# endif
# ifndef EVIOCGMTSLOTS
#  define EVIOCGMTSLOTS(len)	_IOR('E', 0x0a, len)
# endif
# ifndef EVIOCGSW
#  define EVIOCGSW(len)		_IOR('E', 0x1b, len)
# endif

static int
abs_ioctl(struct tcb *const tcp, const unsigned int code,
	  const kernel_ulong_t arg)
{
	static const size_t orig_sz = offsetofend(struct_input_absinfo, flat);
	static const size_t res_sz = offsetofend(struct_input_absinfo,
						 resolution);

	struct_input_absinfo absinfo;
	size_t sz = _IOC_SIZE(code);
	size_t read_sz = MIN(sz, sizeof(absinfo));

	if (sz < orig_sz)
		return RVAL_DECODED;

	tprints(", ");

	if (umoven_or_printaddr(tcp, arg, read_sz, &absinfo))
		return RVAL_IOCTL_DECODED;

	PRINT_FIELD_U("{", absinfo, value);
	PRINT_FIELD_U(", ", absinfo, minimum);

	if (!abbrev(tcp)) {
		PRINT_FIELD_U(", ", absinfo, maximum);
		PRINT_FIELD_U(", ", absinfo, fuzz);
		PRINT_FIELD_U(", ", absinfo, flat);
		if (sz > orig_sz) {
			if (sz >= res_sz)
				PRINT_FIELD_U(", ", absinfo, resolution);
			if (sz != res_sz)
				tprints(", ...");
		}
	} else {
		tprints(", ...");
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
keycode_ioctl(struct tcb *const tcp, const kernel_ulong_t arg)
{
	tprints(", ");

	unsigned int keycode[2];

	if (!umove_or_printaddr(tcp, arg, &keycode)) {
		tprintf("[%u, ", keycode[0]);
		printxval(evdev_keycode, keycode[1], "KEY_???");
		tprints("]");
	}

	return RVAL_IOCTL_DECODED;
}

static int
keycode_V2_ioctl(struct tcb *const tcp, const kernel_ulong_t arg)
{
	tprints(", ");

	struct_input_keymap_entry ike;

	if (umove_or_printaddr(tcp, arg, &ike))
		return RVAL_IOCTL_DECODED;

	PRINT_FIELD_U("{", ike, flags);
	PRINT_FIELD_U(", ", ike, len);

	if (!abbrev(tcp)) {
		PRINT_FIELD_U(", ", ike, index);
		PRINT_FIELD_XVAL(", ", ike, keycode, evdev_keycode, "KEY_???");
		PRINT_FIELD_X_ARRAY(", ", ike, scancode);
	} else {
		tprints(", ...");
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
getid_ioctl(struct tcb *const tcp, const kernel_ulong_t arg)
{
	tprints(", ");

	struct input_id id;

	if (!umove_or_printaddr(tcp, arg, &id)) {
		PRINT_FIELD_U("{", id, bustype);
		PRINT_FIELD_U(", ", id, vendor);
		PRINT_FIELD_U(", ", id, product);
		PRINT_FIELD_U(", ", id, version);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

static int
decode_bitset(struct tcb *const tcp, const kernel_ulong_t arg,
	      const struct xlat *decode_nr, const unsigned int max_nr,
	      const char *const dflt)
{
	tprints(", ");

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
		tprints("[");

		int bit_displayed = 0;
		int i = next_set_bit(decoded_arg, 0, size_bits);
		if (i < 0) {
			tprints(" 0 ");
		} else {
			printxval(decode_nr, i, dflt);

			while ((i = next_set_bit(decoded_arg, i + 1,
						 size_bits)) > 0) {
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
	}

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
		tprints(" /* ");

	if (xlat_verbose(xlat_verbosity) != XLAT_STYLE_ABBREV) {
		print_local_array_ex(tcp, decoded_arg, size / current_wordsize,
				     current_wordsize, print_xlong_array_member,
				     NULL, 0, NULL, NULL);
	}

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
		tprints(" */");

	return RVAL_IOCTL_DECODED;
}

static int
mtslots_ioctl(struct tcb *const tcp, const unsigned int code,
	      const kernel_ulong_t arg)
{
	tprints(", ");

	const size_t size = _IOC_SIZE(code) / sizeof(int);
	if (!size) {
		printaddr(arg);
		return RVAL_IOCTL_DECODED;
	}

	int buffer[size];

	if (umove_or_printaddr(tcp, arg, &buffer))
		return RVAL_IOCTL_DECODED;

	tprints("{code=");
	printxval(evdev_mtslots, buffer[0], "ABS_MT_???");

	tprints(", values=[");

	unsigned int i;
	for (i = 1; i < ARRAY_SIZE(buffer); i++)
		tprintf("%s%d", i > 1 ? ", " : "", buffer[i]);

	tprints("]}");

	return RVAL_IOCTL_DECODED;
}

static int
repeat_ioctl(struct tcb *const tcp, const kernel_ulong_t arg)
{
	tprints(", ");
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
		tprints(", ");
		printnum_int(tcp, arg, "%d");
		return RVAL_IOCTL_DECODED;
	case EV_FF_STATUS:
		return decode_bitset(tcp, arg, evdev_ff_status,
				     FF_STATUS_MAX, "FF_STATUS_???");
	default:
		tprints(", ");
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
		tprints(", ");
		printnum_int(tcp, arg, "%#x");
		return RVAL_IOCTL_DECODED;
	case EVIOCGEFFECTS:
		tprints(", ");
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
		tprints(", ");
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

	return 0;
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
		tprintf(", %d", (int) arg);
		return RVAL_IOCTL_DECODED;
	case EVIOCGRAB:
	case EVIOCREVOKE:
		tprintf(", %" PRI_klu, arg);
		return RVAL_IOCTL_DECODED;
	case EVIOCSCLOCKID:
		tprints(", ");
		printnum_int(tcp, arg, "%u");
		return RVAL_IOCTL_DECODED;
	}

	int rc = evdev_write_ioctl_mpers(tcp, code, arg);

	if (rc != RVAL_DECODED)
		return rc;

	/* multi-number fixed-length commands */
	if ((_IOC_NR(code) & ~ABS_MAX) == _IOC_NR(EVIOCSABS(0)))
		return abs_ioctl(tcp, code, arg);

	return 0;
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
		return evdev_read_ioctl(tcp, code, arg);
	case _IOC_WRITE:
		return evdev_write_ioctl(tcp, code, arg) | RVAL_DECODED;
	default:
		return RVAL_DECODED;
	}
}

#endif /* HAVE_LINUX_INPUT_H */
