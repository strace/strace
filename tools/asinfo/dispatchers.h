/*
 * The dispatchers.h contains all necessary functions to perform main
 * work in the asinfo tool.
 *
 * Copyright (c) 2017 Edgar A. Kaziakhmedov <edgar.kaziakhmedov@virtuozzo.com>
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef ASINFO_DISPATCHERS_H
#define ASINFO_DISPATCHERS_H

#include "arch_interface.h"
#include "syscall_interface.h"

/* The function is purposed to provide correct list of architectures */
struct arch_service *arch_dispatcher(unsigned request_type, char *arch[]);

/* Final arch filtering based on personality */
int abi_dispatcher(struct arch_service *a_serv, unsigned request_type,
		   char *abi[]);

/* The last stage of main filtering */
struct syscall_service *syscall_dispatcher(struct arch_service *arch,
					   int request_type, char *sysc);

#endif /* !ASINFO_DISPATCHERS_H */
