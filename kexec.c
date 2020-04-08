/*
 * Copyright (c) 2014-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2014-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include "xlat/kexec_load_flags.h"
#include "xlat/kexec_arch_values.h"

#ifndef KEXEC_ARCH_MASK
# define KEXEC_ARCH_MASK 0xffff0000
#endif
#ifndef KEXEC_SEGMENT_MAX
# define KEXEC_SEGMENT_MAX 16
#endif

static bool
print_seg(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	const kernel_ulong_t *seg;
	kernel_ulong_t seg_buf[4];

	if (elem_size < sizeof(seg_buf)) {
		unsigned int i;

		for (i = 0; i < ARRAY_SIZE(seg_buf); ++i)
			seg_buf[i] = ((unsigned int *) elem_buf)[i];
		seg = seg_buf;
	} else {
		seg = elem_buf;
	}

	tprints("{buf=");
	printaddr(seg[0]);
	tprintf(", bufsz=%" PRI_klu ", mem=", seg[1]);
	printaddr(seg[2]);
	tprintf(", memsz=%" PRI_klu "}", seg[3]);

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
	/* entry, nr_segments */
	printaddr(tcp->u_arg[0]);
	tprintf(", %" PRI_klu ", ", tcp->u_arg[1]);

	/* segments */
	print_kexec_segments(tcp, tcp->u_arg[2], tcp->u_arg[1]);
	tprints(", ");

	/* flags */
	kernel_ulong_t n = tcp->u_arg[3];
	printxval64(kexec_arch_values, n & KEXEC_ARCH_MASK, "KEXEC_ARCH_???");
	n &= ~(kernel_ulong_t) KEXEC_ARCH_MASK;
	if (n) {
		tprints("|");
		printflags64(kexec_load_flags, n, "KEXEC_???");
	}

	return RVAL_DECODED;
}

#include "xlat/kexec_file_load_flags.h"

SYS_FUNC(kexec_file_load)
{
	/* kernel_fd */
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	/* initrd_fd */
	printfd(tcp, tcp->u_arg[1]);
	tprints(", ");
	/* cmdline_len */
	tprintf("%" PRI_klu ", ", tcp->u_arg[2]);
	/* cmdline */
	printstrn(tcp, tcp->u_arg[3], tcp->u_arg[2]);
	tprints(", ");
	/* flags */
	printflags64(kexec_file_load_flags, tcp->u_arg[4], "KEXEC_FILE_???");

	return RVAL_DECODED;
}
