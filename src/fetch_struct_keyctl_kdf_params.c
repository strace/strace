/*
 * Copyright (c) 2016-2021 Eugene Syromyatnikov <evgsyr@gmail.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include DEF_MPERS_TYPE(struct_keyctl_kdf_params)

#include "keyctl_kdf_params.h"
typedef struct keyctl_kdf_params struct_keyctl_kdf_params;

#include MPERS_DEFS

MPERS_PRINTER_DECL(int, fetch_keyctl_kdf_params, struct tcb *const tcp,
		   kernel_ulong_t addr, struct strace_keyctl_kdf_params *p)
{
	struct_keyctl_kdf_params kdf;
	int ret;

	if ((ret = umove(tcp, addr, &kdf)))
		return ret;

	p->hashname = (kernel_ulong_t)
#ifndef IN_MPERS
		(uintptr_t)
#endif
		kdf.hashname;
	p->otherinfo = (kernel_ulong_t)
#ifndef IN_MPERS
		(uintptr_t)
#endif
		kdf.otherinfo;
	p->otherinfolen = kdf.otherinfolen;

	memcpy(p->__spare, kdf.__spare, sizeof(kdf.__spare));

	return 0;
}
