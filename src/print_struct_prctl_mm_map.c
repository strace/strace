/*
 * Decode struct sg_req_info.
 *
 * Copyright (c) 2021 Eugene Syromyatnikov <evgsyr@gmail.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include DEF_MPERS_TYPE(struct_prctl_mm_map)

#include <linux/prctl.h>

typedef struct prctl_mm_map struct_prctl_mm_map;

#include MPERS_DEFS

MPERS_PRINTER_DECL(void, print_struct_prctl_mm_map,
		   struct tcb *tcp, kernel_ulong_t addr, kernel_ulong_t size)
{
	struct_prctl_mm_map arg;

	if (size < offsetofend(struct_prctl_mm_map, exe_fd)) {
		printaddr(addr);
		return;
	}
	if (umoven_or_printaddr(tcp, addr, MIN(size, sizeof(arg)), &arg))
		return;

	tprint_struct_begin();
	PRINT_FIELD_X(arg, start_code);
	tprint_struct_next();
	PRINT_FIELD_X(arg, end_code);
	tprint_struct_next();
	PRINT_FIELD_X(arg, start_data);
	tprint_struct_next();
	PRINT_FIELD_X(arg, end_data);
	tprint_struct_next();
	PRINT_FIELD_X(arg, start_brk);
	tprint_struct_next();
	PRINT_FIELD_X(arg, brk);
	tprint_struct_next();
	PRINT_FIELD_X(arg, start_stack);
	tprint_struct_next();
	PRINT_FIELD_X(arg, arg_start);
	tprint_struct_next();
	PRINT_FIELD_X(arg, arg_end);
	tprint_struct_next();
	PRINT_FIELD_X(arg, env_start);
	tprint_struct_next();
	PRINT_FIELD_X(arg, env_end);
	tprint_struct_next();
	tprints_field_name("auxv");
	print_auxv(tcp, (mpers_ptr_t) arg.auxv, arg.auxv_size);
	tprint_struct_next();
	PRINT_FIELD_U(arg, auxv_size);
	tprint_struct_next();
	PRINT_FIELD_FD(arg, exe_fd, tcp);
	if (size > sizeof(arg)) {
		tprint_struct_next();
		tprint_more_data_follows();
	}
	tprint_struct_end();
}
