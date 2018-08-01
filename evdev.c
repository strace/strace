/*
 * Copyright (c) 2015 Etienne Gemsa <etienne.gemsa@lse.epita.fr>
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2018 The strace developers.
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

#include "xlat/evdev_abs.h"
#include "xlat/evdev_ev.h"

#ifdef HAVE_LINUX_INPUT_H

# include <linux/ioctl.h>
# include <linux/input.h>

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

# ifndef SYN_MAX
#  define SYN_MAX 0xf
# endif

const size_t evdev_abs_size = ARRAY_SIZE(evdev_abs) - 1;

static int
abs_ioctl(struct tcb *const tcp, const kernel_ulong_t arg)
{
	tprints(", ");

	struct input_absinfo absinfo;

	if (!umove_or_printaddr(tcp, arg, &absinfo)) {
		tprintf("{value=%u"
			", minimum=%u, ",
			absinfo.value,
			absinfo.minimum);

		if (!abbrev(tcp)) {
			tprintf("maximum=%u"
				", fuzz=%u"
				", flat=%u",
				absinfo.maximum,
				absinfo.fuzz,
				absinfo.flat);
# ifdef HAVE_STRUCT_INPUT_ABSINFO_RESOLUTION
			tprintf(", resolution=%u",
				absinfo.resolution);
# endif
		} else {
			tprints("...");
		}

		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

static int
keycode_ioctl(struct tcb *const tcp, const kernel_ulong_t arg)
{
	tprints(", ");

	unsigned int keycode[2];

	if (!umove_or_printaddr(tcp, arg, &keycode)) {
		tprintf("[%u, ", keycode[0]);
		printxval_index(evdev_keycode, keycode[1], "KEY_???");
		tprints("]");
	}

	return RVAL_IOCTL_DECODED;
}

# ifdef EVIOCGKEYCODE_V2
static int
keycode_V2_ioctl(struct tcb *const tcp, const kernel_ulong_t arg)
{
	tprints(", ");

	struct input_keymap_entry ike;

	if (umove_or_printaddr(tcp, arg, &ike))
		return RVAL_IOCTL_DECODED;

	tprintf("{flags=%" PRIu8
		", len=%" PRIu8 ", ",
		ike.flags,
		ike.len);

	if (!abbrev(tcp)) {
		unsigned int i;

		tprintf("index=%" PRIu16 ", keycode=", ike.index);
		printxval_index(evdev_keycode, ike.keycode, "KEY_???");
		tprints(", scancode=[");
		for (i = 0; i < ARRAY_SIZE(ike.scancode); i++) {
			if (i > 0)
				tprints(", ");
			tprintf("%" PRIx8, ike.scancode[i]);
		}
		tprints("]");
	} else {
		tprints("...");
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}
# endif /* EVIOCGKEYCODE_V2 */

static int
getid_ioctl(struct tcb *const tcp, const kernel_ulong_t arg)
{
	tprints(", ");

	struct input_id id;

	if (!umove_or_printaddr(tcp, arg, &id))
		tprintf("{ID_BUS=%" PRIu16
			", ID_VENDOR=%" PRIu16
			", ID_PRODUCT=%" PRIu16
			", ID_VERSION=%" PRIu16 "}",
			id.bustype,
			id.vendor,
			id.product,
			id.version);

	return RVAL_IOCTL_DECODED;
}

static int
decode_bitset_(struct tcb *const tcp, const kernel_ulong_t arg,
	       const struct xlat decode_nr[], const unsigned int max_nr,
	       const char *const dflt, size_t decode_nr_size, enum xlat_type xt)
{
	tprints(", ");

	unsigned int size;
	if ((kernel_ulong_t) tcp->u_rval > max_nr / 8)
		size = max_nr;
	else
		size = tcp->u_rval * 8;
	char decoded_arg[size];

	if (umove_or_printaddr(tcp, arg, &decoded_arg))
		return RVAL_IOCTL_DECODED;

	tprints("[");

	int bit_displayed = 0;
	int i = next_set_bit(decoded_arg, 0, size);
	if (i < 0) {
		tprints(" 0 ");
	} else {
		printxval_dispatch(decode_nr, decode_nr_size, i, dflt, xt);

		while ((i = next_set_bit(decoded_arg, i + 1, size)) > 0) {
			if (abbrev(tcp) && bit_displayed >= 3) {
				tprints(", ...");
				break;
			}
			tprints(", ");
			printxval_dispatch(decode_nr, decode_nr_size, i, dflt,
					   xt);
			bit_displayed++;
		}
	}

	tprints("]");

	return RVAL_IOCTL_DECODED;
}

#define decode_bitset(tcp_, arg_, decode_nr_, max_nr_, dflt_, xt_) \
	decode_bitset_((tcp_), (arg_), (decode_nr_), (max_nr_), \
		       (dflt_), ARRAY_SIZE(decode_nr_), (xt_))

# ifdef EVIOCGMTSLOTS
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
# endif /* EVIOCGMTSLOTS */

# if defined EVIOCGREP || defined EVIOCSREP
static int
repeat_ioctl(struct tcb *const tcp, const kernel_ulong_t arg)
{
	tprints(", ");
	printpair_int(tcp, arg, "%u");
	return RVAL_IOCTL_DECODED;
}
# endif /* EVIOCGREP || EVIOCSREP */

