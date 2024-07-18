/*
 * Support for decoding of VT ioctl commands.
 *
 * Copyright (c) 2019-2021 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2019-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <linux/ioctl.h>
#include <linux/kd.h>
#include <linux/keyboard.h>

#include "print_fields.h"
#include "print_utils.h"

#include "xlat/kd_default_led_flags.h"
#include "xlat/kd_kbd_modes.h"
#include "xlat/kd_kbd_types.h"
#include "xlat/kd_keymap_flags.h"
#include "xlat/kd_key_tables.h"
#include "xlat/kd_key_types.h"
#include "xlat/kd_key_fn_keys.h"
#include "xlat/kd_key_fn_key_vals.h"
#include "xlat/kd_key_spec_keys.h"
#include "xlat/kd_key_pad_keys.h"
#include "xlat/kd_key_dead_keys.h"
#include "xlat/kd_key_cur_keys.h"
#include "xlat/kd_key_shift_keys.h"
#include "xlat/kd_key_ascii_keys.h"
#include "xlat/kd_key_lock_keys.h"
#include "xlat/kd_key_slock_keys.h"
#include "xlat/kd_key_brl_keys.h"
#include "xlat/kd_led_flags.h"
#include "xlat/kd_meta_vals.h"
#include "xlat/kd_modes.h"

#define XLAT_MACROS_ONLY
# include "xlat/kd_ioctl_cmds.h"
#undef XLAT_MACROS_ONLY

#ifdef HAVE_STRUCT_KBDIACRUC
typedef struct kbdiacruc struct_kbdiacruc;
#else
typedef struct {
	unsigned int diacr;
	unsigned int base;
	unsigned int result;
} struct_kbdiacruc;
#endif

#ifdef HAVE_STRUCT_KBDIACRSUC
typedef struct kbdiacrsuc struct_kbdiacrsuc;
#else
typedef struct {
	unsigned int kb_cnt;
	struct_kbdiacruc kbdiacruc[256];
} struct_kbdiacrsuc;
#endif

enum {
	KERNEL_PIT_TICK_RATE = 1193182,
	KERNEL_E_TABSZ = 256,
	KERNEL_MAX_DIACR = 256,
};

static int
kiocsound(struct tcb *const tcp, const kernel_ulong_t arg)
{
	unsigned int freq = arg ? KERNEL_PIT_TICK_RATE / arg : 0;

	tprint_arg_next();
	PRINT_VAL_U(arg);
	if (xlat_verbose(xlat_verbosity) != XLAT_STYLE_RAW) {
		if (freq)
			tprintf_comment("%u Hz", freq);
		else
			tprints_comment("off");
	}

	return RVAL_IOCTL_DECODED;
}

static int
kd_mk_tone(struct tcb *const tcp, const unsigned int arg)
{
	unsigned int ticks = arg >> 16;
	unsigned int count = arg & 0xFFFF;
	unsigned int freq = ticks && count ? KERNEL_PIT_TICK_RATE / count : 0;

	tprint_arg_next();
	tprint_flags_begin();
	if (ticks) {
		tprint_shift_begin();
		PRINT_VAL_U(ticks);
		tprint_shift();
		PRINT_VAL_U(16);
		tprint_shift_end();
		tprint_flags_or();
	}
	PRINT_VAL_U(count);
	tprint_flags_end();

	if (xlat_verbose(xlat_verbosity) != XLAT_STYLE_RAW) {
		if (freq)
			tprintf_comment("%u Hz, %u ms", freq, ticks);
		else
			tprints_comment("off");
	}

	return RVAL_IOCTL_DECODED;
}

static void
print_leds(struct tcb *const tcp, const kernel_ulong_t arg,
	   const bool get, const bool dflt)
{
	unsigned char val;

	if (get) {
		if (umove_or_printaddr(tcp, arg, &val))
			return;
	} else {
		val = arg;
	}

	if (get)
		tprint_indirect_begin();
	printflags(dflt ? kd_default_led_flags : kd_led_flags, val,
		   "LED_???");
	if (get)
		tprint_indirect_end();
}

static int
kd_leds(struct tcb *const tcp, const unsigned int code,
	const kernel_ulong_t arg)
{
	bool get = false;
	bool dflt = false;

	switch (code) {
	case KDGETLED:
	case KDGKBLED:
		get = true;
	}

	switch (code) {
	case KDGKBLED:
	case KDSKBLED:
		dflt = true;
	}

	if (entering(tcp)) {
		tprint_arg_next();

		if (get)
			return 0;
	}

	print_leds(tcp, arg, get, dflt);

	return RVAL_IOCTL_DECODED;
}

static int
kd_get_kb_type(struct tcb *const tcp, const kernel_ulong_t arg)
{
	unsigned char val;

	if (entering(tcp)) {
		tprint_arg_next();
		return 0;
	}

	if (umove_or_printaddr(tcp, arg, &val))
		return RVAL_IOCTL_DECODED;

	tprint_indirect_begin();
	printxval(kd_kbd_types, val, "KB_???");
	tprint_indirect_end();

	return RVAL_IOCTL_DECODED;
}

static int
kd_io(struct tcb *const tcp, kernel_ulong_t arg)
{
	enum { GPFIRST = 0x3b4, GPLAST = 0x3df };

	tprint_arg_next();
	PRINT_VAL_X(arg);

	if (arg >= GPFIRST && arg <= GPLAST
	    && xlat_verbose(xlat_verbosity) != XLAT_STYLE_RAW)
		tprintf_comment("GPFIRST + %" PRI_klu, arg - GPFIRST);

	return RVAL_IOCTL_DECODED;
}

static int
kd_set_mode(struct tcb *const tcp, const kernel_ulong_t arg)
{
	tprint_arg_next();
	printxval(kd_modes, arg, "KD_???");

	return RVAL_IOCTL_DECODED;
}

static int
kd_get_mode(struct tcb *const tcp, const kernel_ulong_t arg)
{
	unsigned int val;

	if (entering(tcp)) {
		tprint_arg_next();
		return 0;
	}

	if (umove_or_printaddr(tcp, arg, &val))
		return RVAL_IOCTL_DECODED;

	tprint_indirect_begin();
	printxval(kd_modes, val, "KD_???");
	tprint_indirect_end();

	return RVAL_IOCTL_DECODED;
}

static int
kd_screen_map(struct tcb *const tcp, const kernel_ulong_t arg, const bool get)
{
	if (entering(tcp)) {
		tprint_arg_next();

		if (get)
			return 0;
	}

	if (entering(tcp) || !syserror(tcp))
		printstr_ex(tcp, arg, KERNEL_E_TABSZ, QUOTE_FORCE_HEX);
	else
		printaddr(arg);

	return RVAL_IOCTL_DECODED;
}

static bool
print_scrmap_array_member(struct tcb *tcp, void *elem_buf,
			  size_t elem_size, void *data)
{
	unsigned short val = *(unsigned short *) elem_buf;

	if ((xlat_verbose(xlat_verbosity) != XLAT_STYLE_ABBREV) ||
	    ((val & ~UNI_DIRECT_MASK) != UNI_DIRECT_BASE))
		PRINT_VAL_X(val);

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW)
		return true;

	if ((val & ~UNI_DIRECT_MASK) == UNI_DIRECT_BASE)
		(xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE
			? tprintf_comment : tprintf_string)("UNI_DIRECT_BASE+%#hx",
							    val & UNI_DIRECT_MASK);

	return true;
}

static int
kd_uni_screen_map(struct tcb *const tcp, const kernel_ulong_t arg,
		  const bool get)
{
	unsigned short elem;

	if (entering(tcp)) {
		tprint_arg_next();

		if (get)
			return 0;
	}

	print_array(tcp, arg, KERNEL_E_TABSZ, &elem, sizeof(elem),
		    tfetch_mem, print_scrmap_array_member, 0);

	return RVAL_IOCTL_DECODED;
}

static int
kd_set_kbd_mode(struct tcb *const tcp, const unsigned int arg)
{
	tprint_arg_next();
	printxval_d(kd_kbd_modes, arg, "K_???");

	return RVAL_IOCTL_DECODED;
}

static int
kd_get_kbd_mode(struct tcb *const tcp, const kernel_ulong_t arg)
{
	unsigned int val;

	if (entering(tcp)) {
		tprint_arg_next();
		return 0;
	}

	if (umove_or_printaddr(tcp, arg, &val))
		return RVAL_IOCTL_DECODED;

	tprint_indirect_begin();
	printxval_d(kd_kbd_modes, val, "K_???");
	tprint_indirect_end();

	return RVAL_IOCTL_DECODED;
}

static int
kd_kbd_entry(struct tcb *const tcp, const kernel_ulong_t arg, const bool get)
{
	static const struct xlat *xlat_tables[] = {
		/* KT_LATIN */
		[KT_FN]    = kd_key_fn_keys,
		[KT_SPEC]  = kd_key_spec_keys,
		[KT_PAD]   = kd_key_pad_keys,
		[KT_DEAD]  = kd_key_dead_keys,
		/* KT_CONS */
		[KT_CUR]   = kd_key_cur_keys,
		[KT_SHIFT] = kd_key_shift_keys,
		/* KT_META */
		[KT_ASCII] = kd_key_ascii_keys,
		[KT_LOCK]  = kd_key_lock_keys,
		/* KT_LETTER */
		[KT_SLOCK] = kd_key_slock_keys,
		/* KT_DEAD2 */
		[KT_BRL]   = kd_key_brl_keys,
	};

	struct kbentry val;
	unsigned char ktyp;
	unsigned char kval;
	const char *str = NULL;

	if (entering(tcp)) {
		tprint_arg_next();

		if (umoven(tcp, arg, offsetofend(struct kbentry, kb_index),
			   &val)) {
			printaddr(arg);
			return RVAL_IOCTL_DECODED;
		}

		tprint_struct_begin();

		const char *keymap_str = xlookup(kd_key_tables, val.kb_table);

		if (keymap_str) {
			tprints_field_name("kb_table");
			print_xlat_ex(val.kb_table, keymap_str,
				      XLAT_STYLE_DEFAULT);
		} else {
			PRINT_FIELD_FLAGS(val, kb_table, kd_keymap_flags,
					  "K_???");
		}

		tprint_struct_next();
		PRINT_FIELD_U(val, kb_index);

		if (get)
			return 0;
	} else if (syserror(tcp)) {
		goto out;
	}

	tprint_struct_next();
	if (umove(tcp, arg + offsetof(struct kbentry, kb_value),
			 &val.kb_value)) {
		tprints_field_name("kb_value");
		tprint_unavailable();
		goto out;
	}

	PRINT_FIELD_X(val, kb_value);

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW)
		goto out;

	ktyp = KTYP(val.kb_value);
	kval = KVAL(val.kb_value);

	if (ktyp < ARRAY_SIZE(xlat_tables) && xlat_tables[ktyp])
		str = xlookup(xlat_tables[ktyp], val.kb_value);

	if (str) {
		tprints_comment(str);
	} else {
		tprint_comment_begin();
		tprints_arg_begin("K");
		printxvals_ex(ktyp, NULL, XLAT_STYLE_ABBREV,
			      kd_key_types, NULL);
		tprint_arg_next();

		switch (ktyp) {
		case KT_LATIN:
		case KT_META:
		case KT_LETTER:
		case KT_DEAD2:
			print_char(kval, SCF_QUOTES | SCF_ESC_WS);
			break;
		default:
			PRINT_VAL_X(kval);
		}

		tprint_arg_end();
		tprint_comment_end();
	}

