/*
 * Classic BPF filter block.
 *
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_BPF_FILTER_H
# define STRACE_BPF_FILTER_H

struct bpf_filter_block {
	uint16_t code;
	uint8_t jt;
	uint8_t jf;
	uint32_t k;
};

typedef bool (*print_bpf_filter_fn)(const struct bpf_filter_block *);

extern void
print_bpf_fprog(struct tcb *const tcp, const kernel_ulong_t addr,
		const unsigned short len, const print_bpf_filter_fn print_k);

extern void
decode_bpf_fprog(struct tcb *const tcp, const kernel_ulong_t addr,
		 const print_bpf_filter_fn print_k);

#endif /* !STRACE_BPF_FILTER_H */
