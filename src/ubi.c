/*
 * Copyright (c) 2012 Mike Frysinger <vapier@gentoo.org>
 * Copyright (c) 2012-2024 The strace developers.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <linux/ioctl.h>
#include <mtd/ubi-user.h>

#include "xlat/ubi_volume_types.h"
#include "xlat/ubi_volume_flags.h"
#include "xlat/ubi_volume_props.h"
#include "xlat/ubi_data_types.h"

static int
decode_UBI_IOCMKVOL(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct ubi_mkvol_req mkvol;

	if (entering(tcp)) {
		tprint_arg_next();
		if (umove_or_printaddr(tcp, arg, &mkvol))
			return RVAL_IOCTL_DECODED;

		tprint_struct_begin();
		PRINT_FIELD_D(mkvol, vol_id);
		tprint_struct_next();
		PRINT_FIELD_D(mkvol, alignment);
		tprint_struct_next();
		PRINT_FIELD_D(mkvol, bytes);
		tprint_struct_next();
		PRINT_FIELD_XVAL(mkvol, vol_type,
				 ubi_volume_types, "UBI_???_VOLUME");
		tprint_struct_next();
		PRINT_FIELD_FLAGS(mkvol, flags,
				  ubi_volume_flags, "UBI_VOL_???");
		tprint_struct_next();
		PRINT_FIELD_D(mkvol, name_len);
		tprint_struct_next();
		PRINT_FIELD_CSTRING_SZ(mkvol, name,
				       1 + CLAMP(mkvol.name_len, 0,
						 (int) sizeof(mkvol.name) - 1));
		tprint_struct_end();
		return 0;
	}

	if (!syserror(tcp)) {
		tprint_value_changed();
		printnum_int(tcp, arg, "%d");
	}

	return RVAL_IOCTL_DECODED;
}

static int
decode_UBI_IOCRSVOL(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct ubi_rsvol_req rsvol;

	tprint_arg_next();
	if (!umove_or_printaddr(tcp, arg, &rsvol)) {
		tprint_struct_begin();
		PRINT_FIELD_D(rsvol, bytes);
		tprint_struct_next();
		PRINT_FIELD_D(rsvol, vol_id);
		tprint_struct_end();
	}

	return RVAL_IOCTL_DECODED;
}

static bool
print_ubi_rnvol_req_ent_array_member(struct tcb *tcp, void *elem_buf,
				     size_t elem_size, void *data)
{
	typeof(&((struct ubi_rnvol_req *) NULL)->ents[0]) p = elem_buf;

	tprint_struct_begin();
	PRINT_FIELD_D(*p, vol_id);
	tprint_struct_next();
	PRINT_FIELD_D(*p, name_len);
	tprint_struct_next();
	PRINT_FIELD_CSTRING_SZ(*p, name,
			       1 + CLAMP(p->name_len, 0,
				         (int) sizeof(p->name) - 1));
	tprint_struct_end();

	return true;
}

static int
decode_UBI_IOCRNVOL(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct ubi_rnvol_req rnvol;

	tprint_arg_next();
	if (!umove_or_printaddr(tcp, arg, &rnvol)) {
		tprint_struct_begin();
		PRINT_FIELD_D(rnvol, count);
		tprint_struct_next();
		PRINT_FIELD_ARRAY_UPTO(rnvol, ents, rnvol.count, tcp,
				       print_ubi_rnvol_req_ent_array_member);
		tprint_struct_end();
	}

	return RVAL_IOCTL_DECODED;
}

static int
decode_UBI_IOCEBCH(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct ubi_leb_change_req leb;

	tprint_arg_next();
	if (!umove_or_printaddr(tcp, arg, &leb)) {
		tprint_struct_begin();
		PRINT_FIELD_D(leb, lnum);
		tprint_struct_next();
		PRINT_FIELD_D(leb, bytes);
		tprint_struct_next();
		PRINT_FIELD_XVAL(leb, dtype, ubi_data_types, "UBI_???");
		tprint_struct_end();
	}

	return RVAL_IOCTL_DECODED;
}

static int
decode_UBI_IOCATT(struct tcb *const tcp, const kernel_ulong_t arg)
{
	if (entering(tcp)) {
		struct ubi_attach_req attach;

		tprint_arg_next();
		if (umove_or_printaddr(tcp, arg, &attach))
			return RVAL_IOCTL_DECODED;

		tprint_struct_begin();
		PRINT_FIELD_D(attach, ubi_num);
		tprint_struct_next();
		PRINT_FIELD_D(attach, mtd_num);
		tprint_struct_next();
		PRINT_FIELD_D(attach, vid_hdr_offset);
		tprint_struct_next();
		PRINT_FIELD_D(attach, max_beb_per1024);
		tprint_struct_next();
		PRINT_FIELD_D(attach, disable_fm);
		tprint_struct_next();
		PRINT_FIELD_D(attach, need_resv_pool);
		tprint_struct_end();
		return 0;
	}

	if (!syserror(tcp)) {
		tprint_value_changed();
		printnum_int(tcp, arg, "%d");
	}

	return RVAL_IOCTL_DECODED;
}

static int
decode_UBI_IOCEBMAP(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct ubi_map_req map;

	tprint_arg_next();
	if (!umove_or_printaddr(tcp, arg, &map)) {
		tprint_struct_begin();
		PRINT_FIELD_D(map, lnum);
		tprint_struct_next();
		PRINT_FIELD_XVAL(map, dtype, ubi_data_types, "UBI_???");
		tprint_struct_end();
	}

	return RVAL_IOCTL_DECODED;
}

static int
decode_UBI_IOCSETVOLPROP(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct ubi_set_vol_prop_req prop;

	tprint_arg_next();
	if (!umove_or_printaddr(tcp, arg, &prop)) {
		tprint_struct_begin();
		PRINT_FIELD_XVAL(prop, property,
				 ubi_volume_props, "UBI_VOL_PROP_???");
		tprint_struct_next();
		PRINT_FIELD_X(prop, value);
		tprint_struct_end();
	}

	return RVAL_IOCTL_DECODED;
}

int
ubi_ioctl(struct tcb *const tcp, const unsigned int code,
	  const kernel_ulong_t arg)
{
	switch (code) {
#define case_UBI(name)  case UBI_ ## name: return decode_UBI_ ## name(tcp, arg)
	case_UBI(IOCATT);
	case_UBI(IOCEBCH);
	case_UBI(IOCEBMAP);
	case_UBI(IOCMKVOL);
	case_UBI(IOCRNVOL);
	case_UBI(IOCRSVOL);
	case_UBI(IOCSETVOLPROP);
#undef case_UBI

	case UBI_IOCVOLUP:
		tprint_arg_next();
		printnum_int64(tcp, arg, "%" PRIi64);
		break;

	case UBI_IOCDET:
	case UBI_IOCEBER:
	case UBI_IOCEBISMAP:
	case UBI_IOCEBUNMAP:
	case UBI_IOCRMVOL:
	case UBI_IOCRPEB:
	case UBI_IOCSPEB:
		tprint_arg_next();
		printnum_int(tcp, arg, "%d");
		break;

	case UBI_IOCVOLCRBLK:
	case UBI_IOCVOLRMBLK:
		/* no arguments */
		break;

	default:
		return RVAL_DECODED;
	}

	return RVAL_IOCTL_DECODED;
}