out:
	tprint_struct_end();

	return RVAL_IOCTL_DECODED;
}

static int
kd_kbd_str_entry(struct tcb *const tcp, const kernel_ulong_t arg,
		 const bool get)
{
	struct kbsentry val;

	if (entering(tcp)) {
		tprint_arg_next();

		if (umove_or_printaddr(tcp, arg, &(val.kb_func)))
			return RVAL_IOCTL_DECODED;

		tprint_struct_begin();
		PRINT_FIELD_XVAL(val, kb_func, kd_key_fn_key_vals,
				 "KVAL(K_???"")");

		if (get)
			return 0;
	} else if (syserror(tcp)) {
		goto out;
	}

	tprint_struct_next();
	tprints_field_name("kb_string");
	printstr_ex(tcp, arg + offsetof(struct kbsentry, kb_string),
		    sizeof(val.kb_string), QUOTE_0_TERMINATED);

out:
	tprint_struct_end();

	return RVAL_IOCTL_DECODED;
}

static bool
print_kbdiacr_array_member(struct tcb *tcp, void *elem_buf,
			   size_t elem_size, void *data)
{
	struct kbdiacr *val = elem_buf;

	tprint_struct_begin();
	PRINT_FIELD_CHAR(*val, diacr, SCF_QUOTES | SCF_ESC_WS);
	tprint_struct_next();
	PRINT_FIELD_CHAR(*val, base, SCF_QUOTES | SCF_ESC_WS);
	tprint_struct_next();
	PRINT_FIELD_CHAR(*val, result, SCF_QUOTES | SCF_ESC_WS);
	tprint_struct_end();

	return true;
}

