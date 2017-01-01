/*
 * Copyright (c) 2012 The Chromium OS Authors.
 * Written by Mike Frysinger <vapier@gentoo.org>.
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
#include <linux/ioctl.h>
#include <linux/loop.h>

typedef struct loop_info struct_loop_info;

#include DEF_MPERS_TYPE(struct_loop_info)

#include MPERS_DEFS

#include "xlat/loop_cmds.h"
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
		tprints(", lo_device=");
		print_dev_t(info.lo_device);
		tprintf(", lo_inode=%" PRI_klu, (kernel_ulong_t) info.lo_inode);
		tprints(", lo_rdevice=");
		print_dev_t(info.lo_rdevice);
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

	tprints(", lo_name=");
	print_quoted_string(info.lo_name, LO_NAME_SIZE,
			    QUOTE_0_TERMINATED);

	if (!abbrev(tcp) || info.lo_encrypt_type != LO_CRYPT_NONE) {
		tprints(", lo_encrypt_key=");
		print_quoted_string((void *) info.lo_encrypt_key,
				    MIN((uint32_t) info.lo_encrypt_key_size,
				    LO_KEY_SIZE), 0);
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
		tprints("{lo_device=");
		print_dev_t(info64.lo_device);
		tprintf(", lo_inode=%" PRIu64, (uint64_t) info64.lo_inode);
		tprints(", lo_rdevice=");
		print_dev_t(info64.lo_rdevice);
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

	tprints(", lo_file_name=");
	print_quoted_string((void *) info64.lo_file_name,
			    LO_NAME_SIZE, QUOTE_0_TERMINATED);

	if (!abbrev(tcp) || info64.lo_encrypt_type != LO_CRYPT_NONE) {
		tprints(", lo_crypt_name=");
		print_quoted_string((void *) info64.lo_crypt_name,
				    LO_NAME_SIZE, QUOTE_0_TERMINATED);
		tprints(", lo_encrypt_key=");
		print_quoted_string((void *) info64.lo_encrypt_key,
				    MIN(info64.lo_encrypt_key_size,
				    LO_KEY_SIZE), 0);
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
		/* fall through */
	case LOOP_SET_STATUS:
		decode_loop_info(tcp, arg);
		break;

	case LOOP_GET_STATUS64:
		if (entering(tcp))
			return 0;
		/* fall through */
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
		tprintf(", %" PRI_klu, arg);
		break;

	default:
		return RVAL_DECODED;
	}

	return RVAL_DECODED | 1;
}
