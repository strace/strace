/*
 * Copyright (c) 2021 Eugene Syromyatnikov <evgsyr@gmail.com>.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <linux/ioctl.h>
#include <linux/fb.h>

#include "xlat/fb_accel_flags.h"
#include "xlat/fb_activate_flags.h"
#include "xlat/fb_activate_vals.h"
#include "xlat/fb_sync_flags.h"
#include "xlat/fb_vblank_flags.h"
#include "xlat/fb_vmodes.h"
#include "xlat/fb_vmode_flags.h"

#define XLAT_MACROS_ONLY
# include "xlat/fb_ioctl_cmds.h"
#undef XLAT_MACROS_ONLY

static void
print_fb_bitfield(struct fb_bitfield *bf)
{
	tprint_struct_begin();
	PRINT_FIELD_U(*bf, offset);
	tprint_struct_next();
	PRINT_FIELD_U(*bf, length);
	tprint_struct_next();
	PRINT_FIELD_U(*bf, msb_right);
	tprint_struct_end();
}

static void
print_fb_activate(uint32_t val)
{
	uint32_t flags = val & ~FB_ACTIVATE_MASK;
	uint32_t mode = val & FB_ACTIVATE_MASK;

	printflags(fb_activate_flags, flags, NULL);
	if (flags)
		tprints("|");
	printxval(fb_activate_vals, mode, "FB_ACTIVATE_???");
}

static void
print_fb_vmode(uint32_t val)
{
	uint32_t flags = val & ~FB_VMODE_MASK;
	uint32_t mode = val & FB_VMODE_MASK;

	printflags(fb_vmode_flags, flags, NULL);
	if (flags)
		tprints("|");
	printflags(fb_vmodes, mode, "FB_VMODE_???");
}

static int
fb_vscreeninfo(struct tcb *const tcp, const kernel_ulong_t arg, const bool get)
{
	struct fb_var_screeninfo si;

	if (entering(tcp)) {
		tprint_arg_next();
		if (get)
			return 0;
	} else {
		if (!get) {
			if (syserror(tcp))
				return RVAL_IOCTL_DECODED;
			else
				tprint_value_changed();
		}
	}

	if (umove_or_printaddr(tcp, arg, &si))
		return RVAL_IOCTL_DECODED;

	tprint_struct_begin();
	PRINT_FIELD_U(si, xres);
	tprint_struct_next();
	PRINT_FIELD_U(si, yres);
	tprint_struct_next();
	PRINT_FIELD_U(si, xres_virtual);
	tprint_struct_next();
	PRINT_FIELD_U(si, yres_virtual);
	tprint_struct_next();
	PRINT_FIELD_U(si, xoffset);
	tprint_struct_next();
	PRINT_FIELD_U(si, yoffset);
	tprint_struct_next();
	PRINT_FIELD_U(si, bits_per_pixel);
	tprint_struct_next();
	if (si.grayscale > 1)
		PRINT_FIELD_PIXFMT(si, grayscale, v4l2_pix_fmts);
	else
		PRINT_FIELD_U(si, grayscale);
	tprint_struct_next();
	PRINT_FIELD_OBJ_PTR(si, red, print_fb_bitfield);
	tprint_struct_next();
	PRINT_FIELD_OBJ_PTR(si, green, print_fb_bitfield);
	tprint_struct_next();
	PRINT_FIELD_OBJ_PTR(si, blue, print_fb_bitfield);
	tprint_struct_next();
	PRINT_FIELD_OBJ_PTR(si, transp, print_fb_bitfield);
	tprint_struct_next();
	/*
	 * Unfortunately, despite presence and some usage of FB_NONSTD_*
	 * constants, the encoding of the field is actually varies depending
	 * on driver.
	 */
	PRINT_FIELD_X(si, nonstd);
	tprint_struct_next();
	PRINT_FIELD_OBJ_VAL(si, activate, print_fb_activate);
	tprint_struct_next();
	PRINT_FIELD_U(si, height);
	tprint_struct_next();
	PRINT_FIELD_U(si, width);
	tprint_struct_next();
	PRINT_FIELD_FLAGS(si, accel_flags, fb_accel_flags, "FB_ACCELF_???");
	tprint_struct_next();
	PRINT_FIELD_U(si, pixclock);
	tprint_struct_next();
	PRINT_FIELD_U(si, left_margin);
	tprint_struct_next();
	PRINT_FIELD_U(si, right_margin);
	tprint_struct_next();
	PRINT_FIELD_U(si, upper_margin);
	tprint_struct_next();
	PRINT_FIELD_U(si, lower_margin);
	tprint_struct_next();
	PRINT_FIELD_U(si, hsync_len);
	tprint_struct_next();
	PRINT_FIELD_U(si, vsync_len);
	tprint_struct_next();
	PRINT_FIELD_FLAGS(si, sync, fb_sync_flags, "FB_SYNC_???");
	tprint_struct_next();
	PRINT_FIELD_OBJ_VAL(si, vmode, print_fb_vmode);
	tprint_struct_next();
	PRINT_FIELD_U(si, rotate);
	tprint_struct_next();
	if (si.colorspace)
		PRINT_FIELD_PIXFMT(si, colorspace, v4l2_pix_fmts);
	else
		PRINT_FIELD_U(si, colorspace);
	if (!IS_ARRAY_ZERO(si.reserved)) {
		tprint_struct_next();
		PRINT_FIELD_X_ARRAY(si, reserved);
	}
	tprint_struct_end();

	return entering(tcp) ? 0 : RVAL_IOCTL_DECODED;
}

