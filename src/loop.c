/*
 * Copyright (c) 2012 The Chromium OS Authors.
 * Copyright (c) 2012-2021 The strace developers.
 * Written by Mike Frysinger <vapier@gentoo.org>.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include DEF_MPERS_TYPE(struct_loop_info)

#include <linux/loop.h>

typedef struct loop_info struct_loop_info;

#include MPERS_DEFS

#include "xlat/loop_flags_options.h"
#include "xlat/loop_crypt_type_options.h"

static void
decode_loop_info(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct_loop_info info;

	if (umove_or_printaddr(tcp, addr, &info))
		return;

	tprint_struct_begin();
	PRINT_FIELD_D(info, lo_number);

	if (!abbrev(tcp)) {
		tprint_struct_next();
		PRINT_FIELD_DEV(info, lo_device);
		tprint_struct_next();
		PRINT_FIELD_U(info, lo_inode);
		tprint_struct_next();
		PRINT_FIELD_DEV(info, lo_rdevice);
	}

	tprint_struct_next();
	PRINT_FIELD_X(info, lo_offset);

	if (!abbrev(tcp) || info.lo_encrypt_type != LO_CRYPT_NONE) {
		tprint_struct_next();
		PRINT_FIELD_XVAL(info, lo_encrypt_type,
				 loop_crypt_type_options, "LO_CRYPT_???");
		/*
		 * It is converted to unsigned before use in the kernel,
		 * see loop_info64_from_old in drivers/block/loop.c
		 */
		tprint_struct_next();
		PRINT_FIELD_U(info, lo_encrypt_key_size);
	}

	tprint_struct_next();
	PRINT_FIELD_FLAGS(info, lo_flags, loop_flags_options, "LO_FLAGS_???");

	tprint_struct_next();
	PRINT_FIELD_CSTRING(info, lo_name);

	if (!abbrev(tcp) || info.lo_encrypt_type != LO_CRYPT_NONE) {
		const unsigned int lo_encrypt_key_size =
			MIN((unsigned) info.lo_encrypt_key_size, LO_KEY_SIZE);
		tprint_struct_next();
		PRINT_FIELD_STRING(info, lo_encrypt_key,
				   lo_encrypt_key_size, 0);
	}

	if (!abbrev(tcp)) {
		tprint_struct_next();
		PRINT_FIELD_X_ARRAY(info, lo_init);
		tprint_struct_next();
		PRINT_FIELD_X_ARRAY(info, reserved);
	} else {
		tprint_struct_next();
		tprint_more_data_follows();
	}

	tprint_struct_end();
}

static void
print_loop_info64(struct tcb *const tcp, const struct loop_info64 *const info64)
{
	if (!abbrev(tcp)) {
		tprint_struct_begin();
		PRINT_FIELD_DEV(*info64, lo_device);
		tprint_struct_next();
		PRINT_FIELD_U(*info64, lo_inode);
		tprint_struct_next();
		PRINT_FIELD_DEV(*info64, lo_rdevice);
		tprint_struct_next();
		PRINT_FIELD_X(*info64, lo_offset);
		tprint_struct_next();
		PRINT_FIELD_U(*info64, lo_sizelimit);
		tprint_struct_next();
		PRINT_FIELD_U(*info64, lo_number);
	} else {
		tprint_struct_begin();
		PRINT_FIELD_X(*info64, lo_offset);
		tprint_struct_next();
		PRINT_FIELD_U(*info64, lo_number);
	}

	if (!abbrev(tcp) || info64->lo_encrypt_type != LO_CRYPT_NONE) {
		tprint_struct_next();
		PRINT_FIELD_XVAL(*info64, lo_encrypt_type,
				 loop_crypt_type_options, "LO_CRYPT_???");
		tprint_struct_next();
		PRINT_FIELD_U(*info64, lo_encrypt_key_size);
	}

	tprint_struct_next();
	PRINT_FIELD_FLAGS(*info64, lo_flags, loop_flags_options, "LO_FLAGS_???");

	tprint_struct_next();
	PRINT_FIELD_CSTRING(*info64, lo_file_name);

	if (!abbrev(tcp) || info64->lo_encrypt_type != LO_CRYPT_NONE) {
		tprint_struct_next();
		PRINT_FIELD_CSTRING(*info64, lo_crypt_name);
		const unsigned int lo_encrypt_key_size =
			MIN((unsigned) info64->lo_encrypt_key_size, LO_KEY_SIZE);
		tprint_struct_next();
		PRINT_FIELD_STRING(*info64, lo_encrypt_key,
				   lo_encrypt_key_size, 0);
	}

	if (!abbrev(tcp)) {
		tprint_struct_next();
		PRINT_FIELD_X_ARRAY(*info64, lo_init);
	} else {
		tprint_struct_next();
		tprint_more_data_follows();
	}

	tprint_struct_end();
}

static void
decode_loop_info64(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct loop_info64 info64;

	if (!umove_or_printaddr(tcp, addr, &info64))
		print_loop_info64(tcp, &info64);
}

static void
decode_loop_config(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct loop_config config;

	if (umove_or_printaddr(tcp, addr, &config))
		return;

	tprint_struct_begin();
	PRINT_FIELD_FD(config, fd, tcp);

	tprint_struct_next();
	PRINT_FIELD_U(config, block_size);

	tprint_struct_next();
	PRINT_FIELD_OBJ_TCB_PTR(config, info, tcp, print_loop_info64);

	if (!IS_ARRAY_ZERO(config.__reserved)) {
		tprint_struct_next();
		PRINT_FIELD_X_ARRAY(config, __reserved);
	}

	tprint_struct_end();
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
		tprint_arg_next();
		decode_loop_info(tcp, arg);
		break;

	case LOOP_GET_STATUS64:
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;
	case LOOP_SET_STATUS64:
		tprint_arg_next();
		decode_loop_info64(tcp, arg);
		break;

	case LOOP_CONFIGURE:
		tprint_arg_next();
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
		tprint_arg_next();
		printfd(tcp, arg);
		break;

	/* newer loop-control stuff */
	case LOOP_CTL_ADD:
	case LOOP_CTL_REMOVE:
		tprint_arg_next();
		PRINT_VAL_D((int) arg);
		break;

	case LOOP_SET_DIRECT_IO:
	case LOOP_SET_BLOCK_SIZE:
		tprint_arg_next();
		PRINT_VAL_U(arg);
		break;

	default:
		return RVAL_DECODED;
	}

	return RVAL_IOCTL_DECODED;
}
