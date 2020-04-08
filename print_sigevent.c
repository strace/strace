/*
 * Copyright (c) 2003, 2004 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2005-2018 Dmitry V. Levin <ldv@altlinux.org>
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

MPERS_PRINTER_DECL(void, print_sigevent,
		   struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct_sigevent sev;

	if (umove_or_printaddr(tcp, addr, &sev))
		return;

	tprints("{");
	if (sev.sigev_value.sival_ptr) {
		tprintf("sigev_value={sival_int=%d, sival_ptr=",
			sev.sigev_value.sival_int);
		printaddr(sev.sigev_value.sival_ptr);
		tprints("}, ");
	}

	tprints("sigev_signo=");
	switch (sev.sigev_notify) {
	case SIGEV_SIGNAL:
	case SIGEV_THREAD:
	case SIGEV_THREAD_ID:
		printsignal(sev.sigev_signo);
		break;
	default:
		tprintf("%u", sev.sigev_signo);
	}

	tprints(", sigev_notify=");
	printxval(sigev_value, sev.sigev_notify, "SIGEV_???");

	switch (sev.sigev_notify) {
	case SIGEV_THREAD_ID:
		tprintf(", sigev_notify_thread_id=%d", sev.sigev_un.tid);
		break;
	case SIGEV_THREAD:
		tprints(", sigev_notify_function=");
		printaddr(sev.sigev_un.sigev_thread.function);
		tprints(", sigev_notify_attributes=");
		printaddr(sev.sigev_un.sigev_thread.attribute);
		break;
	}
	tprints("}");
}
