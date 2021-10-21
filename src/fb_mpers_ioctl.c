/*
 * Copyright (c) 2021 Eugene Syromyatnikov <evgsyr@gmail.com>.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <linux/ioctl.h>
#include <linux/fb.h>

#include DEF_MPERS_TYPE(struct_fb_cmap)
#include DEF_MPERS_TYPE(struct_fb_fix_screeninfo)

typedef struct fb_cmap struct_fb_cmap;
typedef struct fb_fix_screeninfo struct_fb_fix_screeninfo;

#include MPERS_DEFS

#include "xlat/fb_accels.h"
#include "xlat/fb_caps.h"
#include "xlat/fb_types.h"
#include "xlat/fb_type_aux_text.h"
#include "xlat/fb_type_aux_vga_planes.h"
#include "xlat/fb_visuals.h"

#define XLAT_MACROS_ONLY
# include "xlat/fb_ioctl_cmds.h"
#undef XLAT_MACROS_ONLY

static int
fb_fscreeninfo(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_fb_fix_screeninfo si;

	if (entering(tcp)) {
		tprint_arg_next();
		return 0;
	}

	if (umove_or_printaddr(tcp, arg, &si))
		return RVAL_IOCTL_DECODED;

	tprint_struct_begin();
	PRINT_FIELD_CSTRING(si, id);
	tprint_struct_next();
	PRINT_FIELD_PTR(si, smem_start);
	tprint_struct_next();
	PRINT_FIELD_X(si, smem_len);
	tprint_struct_next();
	PRINT_FIELD_XVAL(si, type, fb_types, "FB_TYPE_???");
	tprint_struct_next();
	switch (si.type) {
	case FB_TYPE_TEXT:
		PRINT_FIELD_XVAL(si, type_aux, fb_type_aux_text,
				 "FB_AUX_TEXT_???");
		break;
	case FB_TYPE_VGA_PLANES:
		PRINT_FIELD_XVAL(si, type_aux, fb_type_aux_vga_planes,
				 "FB_AUX_VGA_PLANES_???");
		break;
	default:
		PRINT_FIELD_X(si, type_aux);
	}
	tprint_struct_next();
	PRINT_FIELD_XVAL(si, visual, fb_visuals, "FB_VISUAL_???");
	tprint_struct_next();
	PRINT_FIELD_U(si, xpanstep);
	tprint_struct_next();
	PRINT_FIELD_U(si, ypanstep);
	tprint_struct_next();
	PRINT_FIELD_U(si, ywrapstep);
	tprint_struct_next();
	PRINT_FIELD_U(si, line_length);
	tprint_struct_next();
	PRINT_FIELD_PTR(si, mmio_start);
	tprint_struct_next();
	PRINT_FIELD_X(si, mmio_len);
	tprint_struct_next();
	PRINT_FIELD_XVAL(si, accel, fb_accels, "FB_ACCEL_???");
	tprint_struct_next();
	PRINT_FIELD_FLAGS(si, capabilities, fb_caps, "FB_CAP_???");
	if (!IS_ARRAY_ZERO(si.reserved)) {
		tprint_struct_next();
		PRINT_FIELD_X_ARRAY(si, reserved);
	}
	tprint_struct_end();

	return RVAL_IOCTL_DECODED;
}

#define PRINT_FIELD_U16_ARR(where_, field_, tcp_, len_, print_func_)	\
	do {								\
		uint16_t elem_;						\
		tprints_field_name(#field_);				\
		print_array((tcp_), (mpers_ptr_t) (where_).field_,	\
			    (len_), &elem_, sizeof(elem_),		\
			    tfetch_mem, (print_func_), NULL);		\
	} while (0)

static void
print_fb_cmap(struct tcb *const tcp, const struct_fb_cmap *const cm,
	      const bool print_arrays)
{
	tprint_struct_begin();
	PRINT_FIELD_U(*cm, start);
	tprint_struct_next();
	PRINT_FIELD_U(*cm, len);
	tprint_struct_next();
	if (print_arrays) {
		PRINT_FIELD_U16_ARR(*cm, red, tcp, cm->len,
				    print_uint_array_member);
		tprint_struct_next();
		PRINT_FIELD_U16_ARR(*cm, green, tcp, cm->len,
				    print_uint_array_member);
		tprint_struct_next();
		PRINT_FIELD_U16_ARR(*cm, blue, tcp, cm->len,
				     print_uint_array_member);
		tprint_struct_next();
		PRINT_FIELD_U16_ARR(*cm, transp, tcp, cm->len,
				    print_uint_array_member);
	} else {
		PRINT_FIELD_PTR(*cm, red);
		tprint_struct_next();
		PRINT_FIELD_PTR(*cm, green);
		tprint_struct_next();
		PRINT_FIELD_PTR(*cm, blue);
		tprint_struct_next();
		PRINT_FIELD_PTR(*cm, transp);
	}
	tprint_struct_end();
}

static int
fb_cmap(struct tcb *const tcp, const kernel_ulong_t arg, const bool get)
{
	struct_fb_cmap cm;

	if (entering(tcp)) {
		tprint_arg_next();
		if (get) {
			if (umove_or_printaddr(tcp, arg, &cm))
				return RVAL_IOCTL_DECODED;

			print_fb_cmap(tcp, &cm, false);
			return 0;
		}
	}

	if (get) {
		if (syserror(tcp) || umove(tcp, arg, &cm))
			return RVAL_IOCTL_DECODED;
		else
			tprint_value_changed();
	} else {
		if (umove_or_printaddr(tcp, arg, &cm))
			return RVAL_IOCTL_DECODED;
	}

	print_fb_cmap(tcp, &cm, true);

	return RVAL_IOCTL_DECODED;

}

MPERS_PRINTER_DECL(int, fb_mpers_ioctl, struct tcb *const tcp,
		   const unsigned int code, const kernel_ulong_t arg)
{
	switch (code) {
	case FBIOGET_FSCREENINFO:
		return fb_fscreeninfo(tcp, arg);

	case FBIOGETCMAP:
	case FBIOPUTCMAP:
		return fb_cmap(tcp, arg, code == FBIOGETCMAP);
	}

	return RVAL_DECODED;
}