static int
kd_diacr(struct tcb *const tcp, const kernel_ulong_t arg, const bool get)
{
	unsigned int kb_cnt; /* struct kbdiacrs.kb_cnt */
	struct kbdiacr elem;

	if (entering(tcp)) {
		tprint_arg_next();

		if (get)
			return 0;
	}

	if (umove_or_printaddr(tcp, arg, &kb_cnt))
		return RVAL_IOCTL_DECODED;

	tprint_struct_begin();
	tprints_field_name("kb_cnt");
	PRINT_VAL_U(kb_cnt);
	tprint_struct_next();
	tprints_field_name("kbdiacr");
	print_array_ex(tcp, arg + offsetof(struct kbdiacrs, kbdiacr),
		       MIN(kb_cnt, KERNEL_MAX_DIACR), &elem, sizeof(elem),
		       tfetch_mem, print_kbdiacr_array_member, 0,
		       kb_cnt > KERNEL_MAX_DIACR ? PAF_ARRAY_TRUNCATED : 0,
		       NULL, NULL);
	tprint_struct_end();

	return RVAL_IOCTL_DECODED;
}

static bool
print_kbdiacruc_array_member(struct tcb *tcp, void *elem_buf,
			     size_t elem_size, void *data)
{
	struct_kbdiacruc *val = elem_buf;

	tprint_struct_begin();
	PRINT_FIELD_X(*val, diacr);
	tprint_struct_next();
	PRINT_FIELD_X(*val, base);
	tprint_struct_next();
	PRINT_FIELD_X(*val, result);
	tprint_struct_end();

	return true;
}

