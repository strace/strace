/*
 * Copyright (c) 2003, 2004 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2005-2015 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "defs.h"

#include DEF_MPERS_TYPE(struct_sigevent)

#include <signal.h>
typedef struct sigevent struct_sigevent;

#include MPERS_DEFS

#ifndef IN_MPERS
# include "xlat/sigev_value.h"
#endif

MPERS_PRINTER_DECL(void, print_sigevent)(struct tcb *tcp, const long addr)
{
	struct_sigevent sev;

	if (!umove_or_printaddr(tcp, addr, &sev)) {
		tprintf("{%#lx, ", (unsigned long) sev.sigev_value.sival_ptr);
		if (sev.sigev_notify == SIGEV_SIGNAL)
			tprintf("%s, ", signame(sev.sigev_signo));
		else
			tprintf("%u, ", sev.sigev_signo);
		printxval(sigev_value, sev.sigev_notify, "SIGEV_???");
		tprints(", ");
		if (sev.sigev_notify == SIGEV_THREAD_ID)
#if defined(HAVE_STRUCT_SIGEVENT__SIGEV_UN__PAD)
			/* _pad[0] is the _tid field which might not be
			   present in the userlevel definition of the
			   struct.  */
			tprintf("{%d}", sev._sigev_un._pad[0]);
#elif defined(HAVE_STRUCT_SIGEVENT___PAD)
			tprintf("{%d}", sev.__pad[0]);
#else
# warning unfamiliar struct sigevent => incomplete SIGEV_THREAD_ID decoding
			tprints("{...}");
#endif
		else if (sev.sigev_notify == SIGEV_THREAD)
			tprintf("{%#lx, %#lx}",
				(unsigned long) sev.sigev_notify_function,
				(unsigned long) sev.sigev_notify_attributes);
		else
			tprints("{...}");
		tprints("}");
	}
}
