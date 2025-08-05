/*
 * Copyright (c) 2014-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2014-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/kexec.h>

#include "xlat/kexec_load_flags.h"
#include "xlat/kexec_arch_values.h"

static bool
print_seg(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	const kernel_ulong_t *seg;
	kernel_ulong_t seg_buf[4];

	if (elem_size < sizeof(seg_buf)) {
		for (unsigned int i = 0; i < ARRAY_SIZE(seg_buf); ++i)
			seg_buf[i] = ((unsigned int *) elem_buf)[i];
		seg = seg_buf;
	} else {
		seg = elem_buf;
	}

	tprint_struct_begin();
	tprints_field_name("buf");
	printaddr(seg[0]);
	tprint_struct_next();
	tprints_field_name("bufsz");
	PRINT_VAL_U(seg[1]);
	tprint_struct_next();
	tprints_field_name("mem");
	printaddr(seg[2]);
	tprint_struct_next();
	tprints_field_name("memsz");
	PRINT_VAL_U(seg[3]);
	tprint_struct_end();

	return true;
}

static void
print_kexec_segments(struct tcb *const tcp, const kernel_ulong_t addr,
		     const kernel_ulong_t len)
{
	if (len > KEXEC_SEGMENT_MAX) {
		printaddr(addr);
		return;
	}

	kernel_ulong_t seg[4];
	const size_t sizeof_seg = ARRAY_SIZE(seg) * current_wordsize;

	print_array(tcp, addr, len, seg, sizeof_seg,
		    tfetch_mem, print_seg, 0);
}

SYS_FUNC(kexec_load)
{
	/* entry */
	tprints_arg_name("entry");
	printaddr(tcp->u_arg[0]);

	/* nr_segments */
	tprints_arg_next_name("nr_segments");
	PRINT_VAL_U(tcp->u_arg[1]);

	/* segments */
	tprints_arg_next_name("segments");
	print_kexec_segments(tcp, tcp->u_arg[2], tcp->u_arg[1]);

	/* flags */
	tprints_arg_next_name("flags");
	kernel_ulong_t n = tcp->u_arg[3];
	tprint_flags_begin();
	printxval64(kexec_arch_values, n & KEXEC_ARCH_MASK, "KEXEC_ARCH_???");
	n &= ~(kernel_ulong_t) KEXEC_ARCH_MASK;
	if (n) {
		tprint_flags_or();
		printflags64_in(kexec_load_flags, n, "KEXEC_???");
	}
	tprint_flags_end();

	return RVAL_DECODED;
}

#include "xlat/kexec_file_load_flags.h"

SYS_FUNC(kexec_file_load)
{
	/* kernel_fd */
	tprints_arg_name("kernel_fd");
	printfd(tcp, tcp->u_arg[0]);

	/* initrd_fd */
	tprints_arg_next_name("initrd_fd");
	printfd(tcp, tcp->u_arg[1]);

	/* cmdline_len */
	tprints_arg_next_name("cmdline_len");
	PRINT_VAL_U(tcp->u_arg[2]);

	/* cmdline */
	tprints_arg_next_name("cmdline");
	printstrn(tcp, tcp->u_arg[3], tcp->u_arg[2]);

	/* flags */
	tprints_arg_next_name("flags");
	printflags64(kexec_file_load_flags, tcp->u_arg[4], "KEXEC_FILE_???");

	return RVAL_DECODED;
}