static int
kd_diacr_uc(struct tcb *const tcp, const kernel_ulong_t arg, const bool get)
{
	unsigned int kb_cnt; /* struct kbdiacrs.kb_cnt */
	struct_kbdiacruc elem;

	if (entering(tcp)) {
		tprint_arg_next();

		if (get)
			return 0;
	}

	if (umove_or_printaddr(tcp, arg, &kb_cnt))
		return RVAL_IOCTL_DECODED;

	tprint_struct_begin();
	tprints_field_name("kb_cnt");
	PRINT_VAL_U(kb_cnt);
	tprint_struct_next();
	tprints_field_name("kbdiacruc");
	print_array_ex(tcp, arg + offsetof(struct_kbdiacrsuc, kbdiacruc),
		       MIN(kb_cnt, KERNEL_MAX_DIACR), &elem, sizeof(elem),
		       tfetch_mem, print_kbdiacruc_array_member, 0,
		       kb_cnt > KERNEL_MAX_DIACR ? PAF_ARRAY_TRUNCATED : 0,
		       NULL, NULL);
	tprint_struct_end();

	return RVAL_IOCTL_DECODED;
}

static int
kd_keycode(struct tcb *const tcp, const kernel_ulong_t arg, const bool get)
{
	struct kbkeycode val;

	if (entering(tcp)) {
		tprint_arg_next();

		if (umove_or_printaddr(tcp, arg, &val))
			return RVAL_IOCTL_DECODED;

		tprint_struct_begin();
		PRINT_FIELD_X(val, scancode);
		tprint_struct_next();
		PRINT_FIELD_X(val, keycode);

		if (get)
			return 0;

		goto end;
	}

	/* exiting */
	if (syserror(tcp) ||
	    umove(tcp, arg + offsetof(struct kbkeycode, keycode), &val.keycode))
	{
		tprint_struct_end();
		return RVAL_IOCTL_DECODED;
	}

	tprint_value_changed();
	PRINT_VAL_X(val.keycode);

end:
	tprint_struct_end();

	return RVAL_IOCTL_DECODED;
}

