/*
 * Support for decoding of personality-dependent VT ioctl commands.
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

#include DEF_MPERS_TYPE(struct_unimapdesc)
#include DEF_MPERS_TYPE(struct_consolefontdesc)
#include DEF_MPERS_TYPE(struct_console_font)
#include DEF_MPERS_TYPE(struct_console_font_op)

typedef struct unimapdesc struct_unimapdesc;
typedef struct consolefontdesc struct_consolefontdesc;
typedef struct console_font struct_console_font;
typedef struct console_font_op struct_console_font_op;

#include MPERS_DEFS

#include "print_fields.h"

#include "xlat/kd_font_flags.h"
#include "xlat/kd_font_ops.h"

#define XLAT_MACROS_ONLY
# include "xlat/kd_ioctl_cmds.h"
#undef XLAT_MACROS_ONLY


static bool
print_unipair_array_member(struct tcb *tcp, void *elem_buf,
			   size_t elem_size, void *data)
{
	struct unipair *val = elem_buf;

	tprint_struct_begin();
	PRINT_FIELD_X(*val, unicode);
	tprint_struct_next();
	PRINT_FIELD_X(*val, fontpos);
	tprint_struct_end();

	return true;
}

static int
kd_unimap(struct tcb *const tcp, const kernel_ulong_t arg, const bool get)
{
	struct_unimapdesc val;
	struct unipair elem;
	uint16_t cnt;

	if (entering(tcp))
		tprint_arg_next();

	if (umove_or_printaddr_ignore_syserror(tcp, arg, &val)) {
		if (exiting(tcp))
			tprint_struct_end();

		return RVAL_IOCTL_DECODED;
	}

	if (entering(tcp)) {
		set_tcb_priv_ulong(tcp, val.entry_ct);
		tprint_struct_begin();
		PRINT_FIELD_U(val, entry_ct);

		if (get)
			return 0;
	} else {
		tprint_value_changed();
		PRINT_VAL_U(val.entry_ct);
	}

	if (exiting(tcp) && syserror(tcp) && tcp->u_error != ENOMEM) {
		tprint_struct_next();
		PRINT_FIELD_PTR(val, entries);
		tprint_struct_end();
		return RVAL_IOCTL_DECODED;
	}

	cnt = entering(tcp) ? val.entry_ct : get_tcb_priv_ulong(tcp);

	tprint_struct_next();
	tprints_field_name("entries");
	print_array(tcp, (mpers_ptr_t) val.entries, cnt, &elem, sizeof(elem),
		    tfetch_mem, print_unipair_array_member, 0);

	tprint_struct_end();

	return get && entering(tcp) ? 0 : RVAL_IOCTL_DECODED;
}

static void
print_consolefontdesc(struct tcb *const tcp, const struct_consolefontdesc *cfd,
		      const bool get)
{
	tprint_struct_begin();
	PRINT_FIELD_U(*cfd, charcount);
	tprint_struct_next();
	PRINT_FIELD_U(*cfd, charheight);
	tprint_struct_next();
	if (get) {
		PRINT_FIELD_PTR(*cfd, chardata);
	} else {
		tprints_field_name("chardata");
		printstr_ex(tcp, (mpers_ptr_t) cfd->chardata,
			    MIN(cfd->charcount, 512) * 32, QUOTE_FORCE_HEX);
	}

	tprint_struct_end();
}

static int
kd_fontx(struct tcb *const tcp, const kernel_ulong_t arg, const bool get)
{
	struct_consolefontdesc val;

	if (entering(tcp)) {
		tprint_arg_next();

		if (umove_or_printaddr(tcp, arg, &val))
			return RVAL_IOCTL_DECODED;
	} else {
		if (syserror(tcp) || umove(tcp, arg, &val))
			return RVAL_IOCTL_DECODED;

		tprint_value_changed();
	}

	print_consolefontdesc(tcp, &val, get && entering(tcp));

	return get && entering(tcp) ? 0 : RVAL_IOCTL_DECODED;
}

static void
print_console_font_op(struct tcb *const tcp, const struct_console_font_op *cfo)
{
	enum { KERNEL_MAX_FONT_NAME = 32 };

	tprint_struct_begin();

	if (entering(tcp)) {
		PRINT_FIELD_XVAL(*cfo, op, kd_font_ops, "KD_FONT_OP_???");

		switch (cfo->op) {
		case KD_FONT_OP_SET_DEFAULT:
		case KD_FONT_OP_COPY:
			break;
		default:
			tprint_struct_next();
			PRINT_FIELD_FLAGS(*cfo, flags, kd_font_flags,
					  "KD_FONT_FLAG_???");
		}

		tprint_struct_next();
	}

	switch (cfo->op) {
	case KD_FONT_OP_COPY:
		PRINT_FIELD_U(*cfo, height);
		break;
	default:
		PRINT_FIELD_U(*cfo, width);
		tprint_struct_next();
		PRINT_FIELD_U(*cfo, height);
	}

	switch (cfo->op) {
	case KD_FONT_OP_SET_DEFAULT:
	case KD_FONT_OP_COPY:
		break;
	default:
		tprint_struct_next();
		PRINT_FIELD_U(*cfo, charcount);
	}

	switch (cfo->op) {
	case KD_FONT_OP_GET:
		if (entering(tcp)) {
			tprint_struct_next();
			PRINT_FIELD_PTR(*cfo, data);
			break;
		}
		ATTRIBUTE_FALLTHROUGH;

	case KD_FONT_OP_SET:
		tprint_struct_next();
		tprints_field_name("data");
		printstr_ex(tcp, (mpers_ptr_t) cfo->data,
			    ROUNDUP_DIV(MIN(cfo->width, 32), 8) * 32 *
				MIN(cfo->charcount, 512),
			    QUOTE_FORCE_HEX);
		break;

	case KD_FONT_OP_SET_DEFAULT:
		if (entering(tcp)) {
			tprint_struct_next();
			tprints_field_name("data");
			printstr_ex(tcp, (mpers_ptr_t) cfo->data,
				    KERNEL_MAX_FONT_NAME,
				    QUOTE_0_TERMINATED
				    | QUOTE_EXPECT_TRAILING_0);
		}
		break;

	case KD_FONT_OP_COPY:
		break;

	default:
		tprint_struct_next();
		PRINT_FIELD_PTR(*cfo, data);
	}

	tprint_struct_end();
}

static int
kd_font_op(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_console_font_op val;

	if (entering(tcp)) {
		tprint_arg_next();

		if (umove_or_printaddr(tcp, arg, &val))
			return RVAL_IOCTL_DECODED;
	} else {
		if (syserror(tcp) || umove(tcp, arg, &val))
			return RVAL_IOCTL_DECODED;

		tprint_value_changed();
	}

	print_console_font_op(tcp, &val);

	switch (val.op) {
	case KD_FONT_OP_SET:
	case KD_FONT_OP_COPY:
		return RVAL_IOCTL_DECODED;
	}

	return entering(tcp) ? 0 : RVAL_IOCTL_DECODED;
}

MPERS_PRINTER_DECL(int, kd_mpers_ioctl, struct tcb *const tcp,
		   const unsigned int code, const kernel_ulong_t arg)
{
	switch (code)
	{
	case GIO_UNIMAP:
	case PIO_UNIMAP:
		return kd_unimap(tcp, arg, code == GIO_UNIMAP);

	case GIO_FONTX:
	case PIO_FONTX:
		return kd_fontx(tcp, arg, code == GIO_FONTX);

	case KDFONTOP:
		return kd_font_op(tcp, arg);
	}

	return RVAL_DECODED;
}
