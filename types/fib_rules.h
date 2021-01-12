/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_TYPES_FIB_RULES_H
# define STRACE_TYPES_FIB_RULES_H

#ifdef HAVE_LINUX_FIB_RULES_H
# include <linux/fib_rules.h>
#endif

typedef struct {
	uint8_t	family;
	uint8_t dst_len;
	uint8_t src_len;
	uint8_t tos;
	uint8_t table;
	uint8_t res1;
	uint8_t res2;
	uint8_t action;
	uint32_t flags;
} struct_fib_rule_hdr;

typedef struct {
	uint32_t start;
	uint32_t end;
} struct_fib_rule_uid_range;

typedef struct {
	uint16_t start;
	uint16_t end;
} struct_fib_rule_port_range;

#endif /* STRACE_TYPES_FIB_RULES_H */