static int
kd_sigaccept(struct tcb *const tcp, const kernel_ulong_t arg)
{
	tprint_arg_next();

	if (arg < INT_MAX)
		printsignal(arg);
	else
		PRINT_VAL_U(arg);

	return RVAL_IOCTL_DECODED;
}

static void
print_kbd_repeat(struct kbd_repeat *val)
{
	tprint_struct_begin();
	PRINT_FIELD_D(*val, delay);
	tprint_struct_next();
	PRINT_FIELD_D(*val, period);
	tprint_struct_end();
}

static int
kd_kbdrep(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct kbd_repeat val;

	if (entering(tcp)) {
		tprint_arg_next();

		if (umove_or_printaddr(tcp, arg, &val))
			return RVAL_IOCTL_DECODED;

		print_kbd_repeat(&val);

		return 0;
	}

	/* exiting */
	if (syserror(tcp) || umove(tcp, arg, &val))
		return RVAL_IOCTL_DECODED;

	tprint_value_changed();

	print_kbd_repeat(&val);

	return RVAL_IOCTL_DECODED;
}

static int
kd_font(struct tcb *const tcp, const kernel_ulong_t arg, const bool get)
{
	if (entering(tcp)) {
		tprint_arg_next();

		if (get)
			return 0;
	}

	/*
	 * [GP]IO_FONT are equivalent to KDFONTOP with width == 8,
	 * height == 32, and charcount == 256, so the total size
	 * is (width + 7) / 8 * height * charcount == 8192.
	 */
	if (exiting(tcp) && syserror(tcp))
		printaddr(arg);
	else
		printstr_ex(tcp, arg, 8192, QUOTE_FORCE_HEX);

	return RVAL_IOCTL_DECODED;
}

static int
kd_kbmeta(struct tcb *const tcp, const kernel_ulong_t arg, const bool get)
{
	unsigned int val;

	if (entering(tcp)) {
		tprint_arg_next();

		if (get)
			return 0;
	}

	if (get) {
		if (umove_or_printaddr(tcp, arg, &val))
			return RVAL_IOCTL_DECODED;
	} else {
		val = arg;
	}

	if (get)
		tprint_indirect_begin();
	printxval(kd_meta_vals, val, "K_???");
	if (get)
		tprint_indirect_end();

	return RVAL_IOCTL_DECODED;
}

