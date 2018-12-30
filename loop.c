/*
 * Copyright (c) 2012 The Chromium OS Authors.
 * Copyright (c) 2012-2018 The strace developers.
 * Written by Mike Frysinger <vapier@gentoo.org>.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/ioctl.h>
#include <linux/loop.h>

typedef struct loop_info struct_loop_info;

#include DEF_MPERS_TYPE(struct_loop_info)

#include MPERS_DEFS

#include "print_fields.h"

#define XLAT_MACROS_ONLY
#include "xlat/loop_cmds.h"
#undef XLAT_MACROS_ONLY

#include "xlat/loop_flags_options.h"
#include "xlat/loop_crypt_type_options.h"

static void
decode_loop_info(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct_loop_info info;

	tprints(", ");
	if (umove_or_printaddr(tcp, addr, &info))
		return;

	tprintf("{lo_number=%d", info.lo_number);

	if (!abbrev(tcp)) {
		PRINT_FIELD_DEV(", ", info, lo_device);
		tprintf(", lo_inode=%" PRI_klu, (kernel_ulong_t) info.lo_inode);
		PRINT_FIELD_DEV(", ", info, lo_rdevice);
	}

	tprintf(", lo_offset=%#x", info.lo_offset);

	if (!abbrev(tcp) || info.lo_encrypt_type != LO_CRYPT_NONE) {
		tprints(", lo_encrypt_type=");
		printxval(loop_crypt_type_options, info.lo_encrypt_type,
			"LO_CRYPT_???");
		/*
		 * It is converted to unsigned before use in kernel, see
		 * loop_info64_from_old in drivers/block/loop.c
		 */
		tprintf(", lo_encrypt_key_size=%" PRIu32,
			(uint32_t) info.lo_encrypt_key_size);
	}

	tprints(", lo_flags=");
	printflags(loop_flags_options, info.lo_flags, "LO_FLAGS_???");

	PRINT_FIELD_CSTRING(", ", info, lo_name);

	if (!abbrev(tcp) || info.lo_encrypt_type != LO_CRYPT_NONE) {
		const unsigned int lo_encrypt_key_size =
			MIN((unsigned) info.lo_encrypt_key_size, LO_KEY_SIZE);
		PRINT_FIELD_STRING(", ", info, lo_encrypt_key,
					  lo_encrypt_key_size, 0);
	}

	if (!abbrev(tcp))
		tprintf(", lo_init=[%#" PRI_klx ", %#" PRI_klx "]"
			", reserved=[%#hhx, %#hhx, %#hhx, %#hhx]}",
			(kernel_ulong_t) info.lo_init[0],
			(kernel_ulong_t) info.lo_init[1],
			info.reserved[0], info.reserved[1],
			info.reserved[2], info.reserved[3]);
	else
		tprints(", ...}");
}

static void
decode_loop_info64(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct loop_info64 info64;

	tprints(", ");
	if (umove_or_printaddr(tcp, addr, &info64))
		return;

	if (!abbrev(tcp)) {
		PRINT_FIELD_DEV("{", info64, lo_device);
		tprintf(", lo_inode=%" PRIu64, (uint64_t) info64.lo_inode);
		PRINT_FIELD_DEV(", ", info64, lo_rdevice);
		tprintf(", lo_offset=%#" PRIx64 ", lo_sizelimit=%" PRIu64
			", lo_number=%" PRIu32,
			(uint64_t) info64.lo_offset,
			(uint64_t) info64.lo_sizelimit,
			(uint32_t) info64.lo_number);
	} else {
		tprintf("{lo_offset=%#" PRIx64 ", lo_number=%" PRIu32,
			(uint64_t) info64.lo_offset,
			(uint32_t) info64.lo_number);
	}

	if (!abbrev(tcp) || info64.lo_encrypt_type != LO_CRYPT_NONE) {
		tprints(", lo_encrypt_type=");
		printxval(loop_crypt_type_options, info64.lo_encrypt_type,
			"LO_CRYPT_???");
		tprintf(", lo_encrypt_key_size=%" PRIu32,
			info64.lo_encrypt_key_size);
	}

	tprints(", lo_flags=");
	printflags(loop_flags_options, info64.lo_flags, "LO_FLAGS_???");

	PRINT_FIELD_CSTRING(", ", info64, lo_file_name);

	if (!abbrev(tcp) || info64.lo_encrypt_type != LO_CRYPT_NONE) {
		PRINT_FIELD_CSTRING(", ", info64, lo_crypt_name);
		const unsigned int lo_encrypt_key_size =
			MIN((unsigned) info64.lo_encrypt_key_size, LO_KEY_SIZE);
		PRINT_FIELD_STRING(", ", info64, lo_encrypt_key,
					  lo_encrypt_key_size, 0);
	}

	if (!abbrev(tcp))
		tprintf(", lo_init=[%#" PRIx64 ", %#" PRIx64 "]}",
			(uint64_t) info64.lo_init[0],
			(uint64_t) info64.lo_init[1]);
	else
		tprints(", ...}");
}

MPERS_PRINTER_DECL(int, loop_ioctl,
		   struct tcb *tcp, const unsigned int code,
		   const kernel_ulong_t arg)
{
	switch (code) {
	case LOOP_GET_STATUS:
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;
	case LOOP_SET_STATUS:
		decode_loop_info(tcp, arg);
		break;

	case LOOP_GET_STATUS64:
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;
	case LOOP_SET_STATUS64:
		decode_loop_info64(tcp, arg);
		break;

	case LOOP_CLR_FD:
	case LOOP_SET_CAPACITY:
	/* newer loop-control stuff */
	case LOOP_CTL_GET_FREE:
		/* Takes no arguments */
		break;

	case LOOP_SET_FD:
	case LOOP_CHANGE_FD:
		tprints(", ");
		printfd(tcp, arg);
		break;

	/* newer loop-control stuff */
	case LOOP_CTL_ADD:
	case LOOP_CTL_REMOVE:
		tprintf(", %d", (int) arg);
		break;

	case LOOP_SET_DIRECT_IO:
	case LOOP_SET_BLOCK_SIZE:
		tprintf(", %" PRI_klu, arg);
		break;

	default:
		return RVAL_DECODED;
	}

	return RVAL_IOCTL_DECODED;
}
