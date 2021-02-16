/*
 * Copyright (c) 2017-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_BPF_FPROG_H
# define STRACE_BPF_FPROG_H

struct bpf_fprog {
	unsigned short len;
	kernel_ulong_t filter;
};

#endif /* !STRACE_BPF_FPROG_H */