static int
kd_unimapclr(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct unimapinit umi;

	tprint_arg_next();

	if (umove_or_printaddr(tcp, arg, &umi))
		return RVAL_IOCTL_DECODED;

	tprint_struct_begin();
	PRINT_FIELD_U(umi, advised_hashsize);
	tprint_struct_next();
	PRINT_FIELD_U(umi, advised_hashstep);
	tprint_struct_next();
	PRINT_FIELD_U(umi, advised_hashlevel);
	tprint_struct_end();

	return RVAL_IOCTL_DECODED;
}

static int
kd_cmap(struct tcb *const tcp, const kernel_ulong_t arg, const bool get)
{
	if (entering(tcp)) {
		tprint_arg_next();

		if (get)
			return 0;
	} else {
		if (syserror(tcp)) {
			printaddr(arg);

			return RVAL_IOCTL_DECODED;
		}
	}

	printstr_ex(tcp, arg, 3 * 16, QUOTE_FORCE_HEX);

	return RVAL_IOCTL_DECODED;
}

int
kd_ioctl(struct tcb *const tcp, const unsigned int code,
	 kernel_ulong_t arg)
{
	arg = truncate_kulong_to_current_wordsize(arg);

	switch (code) {
	case KIOCSOUND:
		return kiocsound(tcp, arg);

	case KDMKTONE:
		return kd_mk_tone(tcp, arg);

	case KDGETLED:
	case KDSETLED:
	case KDGKBLED:
	case KDSKBLED:
		return kd_leds(tcp, code, arg);

	case KDGKBTYPE:
		return kd_get_kb_type(tcp, arg);

	case KDADDIO:
	case KDDELIO:
		return kd_io(tcp, arg);

	case KDSETMODE:
		return kd_set_mode(tcp, arg);
	case KDGETMODE:
		return kd_get_mode(tcp, arg);

	case GIO_SCRNMAP:
	case PIO_SCRNMAP:
		return kd_screen_map(tcp, arg, code == GIO_SCRNMAP);

	case GIO_UNISCRNMAP:
	case PIO_UNISCRNMAP:
		return kd_uni_screen_map(tcp, arg, code == GIO_UNISCRNMAP);

	case KDGKBMODE:
		return kd_get_kbd_mode(tcp, arg);
	case KDSKBMODE:
		return kd_set_kbd_mode(tcp, arg);

	case KDGKBENT:
	case KDSKBENT:
		return kd_kbd_entry(tcp, arg, code == KDGKBENT);

	case KDGKBSENT:
	case KDSKBSENT:
		return kd_kbd_str_entry(tcp, arg, code == KDGKBSENT);

	case KDGKBDIACR:
	case KDSKBDIACR:
		return kd_diacr(tcp, arg, code == KDGKBDIACR);

	case KDGKBDIACRUC:
	case KDSKBDIACRUC:
		return kd_diacr_uc(tcp, arg, code == KDGKBDIACRUC);

	case KDGETKEYCODE:
	case KDSETKEYCODE:
		return kd_keycode(tcp, arg, code == KDGETKEYCODE);

	case KDSIGACCEPT:
		return kd_sigaccept(tcp, arg);

	case KDKBDREP:
		return kd_kbdrep(tcp, arg);

	case GIO_FONT:
	case PIO_FONT:
		return kd_font(tcp, arg, code == GIO_FONT);

	case KDGKBMETA:
	case KDSKBMETA:
		return kd_kbmeta(tcp, arg, code == KDGKBMETA);

	case PIO_UNIMAPCLR:
		return kd_unimapclr(tcp, arg);

	case GIO_CMAP:
	case PIO_CMAP:
		return kd_cmap(tcp, arg, code == GIO_CMAP);

	/* no arguments */
	case KDENABIO:
	case KDDISABIO:
	case KDMAPDISP:
	case KDUNMAPDISP:
	case PIO_FONTRESET:
		return RVAL_IOCTL_DECODED;
	}

	/* GIO_UNIMAP, PIO_UNIMAP, GIO_FONTX, PIO_FONTX, KDFONTOP */
	return kd_mpers_ioctl(tcp, code, arg);
}