static int
fb_con2fbmap(struct tcb *const tcp, const kernel_ulong_t arg, const bool get)
{
	struct fb_con2fbmap cf;

	if (entering(tcp)) {
		tprint_arg_next();
		if (get)
			return 0;

		if (umove_or_printaddr(tcp, arg, &cf))
			return RVAL_IOCTL_DECODED;
	} else {
		if (!get && (syserror(tcp) || umove(tcp, arg, &cf))) {
			tprint_struct_end();
			return RVAL_IOCTL_DECODED;
		}
	}

	if (get || entering(tcp)) {
		tprint_struct_begin();
		PRINT_FIELD_U(cf, console);
		tprint_struct_next();

		if (!get)
			return 0;
	}

	PRINT_FIELD_U(cf, framebuffer);
	tprint_struct_end();

	return RVAL_IOCTL_DECODED;
}

static int
fb_vblank(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct fb_vblank vb;

	if (entering(tcp)) {
		tprint_arg_next();
		return 0;
	}

	if (umove_or_printaddr(tcp, arg, &vb))
		return RVAL_IOCTL_DECODED;

	tprint_struct_begin();
	PRINT_FIELD_FLAGS(vb, flags, fb_vblank_flags, "FB_VBLANK_???");
	tprint_struct_next();
	PRINT_FIELD_U(vb, count);
	tprint_struct_next();
	PRINT_FIELD_U(vb, vcount);
	tprint_struct_next();
	PRINT_FIELD_U(vb, hcount);
	tprint_struct_next();
	if (!IS_ARRAY_ZERO(vb.reserved)) {
		tprint_struct_next();
		PRINT_FIELD_X_ARRAY(vb, reserved);
	}

	return RVAL_IOCTL_DECODED;
}

static int
fb_da8xx_sync(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct lcd_sync_arg {
		int back_porch;
		int front_porch;
		int pulse_width;
	} sa;

	if (umove_or_printaddr(tcp, arg, &sa))
		return RVAL_IOCTL_DECODED;

	tprint_struct_begin();
	PRINT_FIELD_D(sa, back_porch);
	tprint_struct_next();
	PRINT_FIELD_D(sa, front_porch);
	tprint_struct_next();
	PRINT_FIELD_D(sa, pulse_width);
	tprint_struct_end();

	return RVAL_IOCTL_DECODED;
}

int
fb_ioctl(struct tcb *const tcp, const unsigned int code,
	     const kernel_ulong_t arg)
{
	switch (code) {
	case FBIOGET_VSCREENINFO:
	case FBIOPUT_VSCREENINFO:
	case FBIOPAN_DISPLAY:
		return fb_vscreeninfo(tcp, arg, code == FBIOGET_VSCREENINFO);

	case FBIOGET_CON2FBMAP:
	case FBIOPUT_CON2FBMAP:
		return fb_con2fbmap(tcp, arg, code == FBIOGET_CON2FBMAP);

	case FBIOBLANK:
		tprint_arg_next();
		PRINT_VAL_D(arg);
		return RVAL_IOCTL_DECODED;

	case FBIOGET_VBLANK:
		return fb_vblank(tcp, arg);

	case FBIO_WAITFORVSYNC:
		tprint_arg_next();
		printnum_int(tcp, arg, "%u");
		return RVAL_IOCTL_DECODED;

	/* The following two commands are from da8xx_fb.c */
	case FBIPUT_HSYNC:
	case FBIPUT_VSYNC:
		return fb_da8xx_sync(tcp, arg);

	/* The following two are from arcfb.c */
	case FBIO_WAITEVENT:
		return RVAL_IOCTL_DECODED;

	case FBIO_GETCONTROL2:
		if (entering(tcp)) {
			tprint_arg_next();
			return 0;
		}
		printnum_char(tcp, arg, "%hhu");
		return RVAL_IOCTL_DECODED;

	/* The following two are from sstfb.c */
	case SSTFB_SET_VGAPASS:
		tprint_arg_next();
		printnum_int(tcp, arg, "%u");
		return RVAL_IOCTL_DECODED;

	case SSTFB_GET_VGAPASS:
		if (entering(tcp)) {
			tprint_arg_next();
			return 0;
		}
		printnum_int(tcp, arg, "%u");
		return RVAL_IOCTL_DECODED;

	/* Unsupported */
	case FBIO_CURSOR:
	case FBIOGET_MONITORSPEC:
	case FBIOPUT_MONITORSPEC:
	case FBIOSWITCH_MONIBIT:

	/* These were implemented by pre-1.7.10 sisfb (v2.6.9-rc1~85^2~1^2~88) */
	case FBIO_ALLOC:
	case FBIO_FREE:
	case FBIOGET_GLYPH:
	case FBIOGET_HWCINFO:
	case FBIOPUT_MODEINFO:
	case FBIOGET_DISPINFO:

	/*
	 * These are declared in include/video/da8xx-fb.h, mentioned only
	 * in da8xx-fb.c, and never have an implementetion (always returned
	 * EINVAL/ENOTTY).
	 */
	case FBIOGET_CONTRAST:
	case FBIOPUT_CONTRAST:
	case FBIGET_BRIGHTNESS:
	case FBIPUT_BRIGHTNESS:
	case FBIGET_COLOR:
	case FBIPUT_COLOR:
		return RVAL_DECODED;
	}

	/* FBIOGET_FSCREENINFO, FBIOGETCMAP, FBIOPUTCMAP */
	return fb_mpers_ioctl(tcp, code, arg);
}
