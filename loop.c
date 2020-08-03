/*
 * Copyright (c) 2012 The Chromium OS Authors.
 * Copyright (c) 2012-2020 The strace developers.
 * Written by Mike Frysinger <vapier@gentoo.org>.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "types/loop.h"

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

	PRINT_FIELD_D("{", info, lo_number);

	if (!abbrev(tcp)) {
		PRINT_FIELD_DEV(", ", info, lo_device);
		PRINT_FIELD_U(", ", info, lo_inode);
		PRINT_FIELD_DEV(", ", info, lo_rdevice);
	}

	PRINT_FIELD_X(", ", info, lo_offset);

	if (!abbrev(tcp) || info.lo_encrypt_type != LO_CRYPT_NONE) {
		PRINT_FIELD_XVAL(", ", info, lo_encrypt_type,
				 loop_crypt_type_options, "LO_CRYPT_???");
		/*
		 * It is converted to unsigned before use in the kernel,
		 * see loop_info64_from_old in drivers/block/loop.c
		 */
		PRINT_FIELD_U(", ", info, lo_encrypt_key_size);
	}

	PRINT_FIELD_FLAGS(", ", info, lo_flags,
			  loop_flags_options, "LO_FLAGS_???");

	PRINT_FIELD_CSTRING(", ", info, lo_name);

	if (!abbrev(tcp) || info.lo_encrypt_type != LO_CRYPT_NONE) {
		const unsigned int lo_encrypt_key_size =
			MIN((unsigned) info.lo_encrypt_key_size, LO_KEY_SIZE);
		PRINT_FIELD_STRING(", ", info, lo_encrypt_key,
					  lo_encrypt_key_size, 0);
	}

	if (!abbrev(tcp)) {
		PRINT_FIELD_X_ARRAY(", ", info, lo_init);
		PRINT_FIELD_X_ARRAY(", ", info, reserved);
	} else {
		tprints(", ...");
	}

	tprints("}");
}

static void
print_loop_info64(struct tcb *const tcp, const struct loop_info64 *const info64)
{
	if (!abbrev(tcp)) {
		PRINT_FIELD_DEV("{", *info64, lo_device);
		PRINT_FIELD_U(", ", *info64, lo_inode);
		PRINT_FIELD_DEV(", ", *info64, lo_rdevice);
		PRINT_FIELD_X(", ", *info64, lo_offset);
		PRINT_FIELD_U(", ", *info64, lo_sizelimit);
		PRINT_FIELD_U(", ", *info64, lo_number);
	} else {
		PRINT_FIELD_X("{", *info64, lo_offset);
		PRINT_FIELD_U(", ", *info64, lo_number);
	}

	if (!abbrev(tcp) || info64->lo_encrypt_type != LO_CRYPT_NONE) {
		PRINT_FIELD_XVAL(", ", *info64, lo_encrypt_type,
				 loop_crypt_type_options, "LO_CRYPT_???");
		PRINT_FIELD_U(", ", *info64, lo_encrypt_key_size);
	}

	PRINT_FIELD_FLAGS(", ", *info64, lo_flags,
			  loop_flags_options, "LO_FLAGS_???");

	PRINT_FIELD_CSTRING(", ", *info64, lo_file_name);

	if (!abbrev(tcp) || info64->lo_encrypt_type != LO_CRYPT_NONE) {
		PRINT_FIELD_CSTRING(", ", *info64, lo_crypt_name);
		const unsigned int lo_encrypt_key_size =
			MIN((unsigned) info64->lo_encrypt_key_size, LO_KEY_SIZE);
		PRINT_FIELD_STRING(", ", *info64, lo_encrypt_key,
					  lo_encrypt_key_size, 0);
	}

	if (!abbrev(tcp))
		PRINT_FIELD_X_ARRAY(", ", *info64, lo_init);
	else
		tprints(", ...");

	tprints("}");
}

static void
decode_loop_info64(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct loop_info64 info64;

	tprints(", ");
	if (!umove_or_printaddr(tcp, addr, &info64))
		print_loop_info64(tcp, &info64);
}

static void
decode_loop_config(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct_loop_config config;

	tprints(", ");
	if (umove_or_printaddr(tcp, addr, &config))
		return;

	PRINT_FIELD_FD("{", config, fd, tcp);

	PRINT_FIELD_U(", ", config, block_size);

	tprints(", info=");
	print_loop_info64(tcp, &config.info);

	if (!IS_ARRAY_ZERO(config.__reserved))
		PRINT_FIELD_X_ARRAY(", ", config, __reserved);

	tprints("}");
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

	case LOOP_CONFIGURE:
		decode_loop_config(tcp, arg);
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
