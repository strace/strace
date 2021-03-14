/*
 * Copyright (c) 2003, 2004 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2005-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include DEF_MPERS_TYPE(struct_sigevent)
#include "sigevent.h"
#include MPERS_DEFS

#include <signal.h>
#include "xlat/sigev_value.h"

static void
print_sigev_value(const typeof_field(struct_sigevent, sigev_value) v)
{
	tprint_struct_begin();
	PRINT_FIELD_D(v, sival_int);
	tprint_struct_next();
	PRINT_FIELD_PTR(v, sival_ptr);
	tprint_struct_end();
}

MPERS_PRINTER_DECL(void, print_sigevent,
		   struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct_sigevent sev;

	if (umove_or_printaddr(tcp, addr, &sev))
		return;

	tprint_struct_begin();
	if (sev.sigev_value.sival_ptr) {
		PRINT_FIELD_OBJ_VAL(sev, sigev_value, print_sigev_value);
		tprint_struct_next();
	}

	switch (sev.sigev_notify) {
	case SIGEV_SIGNAL:
	case SIGEV_THREAD:
	case SIGEV_THREAD_ID:
		PRINT_FIELD_OBJ_VAL(sev, sigev_signo, printsignal);
		break;
	default:
		PRINT_FIELD_U(sev, sigev_signo);
	}

	tprint_struct_next();
	PRINT_FIELD_XVAL(sev, sigev_notify, sigev_value, "SIGEV_???");

	switch (sev.sigev_notify) {
	case SIGEV_THREAD_ID:
#undef sigev_notify_thread_id
#define sigev_notify_thread_id sigev_un.tid
		tprint_struct_next();
		PRINT_FIELD_D(sev, sigev_notify_thread_id);
		break;
	case SIGEV_THREAD:
#undef sigev_notify_function
#define sigev_notify_function sigev_un.sigev_thread.function
		tprint_struct_next();
		PRINT_FIELD_PTR(sev, sigev_notify_function);
#undef sigev_notify_attributes
#define sigev_notify_attributes sigev_un.sigev_thread.attribute
		tprint_struct_next();
		PRINT_FIELD_PTR(sev, sigev_notify_attributes);
		break;
	}
	tprint_struct_end();
}