static int
bit_ioctl(struct tcb *const tcp, const unsigned int ev_nr,
	  const kernel_ulong_t arg)
{
	switch (ev_nr) {
		case 0:
			return decode_bitset(tcp, arg, evdev_ev,
					     EV_MAX, "EV_???", XT_SORTED);
		case EV_KEY:
			return decode_bitset(tcp, arg, evdev_keycode,
					     KEY_MAX, "KEY_???", XT_INDEXED);
		case EV_REL:
			return decode_bitset(tcp, arg, evdev_relative_axes,
					     REL_MAX, "REL_???", XT_INDEXED);
		case EV_ABS:
			return decode_bitset(tcp, arg, evdev_abs,
					     ABS_MAX, "ABS_???", XT_INDEXED);
		case EV_MSC:
			return decode_bitset(tcp, arg, evdev_misc,
					     MSC_MAX, "MSC_???", XT_INDEXED);
		case EV_SW:
			return decode_bitset(tcp, arg, evdev_switch,
					     SW_MAX, "SW_???", XT_INDEXED);
		case EV_LED:
			return decode_bitset(tcp, arg, evdev_leds,
					     LED_MAX, "LED_???", XT_INDEXED);
		case EV_SND:
			return decode_bitset(tcp, arg, evdev_snd,
					     SND_MAX, "SND_???", XT_INDEXED);
		case EV_REP:
			return decode_bitset(tcp, arg, evdev_autorepeat,
					     REP_MAX, "REP_???", XT_INDEXED);
		case EV_FF:
			return decode_bitset(tcp, arg, evdev_ff_types,
					     FF_MAX, "FF_???", XT_SORTED);
		case EV_PWR:
			tprints(", ");
			printnum_int(tcp, arg, "%d");
			return RVAL_IOCTL_DECODED;
		case EV_FF_STATUS:
			return decode_bitset(tcp, arg, evdev_ff_status,
					     FF_STATUS_MAX, "FF_STATUS_???",
					     XT_INDEXED);
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
# ifdef EVIOCGREP
		case EVIOCGREP:
			return repeat_ioctl(tcp, arg);
# endif
		case EVIOCGKEYCODE:
			return keycode_ioctl(tcp, arg);
# ifdef EVIOCGKEYCODE_V2
		case EVIOCGKEYCODE_V2:
			return keycode_V2_ioctl(tcp, arg);
# endif
	}

	/* fixed-number variable-length commands */
	switch (_IOC_NR(code)) {
# ifdef EVIOCGMTSLOTS
		case _IOC_NR(EVIOCGMTSLOTS(0)):
			return mtslots_ioctl(tcp, code, arg);
# endif
		case _IOC_NR(EVIOCGNAME(0)):
		case _IOC_NR(EVIOCGPHYS(0)):
		case _IOC_NR(EVIOCGUNIQ(0)):
			tprints(", ");
			if (syserror(tcp))
				printaddr(arg);
			else
				printstrn(tcp, arg, tcp->u_rval);
			return RVAL_IOCTL_DECODED;
# ifdef EVIOCGPROP
		case _IOC_NR(EVIOCGPROP(0)):
			return decode_bitset(tcp, arg, evdev_prop,
					     INPUT_PROP_MAX, "PROP_???",
					     XT_INDEXED);
# endif
		case _IOC_NR(EVIOCGSND(0)):
			return decode_bitset(tcp, arg, evdev_snd,
					     SND_MAX, "SND_???", XT_INDEXED);
# ifdef EVIOCGSW
		case _IOC_NR(EVIOCGSW(0)):
			return decode_bitset(tcp, arg, evdev_switch,
					     SW_MAX, "SW_???", XT_INDEXED);
# endif
		case _IOC_NR(EVIOCGKEY(0)):
			return decode_bitset(tcp, arg, evdev_keycode,
					     KEY_MAX, "KEY_???", XT_INDEXED);
		case _IOC_NR(EVIOCGLED(0)):
			return decode_bitset(tcp, arg, evdev_leds,
					     LED_MAX, "LED_???", XT_INDEXED);
	}

	/* multi-number fixed-length commands */
	if ((_IOC_NR(code) & ~ABS_MAX) == _IOC_NR(EVIOCGABS(0)))
		return abs_ioctl(tcp, arg);

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
# ifdef EVIOCSREP
		case EVIOCSREP:
			return repeat_ioctl(tcp, arg);
# endif
		case EVIOCSKEYCODE:
			return keycode_ioctl(tcp, arg);
# ifdef EVIOCSKEYCODE_V2
		case EVIOCSKEYCODE_V2:
			return keycode_V2_ioctl(tcp, arg);
# endif
		case EVIOCRMFF:
			tprintf(", %d", (int) arg);
			return RVAL_IOCTL_DECODED;
		case EVIOCGRAB:
# ifdef EVIOCREVOKE
		case EVIOCREVOKE:
# endif
			tprintf(", %" PRI_klu, arg);
			return RVAL_IOCTL_DECODED;
# ifdef EVIOCSCLOCKID
		case EVIOCSCLOCKID:
			tprints(", ");
			printnum_int(tcp, arg, "%u");
			return RVAL_IOCTL_DECODED;
# endif
		default: {
			int rc = evdev_write_ioctl_mpers(tcp, code, arg);

			if (rc != RVAL_DECODED)
				return rc;
		}
	}

	/* multi-number fixed-length commands */
	if ((_IOC_NR(code) & ~ABS_MAX) == _IOC_NR(EVIOCSABS(0)))
		return abs_ioctl(tcp, arg);

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
